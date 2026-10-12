#include "Interactions/Actors/ChaosResonanceObj.h"

#include "GameplayTagContainer.h"
#include "StillHearGameInstance.h"
#include "AbilitySystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "Components/AudioComponent.h"
#include "SaveSystem/SaveIdComponent.h"
#include "Components/CapsuleComponent.h"
#include "UI/Indicator/IndicatorComponent.h"
#include "TraceAndCollision/CustomCollision.h"

#pragma region CONSTRUCTOR
AChaosResonanceObj::AChaosResonanceObj()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	
	SaveIdComponent = CreateDefaultSubobject<USaveIdComponent>(TEXT("SaveIdComponent"));

	CoreGeometryCollectionComponent = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("GeometryCollectionComponent"));
	CoreGeometryCollectionComponent->SetupAttachment(RootComponent);
	CoreGeometryCollectionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CoreGeometryCollectionComponent->SetCollisionResponseToAllChannels(ECR_Block);
	CoreGeometryCollectionComponent->SetCollisionObjectType(ECC_WorldDynamic);
	CoreGeometryCollectionComponent->SetSimulatePhysics(false);
	
	DebrisGeometryCollectionComponent = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("DebrisGeometryCollectionComponent"));
	DebrisGeometryCollectionComponent->SetupAttachment(RootComponent);
	DebrisGeometryCollectionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	DebrisGeometryCollectionComponent->SetCollisionResponseToAllChannels(ECR_Block);
	DebrisGeometryCollectionComponent->SetCollisionObjectType(ECustomCollision::Resonance);
	DebrisGeometryCollectionComponent->SetSimulatePhysics(false);
	
	OutlineMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OutlineMeshComponent"));
	OutlineMeshComponent->SetupAttachment(RootComponent);
	OutlineMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	OutlineMeshComponent->SetVisibility(false);
	
	NiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponent"));
	NiagaraComponent->SetupAttachment(RootComponent);
	NiagaraComponent->bAutoActivate = false;

	ProximityNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ProximityNiagaraComponent"));
	ProximityNiagaraComponent->SetupAttachment(RootComponent);
	ProximityNiagaraComponent->SetAutoActivate(false);
	
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComponent->SetupAttachment(RootComponent);
	SphereComponent->SetGenerateOverlapEvents(true);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	
	IndicatorComponent = CreateDefaultSubobject<UIndicatorComponent>(TEXT("IndicatorComponent"));
	IndicatorComponent->bAutoRegister = false;

	ShakeComponent = CreateDefaultSubobject<UInteractableShakeComponent>(TEXT("ShakeComponent"));
	
	CacheMode = ECacheMode::Play;
	StartMode = EStartMode::Triggered;
	
	InteractionSuccessTag = FGameplayTag::RequestGameplayTag("Event.Interaction.Success");
}
#pragma endregion

#pragma region METHODS
void AChaosResonanceObj::BeginPlay()
{
	bHasCoreGeometry = (CoreGeometryCollectionComponent && CoreGeometryCollectionComponent->GetRestCollection() != nullptr);

	// Climb traces gate on the plain "Climb" actor tag, not on ResonanceTags
	if (bClimbable)
		Tags.AddUnique(FName("Climb"));

	// Workaround for UE5 Geometry Collection streaming bug:
	// We dynamically spawn a clone, and the original actor goes invisible and forwards all interactions to the clone
	if (!Tags.Contains(FName("DynamicallySpawned")))
	{
		// BeginPlay can re-run on this proxy without an EndPlay in between (streaming show/hide);
		// don't spawn a second clone on top of an already-pending or already-live one.
		if (bCloneSpawnPending)
		{
			return;
		}

		if (IsValid(ActiveClone))
		{
			// Disable collision immediately (Destroy() below is deferred and Chaos's own removal can
			// lag a frame) so a physics query can't resolve this clone's body after it's been retired.
			if (ActiveClone->CoreGeometryCollectionComponent)
				ActiveClone->CoreGeometryCollectionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			if (ActiveClone->DebrisGeometryCollectionComponent)
				ActiveClone->DebrisGeometryCollectionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			// Deferred to next tick: destroying it synchronously here could race with this same
			// AddToWorld pass still dispatching BeginPlay to other actors in this level.
			TWeakObjectPtr<AChaosResonanceObj> WeakOldClone(ActiveClone);
			if (UWorld* World = GetWorld())
			{
				World->GetTimerManager().SetTimerForNextTick([WeakOldClone]()
				{
					if (AChaosResonanceObj* OldClone = WeakOldClone.Get())
					{
						OldClone->Destroy();
					}
				});
			}
			ActiveClone = nullptr;
		}

		// Proxy must be Movable for the save system to restore its transform
		if (RootComponent)
			RootComponent->SetMobility(EComponentMobility::Movable);

		InitialTransform = GetActorTransform();

		// The proxy is the only actor with a stable GUID; register it for
		// checkpoint snapshots so checkpoint state is persisted to disk
		if (const UWorld* World = GetWorld())
		{
			if (UStillHearGameInstance* GI = World->GetGameInstance<UStillHearGameInstance>())
			{
				// Proxy must also listen to world reset: if the clone was destroyed (level unloaded)
				// between sessions, only the proxy can clear bIsConsumed and re-spawn a fresh clone.
				// Remove first: BeginPlay re-entering without an EndPlay in between (see above) would
				// otherwise stack a duplicate binding, causing Reset/SaveCheckpointState/ClearCheckpointState
				// to fire more than once per broadcast on this same proxy.
				GI->OnRequestWorldReset.RemoveAll(this);
				GI->OnCheckpointSnapshot.RemoveAll(this);
				GI->OnClearCheckpointState.RemoveAll(this);

				GI->OnRequestWorldReset.AddUObject(this, &AChaosResonanceObj::Reset);
				GI->OnCheckpointSnapshot.AddUObject(this, &AChaosResonanceObj::SaveCheckpointState);
				GI->OnClearCheckpointState.AddUObject(this, &AChaosResonanceObj::ClearCheckpointState);
			}
		}

		DeferredRespawn(GetActorTransform());
		return; // Stay dormant as an invisible proxy
	}

	if (!CacheColl)
	{
		return;
	}

	CacheCollection = CacheColl;

	if (!CacheCollection)
		return;
	
	// Set up the debris fade-out timeline
	if (FadeCurve)
	{
		FOnTimelineFloat ProgressFunction;
		ProgressFunction.BindUFunction(this, FName("HandleFadeUpdate"));
		DebrisTimeline.AddInterpFloat(FadeCurve, ProgressFunction);

		FOnTimelineEvent FinishedFunction;
		FinishedFunction.BindUFunction(this, FName("HandleFadeFinished"));
		DebrisTimeline.SetTimelineFinishedFunc(FinishedFunction);
	}
	
	// Register geometry collection components as observed chaos cache tracks
	ClearObservedComponents();
	
	if (CoreGeometryCollectionComponent && CoreGeometryCollectionComponent->GetRestCollection())
	{
		FObservedComponent& CoreObservedComponent = AddNewObservedComponent(CoreGeometryCollectionComponent); // Add the geometry collection component to the list of observed components
		CoreObservedComponent.CacheName = CoreCacheTrackName; // Set the name of the cache track to observe
	}
	
	if (DebrisGeometryCollectionComponent && DebrisGeometryCollectionComponent->GetRestCollection())
	{
		FObservedComponent& DebrisObservedComponent = AddNewObservedComponent(DebrisGeometryCollectionComponent);
		DebrisObservedComponent.CacheName = DebrisCacheTrackName;
	}
	
	if (!SphereComponent)
		return;
	
	for (const TEnumAsByte Channel : CollisionChannels)
		SphereComponent->SetCollisionResponseToChannel(Channel, ECR_Overlap);
	
	SphereComponent->OnComponentBeginOverlap.AddUniqueDynamic(this, &AChaosResonanceObj::OnSphereBeginOverlap);
	SphereComponent->OnComponentEndOverlap.AddUniqueDynamic(this, &AChaosResonanceObj::OnSphereEndOverlap);
	
	// Bind to global reset event
	if (const UWorld* World = GetWorld())
	{
		if (UStillHearGameInstance* GI = World->GetGameInstance<UStillHearGameInstance>())
		{
			GI->OnRequestWorldReset.AddUObject(this, &AChaosResonanceObj::Reset);
			GI->OnCheckpointSnapshot.AddUObject(this, &AChaosResonanceObj::SaveCheckpointState);
			GI->OnClearCheckpointState.AddUObject(this, &AChaosResonanceObj::ClearCheckpointState);
		}
	}

	InitialTransform = GetActorTransform();
	
	if (IndicatorComponent)
	{
		OriginalPromptActions = IndicatorComponent->InputActions;
		OriginalSeparatorClass = IndicatorComponent->SeparatorClass;
	}
	
	if (RootComponent)
	{
		RootComponent->SetMobility(EComponentMobility::Movable);
	}

	Super::BeginPlay();
}

void AChaosResonanceObj::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Unbind from global reset to prevent stale callbacks when this actor's sublevel is unloaded
	if (const UWorld* World = GetWorld())
	{
		if (UStillHearGameInstance* GI = World->GetGameInstance<UStillHearGameInstance>())
		{
			GI->OnRequestWorldReset.RemoveAll(this);
			GI->OnCheckpointSnapshot.RemoveAll(this);
			GI->OnClearCheckpointState.RemoveAll(this);
		}
	}

	if (ActiveClone)
	{
		// Disable collision immediately (Destroy() below is deferred and Chaos's own removal can
		// lag a frame) so a physics query can't resolve this clone's body after it's been retired.
		if (ActiveClone->CoreGeometryCollectionComponent)
			ActiveClone->CoreGeometryCollectionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if (ActiveClone->DebrisGeometryCollectionComponent)
			ActiveClone->DebrisGeometryCollectionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// Deferred to next tick: this EndPlay can fire as part of the same streaming teardown
		// pass that is about to reach the clone itself, so destroying it here would race with that.
		TWeakObjectPtr<AChaosResonanceObj> WeakClone(ActiveClone);
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimerForNextTick([WeakClone]()
			{
				if (AChaosResonanceObj* Clone = WeakClone.Get())
				{
					Clone->Destroy();
				}
			});
		}
		ActiveClone = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void AChaosResonanceObj::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (DebrisTimeline.IsPlaying())
		DebrisTimeline.TickTimeline(DeltaSeconds);
}

void AChaosResonanceObj::SetHighlight(const bool bEnableHighlight) const
{
	if (IsValid(ActiveClone))
	{
		ActiveClone->SetHighlight(bEnableHighlight);
		return;
	}

	if (!OutlineMeshComponent)
		return;

	OutlineMeshComponent->SetVisibility(bEnableHighlight);
}
#pragma endregion

#pragma region UFUNCTIONS
void AChaosResonanceObj::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this || IsHidden() || bIsInteracting || bIsConsumed)
	{
		return;
	}

	SetHighlight(true);
	
	if (IndicatorComponent)
		IndicatorComponent->RegisterIndicator();

	if (ShakeComponent)
		ShakeComponent->SetInRange(true);

	PlayProximityVFX(OtherActor);

	if (ProximitySound && OtherActor->IsA(ACharacter::StaticClass()))
	{
		if (!IsValid(ProximityAudioComponent))
		{
			ProximityAudioComponent = UGameplayStatics::SpawnSoundAtLocation(this, ProximitySound, GetActorLocation(), FRotator::ZeroRotator, 1.0f, 1.0f, 0.0f, nullptr, nullptr, true);
		}
		if (IsValid(ProximityAudioComponent))
		{
			if (FadeInDuration > 0.0f)
			{
				ProximityAudioComponent->FadeIn(FadeInDuration, 1.0f);
			}
			else
			{
				ProximityAudioComponent->Play();
			}
		}
	}
}

void AChaosResonanceObj::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor || OtherActor == this)
		return;

	SetHighlight(false);
	
	if (IndicatorComponent)
		IndicatorComponent->UnregisterIndicator();

	if (ShakeComponent)
		ShakeComponent->SetInRange(false);

	StopProximityVFX(OtherActor);

	if (IsValid(ProximityAudioComponent) && OtherActor->IsA(ACharacter::StaticClass()))
	{
		if (FadeOutDuration > 0.0f)
		{
			ProximityAudioComponent->FadeOut(FadeOutDuration, 0.0f);
		}
		else
		{
			ProximityAudioComponent->Stop();
		}
		ProximityAudioComponent = nullptr;
	}
}

void AChaosResonanceObj::HandleFadeUpdate(const float Value) const
{
	if (DebrisGeometryCollectionComponent)
		DebrisGeometryCollectionComponent->SetScalarParameterValueOnMaterials(FName("FadeOpacity"), Value);
}

void AChaosResonanceObj::HandleFadeFinished()
{
	if (DebrisGeometryCollectionComponent)
	{
		DebrisGeometryCollectionComponent->SetVisibility(false, true);
		DebrisGeometryCollectionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	SetActorTickEnabled(false);
	EndInteraction(PendingInteractor);

	// Always finalize via ApplyConsumedState here regardless of bReset - it only affects a later
	// Reset()/checkpoint revert, not this break, otherwise no-core-geometry objects never disable collision.
	ApplyConsumedState();
	if (IsValid(MyProxy))
	{
		MyProxy->ApplyConsumedState();
	}
}

void AChaosResonanceObj::CallInteraction(ACharacter* Interactor, const float CollisionIgnoreDelay)
{
	if (IsValid(ActiveClone))
	{
		ActiveClone->CallInteraction(Interactor, CollisionIgnoreDelay);
		return;
	}

	ExecuteStartInteraction(Interactor, CollisionIgnoreDelay);
}

EChaosResonanceState AChaosResonanceObj::GetResonanceState() const
{
	if (bIsConsumed)   
		return EChaosResonanceState::Consumed;
	
	if (bIsInteracting) 
		return EChaosResonanceState::Breaking;
	
	return EChaosResonanceState::Intact;
}
#pragma endregion

#pragma region INTERFACE METHODS
void AChaosResonanceObj::StartInteraction(const TObjectPtr<ACharacter> Interactor)
{
	if (IsValid(ActiveClone))
	{
		ActiveClone->StartInteraction(Interactor);
		return;
	}

	if (bIsInteracting || GetWorldTimerManager().IsTimerActive(PreInteractionTimerHandle) || bIsConsumed)
	{
		return;
	}

	bIsInteracting = true;

	if (ShakeComponent && ShakeComponent->ShouldPreInteractionShake())
	{
		PendingInteractor = Interactor;
		ShakeComponent->SetPreInteracting(true);

		FTimerDelegate TimerDel;
		TimerDel.BindUObject(this, &AChaosResonanceObj::ExecuteStartInteraction, PendingInteractor, 0.0f);
		GetWorldTimerManager().SetTimer(PreInteractionTimerHandle, TimerDel, ShakeComponent->GetPreInteractionDuration(), false);
	}
	else
	{
		ExecuteStartInteraction(Interactor);
	}
}

void AChaosResonanceObj::ExecuteStartInteraction(const TObjectPtr<ACharacter> Interactor, const float CollisionIgnoreDelay)
{
	// Guard against re-entrant calls (CallInteraction/TriggerInteraction can reach this directly,
	// bypassing StartInteraction's guard), which would restart Chaos cache playback mid-break.
	if (bIsConsumed)
	{
		return;
	}

	// Broadcast to Blueprint listeners regardless of activation path (resonance or trigger component)
	OnTriggeredBy.Broadcast(PendingTriggerer.Get());
	PendingTriggerer = nullptr;

	if (LinkedResonanceObj && LinkedResonanceObj != this)
	{
		LinkedResonanceObj->TriggerInteraction(this);
	}

	PendingInteractor = Interactor;
	bIsConsumed = true;
	
	// Propagate consumed state to the proxy actor so that it gets saved when level streams out
	if (IsValid(MyProxy))
	{
		MyProxy->bIsConsumed = true;
	}
	
	// Update or hide the indicator prompt for the resonance phase
	if (IndicatorComponent)
	{
		if (ResonancePromptActions.Num() > 0)
		{
			IndicatorComponent->UpdatePromptActions(ResonancePromptActions, ResonancePromptSeparatorClass);
			IndicatorComponent->RegisterIndicator();
		}
		else
		{
			IndicatorComponent->UnregisterIndicator();
		}
	}
	
	if (OutlineMeshComponent)
	{
		SetHighlight(false);
		OutlineMeshComponent->SetVisibility(false);
	}
	
	// Permanently remove the GeometryCollectionComponents from the shake system before handing them to the Chaos cache player
	if (ShakeComponent)
	{
		ShakeComponent->StopShakingComponent(CoreGeometryCollectionComponent);
		ShakeComponent->StopShakingComponent(DebrisGeometryCollectionComponent);
	}

	// Play the chaos cache destruction animation
	SetStartTime(0.0f);
	SetActorTickEnabled(true); // Crucial for AChaosCacheManager to advance the sequence!
	TriggerAll();
	
	if (NiagaraComponent)
		NiagaraComponent->Activate(true);

	if (ProximityNiagaraComponent)
	{
		ProximityNiagaraComponent->Deactivate();
	}
	
	// Start the debris fade-out timeline if configured
	if (DebrisGeometryCollectionComponent && FadeCurve)
	{
		DebrisTimeline.PlayFromStart();
	}
	else
	{
		// Fallback to ensure interaction ends and collisions are restored if FadeCurve is missing
		FTimerHandle FallbackHandle;
		GetWorldTimerManager().SetTimer(FallbackHandle, [this]()
		{
			HandleFadeFinished();
		}, 3.0f, false);
	}

	if (IsValid(ChaosAudioComponent))
	{
		ChaosAudioComponent->Stop();
	}
	if (ChaosInteractionSound)	
	{
		ChaosAudioComponent = UGameplayStatics::SpawnSoundAtLocation(this, ChaosInteractionSound, GetActorLocation(), FRotator::ZeroRotator, 1.0f, 1.0f, 0.0f, nullptr, nullptr, true);
		if (IsValid(ChaosAudioComponent))
		{
			if (FadeInDuration > 0.0f)
			{
				ChaosAudioComponent->FadeIn(FadeInDuration, 1.0f);
			}
			else
			{
				ChaosAudioComponent->Play();
			}
		}
	}
	
	// Notify the interactor's ability system that the interaction succeeded.
	// Interactor is only set when the caller explicitly wants this treatment (see CallInteraction) -
	// leave it unset for triggers that shouldn't touch the interactor's collision at all.
	if (Interactor)
	{
		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Interactor))
		{
			if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
			{
				FGameplayEventData Payload;
				Payload.EventTag = InteractionSuccessTag;
				Payload.Instigator = Cast<AActor>(Interactor);
				Payload.Target = Cast<AActor>(Interactor);
				Payload.OptionalObject = this;

				ASC->HandleGameplayEvent(Payload.EventTag, &Payload);
			}
		}

		// Ignore resonance collision on the player during destruction. Delayed if CollisionIgnoreDelay > 0
		if (CollisionIgnoreDelay > 0.0f)
		{
			TWeakObjectPtr<ACharacter> WeakInteractor(Interactor.Get());
			FTimerHandle IgnoreCollisionTimerHandle;
			GetWorldTimerManager().SetTimer(IgnoreCollisionTimerHandle, [WeakInteractor]()
			{
				if (ACharacter* Char = WeakInteractor.Get())
				{
					if (UCapsuleComponent* CapsuleComponent = Char->GetCapsuleComponent())
					{
						CapsuleComponent->SetCollisionResponseToChannel(ECustomCollision::Resonance, ECR_Ignore);
					}
				}
			}, CollisionIgnoreDelay, false);
		}
		else if (UCapsuleComponent* CapsuleComponent = Interactor->GetCapsuleComponent())
		{
			CapsuleComponent->SetCollisionResponseToChannel(ECustomCollision::Resonance, ECR_Ignore);
		}
	}

	if (ShakeComponent)
		ShakeComponent->SetInteracting(true);

	SetHighlight(false);
}

void AChaosResonanceObj::TriggerInteraction(AActor* Triggerer)
{
	if (IsValid(ActiveClone))
	{
		ActiveClone->TriggerInteraction(Triggerer);
		return;
	}

	PendingTriggerer = Triggerer;
	StartInteraction();
}

void AChaosResonanceObj::EndInteraction(TObjectPtr<ACharacter> Interactor)
{
	if (IsValid(ActiveClone))
	{
		ActiveClone->EndInteraction(Interactor);
		return;
	}

	if (!bIsInteracting)
		return;

	// Restore resonance collision on the player
	if (IsValid(Interactor))
	{
		if (UCapsuleComponent* CapsuleComponent = Interactor->GetCapsuleComponent())
		{
			CapsuleComponent->SetCollisionResponseToChannel(ECustomCollision::Resonance, ECR_Block);
		}
	}

	if (IndicatorComponent)
	{
		IndicatorComponent->UpdatePromptActions(OriginalPromptActions, OriginalSeparatorClass);
		IndicatorComponent->UnregisterIndicator();
	}

	bIsInteracting = false;
	GetWorldTimerManager().ClearTimer(PreInteractionTimerHandle);
	PendingInteractor = nullptr;

	if (IsValid(ChaosAudioComponent))
	{
		if (FadeOutDuration > 0.0f)
		{
			ChaosAudioComponent->FadeOut(FadeOutDuration, 0.0f);
		}
		else
		{
			ChaosAudioComponent->Stop();
		}
		ChaosAudioComponent = nullptr;
	}

	if (IsValid(ProximityAudioComponent))
	{
		if (FadeOutDuration > 0.0f)
		{
			ProximityAudioComponent->FadeOut(FadeOutDuration, 0.0f);
		}
		else
		{
			ProximityAudioComponent->Stop();
		}
		ProximityAudioComponent = nullptr;
	}

	if (ShakeComponent)
	{
		ShakeComponent->SetInteracting(false);
		ShakeComponent->PlayAfterShake();
	}
}

void AChaosResonanceObj::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	if (IsValid(ActiveClone))
	{
		ActiveClone->GetOwnedGameplayTags(TagContainer);
		return;
	}
	TagContainer.AppendTags(ResonanceTags);
}

void AChaosResonanceObj::Reset()
{
	if (IsValid(ChaosAudioComponent))
	{
		ChaosAudioComponent->Stop();
		ChaosAudioComponent = nullptr;
	}

	if (IsValid(ProximityAudioComponent))
	{
		ProximityAudioComponent->Stop();
		ProximityAudioComponent = nullptr;
	}

	if (ProximityNiagaraComponent)
	{
		ProximityNiagaraComponent->Deactivate();
	}

	// Guard against stale delegates from actors whose sublevel was hidden/unloaded
	if (!IsValid(this) || !GetWorld())
	{
		return;
	}

	bool bForceNewGame = false;
	if (const UWorld* World = GetWorld())
	{
		if (const UStillHearGameInstance* GI = World->GetGameInstance<UStillHearGameInstance>())
		{
			bForceNewGame = GI->bIsNewGameResetting;
		}
	}

	const bool bIsProxy = !Tags.Contains(FName("DynamicallySpawned"));

	if (bIsProxy)
	{
		// Proxy path: Check if we are consumed and either should not reset or were consumed at a checkpoint
		const bool bProxyOrCloneConsumed = bIsConsumed || (IsValid(ActiveClone) && ActiveClone->bIsConsumed);
		if (!bReset && bProxyOrCloneConsumed && !bForceNewGame)
		{
			ApplyConsumedState();
			if (IsValid(ActiveClone))
			{
				ActiveClone->ApplyConsumedState();
			}
			return;
		}

		if (bReset && bHasCheckpointSnapshot && bCheckpointIsConsumed)
		{
			ApplyConsumedState();
			if (IsValid(ActiveClone))
			{
				ActiveClone->ApplyConsumedState();
			}
			return;
		}

		bIsConsumed = false;
	}
	else
	{
		// Clone path: Delegate proxy destruction or consumed state transitions
		if (!bReset && bIsConsumed && !bForceNewGame)
		{
			ApplyConsumedState();
			if (IsValid(MyProxy))
			{
				MyProxy->ApplyConsumedState();
			}
			return;
		}

		if (bReset && bHasCheckpointSnapshot && bCheckpointIsConsumed)
		{
			ApplyConsumedState();
			if (IsValid(MyProxy))
			{
				MyProxy->ApplyConsumedState();
			}
			return;
		}

		bIsConsumed = false;
		if (IsValid(MyProxy))
		{
			MyProxy->bIsConsumed = false;
		}
	}

	if (IsValid(ActiveClone))
	{
		ActiveClone->Reset();
		return;
	}

	// Guard against duplicate next-tick clone spawns
	if (bCloneSpawnPending)
	{
		return;
	}

	if (!bReset && !bForceNewGame)
		return;

	DeferredRespawn(InitialTransform);
}

void AChaosResonanceObj::SaveCheckpointState()
{
	bHasCheckpointSnapshot = true;
	
	if (IsValid(ActiveClone))
	{
		bCheckpointIsConsumed = ActiveClone->bIsConsumed;
	}
	else
	{
		bCheckpointIsConsumed = bIsConsumed;
	}
}

void AChaosResonanceObj::ClearCheckpointState()
{
	bHasCheckpointSnapshot = false;
}

void AChaosResonanceObj::OnPostLoad_Implementation()
{
	if (!Tags.Contains(FName("DynamicallySpawned")))
	{
		// Proxy path: save data has just been deserialized
		// DeferredRespawn may have already fired (timing-dependent) and spawned a clone before the save data was available
		// If currently consumed or consumed at checkpoint, check core geometry
		if (bIsConsumed || (bHasCheckpointSnapshot && bCheckpointIsConsumed))
		{
			ApplyConsumedState();
			if (IsValid(ActiveClone))
			{
				ActiveClone->ApplyConsumedState();
			}
		}
		return;
	}

	Reset();
}

void AChaosResonanceObj::DeferredRespawn(const FTransform& SpawnTransform)
{
	UClass* ActorClass = GetClass();
	TWeakObjectPtr WeakLevel(GetLevel());
	const bool bSavedReset = bReset;
	
	TWeakObjectPtr WeakWorld(GetWorld());
	TWeakObjectPtr WeakThis(this);

	const bool bIsProxy = !Tags.Contains(FName("DynamicallySpawned"));

	bCloneSpawnPending = true;

	if (bIsProxy)
	{
		// Hide and disable collision, becoming an invisible proxy
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);
		SetActorTickEnabled(false);
		
		if (CoreGeometryCollectionComponent)
		{
			CoreGeometryCollectionComponent->SetVisibility(false, true);
			CoreGeometryCollectionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			CoreGeometryCollectionComponent->DestroyComponent();
		}
		if (DebrisGeometryCollectionComponent)
		{
			DebrisGeometryCollectionComponent->SetVisibility(false, true);
			DebrisGeometryCollectionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			DebrisGeometryCollectionComponent->DestroyComponent();
		}
		if (SphereComponent)
		{
			SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			SphereComponent->SetGenerateOverlapEvents(false);
			SphereComponent->DestroyComponent();
		}
		if (OutlineMeshComponent)
		{
			OutlineMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			OutlineMeshComponent->SetVisibility(false);
			OutlineMeshComponent->DestroyComponent();
		}
	}
	else
	{
		// If a clone resetting, hide before destruction to prevent UE5 Chaos Ghost Mesh bugs
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);
		
		if (CoreGeometryCollectionComponent)
		{
			CoreGeometryCollectionComponent->SetVisibility(false, true);
			CoreGeometryCollectionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		if (DebrisGeometryCollectionComponent)
		{
			DebrisGeometryCollectionComponent->SetVisibility(false, true);
			DebrisGeometryCollectionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}

	if (UWorld* World = WeakWorld.Get())
	{
		World->GetTimerManager().SetTimerForNextTick([WeakThis, ActorClass, WeakLevel, bSavedReset, SpawnTransform, WeakWorld, bIsProxy]()
		{
			AChaosResonanceObj* OldActor = WeakThis.Get();
			if (IsValid(OldActor))
			{
				OldActor->bCloneSpawnPending = false;

				const bool bMarkedConsumed = OldActor->bIsConsumed || (OldActor->bHasCheckpointSnapshot && OldActor->bCheckpointIsConsumed);

				// If save data marks this object as consumed, skip clone spawning ONLY if it has no core geometry.
				// If it has core geometry, we still spawn the clone to render/simulate the core in its broken state.
				if (bIsProxy && bMarkedConsumed && !OldActor->bHasCoreGeometry)
				{
					return;
				}

				if (UWorld* ValidWorld = WeakWorld.Get())
				{
					if (ULevel* ValidLevel = WeakLevel.Get())
					{
						FActorSpawnParameters SpawnParams;
						SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
						SpawnParams.OverrideLevel = ValidLevel;
						SpawnParams.bDeferConstruction = true;
						
						if (AChaosResonanceObj* NewActor = ValidWorld->SpawnActor<AChaosResonanceObj>(ActorClass, SpawnTransform, SpawnParams))
						{
							NewActor->bReset = bSavedReset;
							NewActor->Tags.Add(FName("DynamicallySpawned"));
							
							// Restore C++ instance properties directly
							NewActor->CoreCacheTrackName = OldActor->CoreCacheTrackName;
							NewActor->DebrisCacheTrackName = OldActor->DebrisCacheTrackName;
							NewActor->CollisionChannels = OldActor->CollisionChannels;
							NewActor->bClimbable = OldActor->bClimbable;
							NewActor->ResonanceTags = OldActor->ResonanceTags;
							NewActor->ChaosInteractionSound = OldActor->ChaosInteractionSound;
							NewActor->InteractionSuccessTag = OldActor->InteractionSuccessTag;
							NewActor->LinkedResonanceObj = OldActor->LinkedResonanceObj;
							NewActor->FadeInDuration = OldActor->FadeInDuration;
							NewActor->FadeOutDuration = OldActor->FadeOutDuration;
							NewActor->ProximitySound = OldActor->ProximitySound;
							NewActor->ProximityVFX = OldActor->ProximityVFX;

							NewActor->FinishSpawning(SpawnTransform);

							// FinishSpawning runs construction scripts/BeginPlay on NewActor, which can
							// trigger world-reset or level-streaming churn (e.g. during a New Game / slot
							// transition) that removes OldActor's level from the world mid-flight and
							// destroys it. Re-validate before touching it again to avoid a use-after-free
							if (!IsValid(OldActor))
							{
								return;
							}

							if (bIsProxy)
							{
								// The proxy links to its first clone
								OldActor->ActiveClone = NewActor;
								NewActor->MyProxy = OldActor;

								// If the proxy is already consumed, immediately transition the newly spawned clone to the static core consumed state
								if (bMarkedConsumed)
								{
									NewActor->ApplyConsumedState();
								}
							}
							else
							{
								// A clone is replacing itself. Update the proxy to point to the new clone
								NewActor->MyProxy = OldActor->MyProxy;
								if (IsValid(NewActor->MyProxy))
								{
									NewActor->MyProxy->ActiveClone = NewActor;
								}
							}
						}
					}
				}
				
				if (!bIsProxy)
				{
					// If this was a clone replacing itself (Reset), destroy the old clone
					if (OldActor->CoreGeometryCollectionComponent)
						OldActor->CoreGeometryCollectionComponent->DestroyComponent();
					if (OldActor->DebrisGeometryCollectionComponent)
						OldActor->DebrisGeometryCollectionComponent->DestroyComponent();
						
					OldActor->Destroy();
				}
			}
		});
	}
}

void AChaosResonanceObj::PlayProximityVFX(AActor* OtherActor)
{
	if (IsValid(ActiveClone))
	{
		ActiveClone->PlayProximityVFX(OtherActor);
		return;
	}

	if (!OtherActor || OtherActor == this || IsHidden() || bIsConsumed)
		return;

	if (ProximityVFX && OtherActor->IsA(ACharacter::StaticClass()))
	{
		if (ProximityNiagaraComponent)
		{
			ProximityNiagaraComponent->SetAsset(ProximityVFX);
			ProximityNiagaraComponent->Activate(true);
		}
	}
}

void AChaosResonanceObj::StopProximityVFX(AActor* OtherActor)
{
	if (IsValid(ActiveClone))
	{
		ActiveClone->StopProximityVFX(OtherActor);
		return;
	}

	if (!OtherActor)
		return;

	if (ProximityNiagaraComponent && OtherActor->IsA(ACharacter::StaticClass()))
	{
		ProximityNiagaraComponent->Deactivate();
	}
}

void AChaosResonanceObj::ApplyConsumedState()
{
	bIsConsumed = true;

	// Disable outline highlighting
	SetHighlight(false);
	
	// Disable proximity overlaps and interactions
	if (SphereComponent)
	{
		SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SphereComponent->SetGenerateOverlapEvents(false);
	}
	if (IndicatorComponent)
	{
		IndicatorComponent->UnregisterIndicator();
	}
	if (OutlineMeshComponent)
	{
		OutlineMeshComponent->SetVisibility(false);
	}
	
	// Deactivate any Niagaras (resonance, proximity VFX)
	if (NiagaraComponent)
	{
		NiagaraComponent->Deactivate();
	}
	if (ProximityNiagaraComponent)
	{
		ProximityNiagaraComponent->Deactivate();
	}

	// Debris geometry collection is hidden permanently
	if (DebrisGeometryCollectionComponent)
	{
		DebrisGeometryCollectionComponent->SetVisibility(false, true);
		DebrisGeometryCollectionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// Keep core geometry collection visible and force its Chaos Cache evaluation to the end of the simulation sequence
	if (CoreGeometryCollectionComponent)
	{
		CoreGeometryCollectionComponent->SetVisibility(true, true);
		CoreGeometryCollectionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		SetStartTime(999.0f); // Force evaluation at end time
		SetActorTickEnabled(true);
		TriggerAll();
	}
	
	if (!bHasCoreGeometry)
	{
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);
	}

	SetActorTickEnabled(false);
}
#pragma endregion