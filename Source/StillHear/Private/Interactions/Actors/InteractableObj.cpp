#include "Interactions/Actors/InteractableObj.h"

#include "StillHearGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "Components/AudioComponent.h"
#include "SaveSystem/SaveIdComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "VFX/SplineVFXTravelerComponent.h"
#include "UI/Indicator/IndicatorComponent.h"
#include "TraceAndCollision/CustomCollision.h"
#include "Character/FloatingCompanionComponent.h"
#include "GameplayAbilitySystem/Tags/GameplayTags.h"
#include "Interactions/Components/InteractableShakeComponent.h"
#include "Interactions/Components/CompanionInteractionSpotComponent.h"

#pragma region CONSTRUCTOR
AInteractableObj::AInteractableObj()
{
	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	SetRootComponent(DefaultSceneRoot);
	DefaultSceneRoot->SetMobility(EComponentMobility::Movable);

	SaveIdComponent = CreateDefaultSubobject<USaveIdComponent>(TEXT("SaveIdComponent"));

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(RootComponent);
	StaticMeshComponent->SetCollisionObjectType(ECustomCollision::Interactable);
	
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComponent->SetupAttachment(RootComponent);
	SphereComponent->SetGenerateOverlapEvents(true);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	
	InteractNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("InteractNiagaraComponent"));
	InteractNiagaraComponent->SetupAttachment(RootComponent);
	InteractNiagaraComponent->SetAutoActivate(false);
	
	ProximityNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ProximityNiagaraComponent"));
	ProximityNiagaraComponent->SetupAttachment(RootComponent);
	ProximityNiagaraComponent->SetAutoActivate(false);
	
	IndicatorComponent = CreateDefaultSubobject<UIndicatorComponent>(TEXT("IndicatorComponent"));
	IndicatorComponent->bAutoRegister = false;

	ShakeComponent = CreateDefaultSubobject<UInteractableShakeComponent>(TEXT("ShakeComponent"));
	
	PrimaryActorTick.bCanEverTick = true;
}
#pragma endregion

#pragma region METHODS
void AInteractableObj::BeginPlay()
{
	Super::BeginPlay();
	
	if (RootComponent)
	{
		RootComponent->SetMobility(EComponentMobility::Movable);
	}
	
	if (!SphereComponent)
		return;

	for (const TEnumAsByte Channel : CollisionChannels)
		SphereComponent->SetCollisionResponseToChannel(Channel, ECR_Overlap);
	
	SphereComponent->OnComponentBeginOverlap.AddUniqueDynamic(this, &AInteractableObj::OnSphereBeginOverlap);
	SphereComponent->OnComponentEndOverlap.AddUniqueDynamic(this, &AInteractableObj::OnSphereEndOverlap);

	// Bind to global reset event
	if (const UWorld* World = GetWorld())
	{
		if (UStillHearGameInstance* GI = World->GetGameInstance<UStillHearGameInstance>())
		{
			GI->OnRequestWorldReset.AddUObject(this, &AInteractableObj::Reset);
			GI->OnCheckpointSnapshot.AddUObject(this, &AInteractableObj::SaveCheckpointState);
			GI->OnClearCheckpointState.AddUObject(this, &AInteractableObj::ClearCheckpointState);
		}
	}

	OriginalInteractableTags = InteractableTags; // Cache the original tags to allow resetting them later
	
	if (IndicatorComponent)
	{
		OriginalPromptActions = IndicatorComponent->InputActions;
		OriginalSeparatorClass = IndicatorComponent->SeparatorClass;
	}
		
	OriginalLocation = GetActorLocation();
	OriginalRotation = GetActorRotation();

	if (bDestroyOnEnd && DissolveCurve)
	{
		// Resolve dissolve targets
		if (DissolveTargets.Num() > 0)
		{
			for (const FComponentReference& Ref : DissolveTargets)
			{
				if (UMeshComponent* Mesh = Cast<UMeshComponent>(Ref.GetComponent(this)))
				{
					ResolvedDissolveMeshes.Add(Mesh);
				}
			}
		}
		
		// Fallback to main mesh if no targets specified
		if (ResolvedDissolveMeshes.Num() == 0 && StaticMeshComponent)
		{
			ResolvedDissolveMeshes.Add(Cast<UMeshComponent>(StaticMeshComponent));
		}

		FOnTimelineFloat ProgressFunction;
		ProgressFunction.BindUFunction(this, FName("HandleDissolveUpdate"));
		DissolveTimeline.AddInterpFloat(DissolveCurve, ProgressFunction);

		FOnTimelineEvent FinishedFunction;
		FinishedFunction.BindUFunction(this, FName("HandleDissolveFinished"));
		DissolveTimeline.SetTimelineFinishedFunc(FinishedFunction);

		// Ensure the object starts fully visible (1.0 = full, 0.0 = dissolved)
		HandleDissolveUpdate(1.0f);
	}

	SetActorHiddenInGame(false);
}

void AInteractableObj::EndPlay(const EEndPlayReason::Type EndPlayReason)
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
	
	Super::EndPlay(EndPlayReason);
}

void AInteractableObj::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (DissolveTimeline.IsPlaying())
	{
		DissolveTimeline.TickTimeline(DeltaTime);
	}
}

void AInteractableObj::SetHighlight(const bool bEnableHighlight) const
{
	if (!HighlightMaterial)
		return;
	
	TArray<UMeshComponent*> MeshComponents;
	GetComponents<UMeshComponent>(MeshComponents); // Get all mesh components in the actor, including the static mesh component and any additional mesh components that might be added in child classes
	
	for (UMeshComponent* MeshComponent : MeshComponents)
	{
		if (!IsValid(MeshComponent))
			continue;
		
		if (bEnableHighlight)
			MeshComponent->SetOverlayMaterial(HighlightMaterial);
		else
			MeshComponent->SetOverlayMaterial(nullptr);
	}
}

void AInteractableObj::Reset()
{
	bool bForceNewGame = false;
	if (const UWorld* World = GetWorld())
	{
		if (const UStillHearGameInstance* GI = World->GetGameInstance<UStillHearGameInstance>())
		{
			bForceNewGame = GI->bIsNewGameResetting;
		}
	}

	if (!bResetObj && !bForceNewGame)
		return;

	// Guard against stale delegates from actors whose sublevel was hidden/unloaded
	if (!IsValid(this) || !GetWorld())
		return;

	// Determine target state: checkpoint snapshot or initial BeginPlay state
	const bool bFromSnapshot = bHasCheckpointSnapshot;
	const FVector& LocationTarget = bFromSnapshot ? CheckpointLocation : OriginalLocation;
	const FRotator& RotationTarget = bFromSnapshot ? CheckpointRotation : OriginalRotation;
	const bool bConsumedTarget = bFromSnapshot ? bCheckpointIsConsumed : false;
	const bool bInteractingTarget = bFromSnapshot ? bCheckpointIsInteracting : false;
	const bool bHiddenTarget = bFromSnapshot ? bCheckpointIsHidden : false;
	const FGameplayTagContainer& TagsTarget = bFromSnapshot ? CheckpointInteractableTags : OriginalInteractableTags;

	bIsInteracting = bInteractingTarget;
	bIsConsumed = bConsumedTarget;
	GetWorldTimerManager().ClearTimer(PreInteractionTimerHandle);
	PendingInteractor = nullptr;

	if (IsValid(StartInteractionAudioComponent))
	{
		StartInteractionAudioComponent->Stop();
		StartInteractionAudioComponent = nullptr;
	}
	if (IsValid(EndInteractionAudioComponent))
	{
		EndInteractionAudioComponent->Stop();
		EndInteractionAudioComponent = nullptr;
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

	if (ShakeComponent)
		ShakeComponent->StopAllShakes();
	
	DissolveTimeline.Stop();
	DissolveTimeline.SetPlaybackPosition(0.0f, false);
	
	SetActorHiddenInGame(bHiddenTarget);
	SetActorTickEnabled(!bHiddenTarget);
	
	if (bConsumedTarget)
	{
		// Object was consumed at checkpoint: keep it disabled
		if (SphereComponent)
			SphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
		if (StaticMeshComponent)
			StaticMeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	}
	else
	{
		// Object was not consumed: restore full collision
		if (SphereComponent)
		{
			for (const TEnumAsByte Channel : CollisionChannels)
				SphereComponent->SetCollisionResponseToChannel(Channel, ECR_Overlap);
		}
		
		if (StaticMeshComponent)
		{
			StaticMeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
			StaticMeshComponent->SetVisibility(true, true);
		}
	}
	
	InteractableTags = TagsTarget;
	
	SetActorLocation(LocationTarget);
	SetActorRotation(RotationTarget);

	// Reset dissolve visuals only if not consumed
	if (!bConsumedTarget)
	{
		for (TObjectPtr Mesh : ResolvedDissolveMeshes)
		{
			if (Mesh)
			{
				Mesh->SetVisibility(true);
				Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			}
		}
		
		HandleDissolveUpdate(1.0f);
	}
}
#pragma endregion

#pragma region UFUNCTIONS
void AInteractableObj::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this || IsHidden() || bIsConsumed)
		return;

	SetHighlight(true);
	
	if (IndicatorComponent)
		IndicatorComponent->RegisterIndicator();

	if (ShakeComponent)
		ShakeComponent->SetInRange(true);

	PlayProximitySound(OtherActor);
	PlayProximityVFX(OtherActor);
}

void AInteractableObj::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor || OtherActor == this)
		return;
	
	SetHighlight(false);
	
	if (IndicatorComponent)
		IndicatorComponent->UnregisterIndicator();

	if (ShakeComponent)
		ShakeComponent->SetInRange(false);

	StopProximitySound(OtherActor);
	StopProximityVFX(OtherActor);
}

void AInteractableObj::CallStartInteraction()
{
	StartInteraction();
}

void AInteractableObj::CallEndInteraction()
{
	EndInteraction();
}
#pragma endregion

#pragma region INTERFACE METHODS
void AInteractableObj::StartInteraction(TObjectPtr<ACharacter> Interactor)
{
	if (bIsInteracting || GetWorldTimerManager().IsTimerActive(PreInteractionTimerHandle))
		return;

	bIsInteracting = true;

	if (ShakeComponent && ShakeComponent->ShouldPreInteractionShake())
	{
		PendingInteractor = Interactor;
		
		// Lock player movement via GAS tag only during the initial shake
		if (PendingInteractor)
		{
			if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(PendingInteractor))
			{
				ASC->AddLooseGameplayTag(TAG_Status_MainCharacter_ForceMoving);
			}
		}

		ShakeComponent->SetPreInteracting(true);

		FTimerDelegate TimerDel;
		TimerDel.BindUObject(this, &AInteractableObj::ExecuteStartInteraction, PendingInteractor);
		GetWorldTimerManager().SetTimer(PreInteractionTimerHandle, TimerDel, ShakeComponent->GetPreInteractionDuration(), false);
	}
	else
	{
		ExecuteStartInteraction(Interactor);
	}
}

void AInteractableObj::ExecuteStartInteraction(TObjectPtr<ACharacter> Interactor)
{
	// Unlock player movement as the actual interaction starts
	if (Interactor)
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Interactor))
		{
			ASC->RemoveLooseGameplayTag(TAG_Status_MainCharacter_ForceMoving);
		}
	}

	PendingInteractor = nullptr;

	if (IndicatorComponent)
		IndicatorComponent->UnregisterIndicator();
	
	if (bDestroyOnEnd)
		bIsConsumed = true;
	
	OnInteractionStarted.Broadcast();
	
	if (StartInteractionVFX && InteractNiagaraComponent)
	{
		InteractNiagaraComponent->SetAsset(StartInteractionVFX);
		InteractNiagaraComponent->Activate(true);
	}
	
	if (ProximityNiagaraComponent)
	{
		ProximityNiagaraComponent->Deactivate();
	}
	
	if (IsValid(StartInteractionAudioComponent))
	{
		StartInteractionAudioComponent->Stop();
	}
	if (StartInteractionSound)
	{
		StartInteractionAudioComponent = UGameplayStatics::SpawnSoundAtLocation(this, StartInteractionSound, GetActorLocation(), FRotator::ZeroRotator, 1.0f, 1.0f, 0.0f, nullptr, nullptr, true);
		if (IsValid(StartInteractionAudioComponent))
		{
			if (FadeInDuration > 0.0f)
			{
				StartInteractionAudioComponent->FadeIn(FadeInDuration, 1.0f);
			}
			else
			{
				StartInteractionAudioComponent->Play();
			}
		}
	}

	if (ShakeComponent)
		ShakeComponent->SetInteracting(true);

	if (UCompanionInteractionSpotComponent* Spot = FindComponentByClass<UCompanionInteractionSpotComponent>())
	{
		if (Interactor)
		{
			if (UFloatingCompanionComponent* Companion = Interactor->FindComponentByClass<UFloatingCompanionComponent>())
			{
				Companion->EngageInteractionSpot(Spot);
			}
		}
	}

	// Trigger any linked Spline VFX Traveler components (only on direct interaction, not chain triggers)
	if (!bIsTriggeredByChain)
	{
		for (const FSplineVFXTravelerTrigger& TriggerData : LinkedVFXTravelers)
		{
			if (TriggerData.TargetActor)
			{
				if (USplineVFXTravelerComponent* Traveler = TriggerData.TargetActor->FindComponentByClass<USplineVFXTravelerComponent>())
				{
					Traveler->TriggerTravel(TriggerData.bReverse);
				}
			}
		}
	}
	bIsTriggeredByChain = false;
}

void AInteractableObj::EndInteraction(TObjectPtr<ACharacter> Interactor)
{
	if (!bIsInteracting)
		return;

	bIsInteracting = false;

	// Unlock player movement if it was locked during pre-shake
	ACharacter* TargetInteractor = Interactor ? Interactor.Get() : PendingInteractor.Get();
	if (TargetInteractor)
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetInteractor))
		{
			ASC->RemoveLooseGameplayTag(TAG_Status_MainCharacter_ForceMoving);
		}
        
		if (UFloatingCompanionComponent* Companion = TargetInteractor->FindComponentByClass<UFloatingCompanionComponent>())
		{
			Companion->DisengageInteractionSpot();
		}
	}

	if (GetWorld())
	{
		GetWorldTimerManager().ClearTimer(PreInteractionTimerHandle);
	}
	PendingInteractor = nullptr;

	OnInteractionStopped.Broadcast();
	
	if (InteractNiagaraComponent)
		InteractNiagaraComponent->Deactivate();

	if (IndicatorComponent && !bIsConsumed && SphereComponent)
	{
		TArray<AActor*> OverlappingActors;
		SphereComponent->GetOverlappingActors(OverlappingActors);
		
		for (AActor* Actor : OverlappingActors)
		{
			if (Actor && Actor != this)
			{
				IndicatorComponent->RegisterIndicator();
				break;
			}
		}
	}
	
	if (ProximityNiagaraComponent && !bIsConsumed && SphereComponent)
	{
		TArray<AActor*> OverlappingActors;
		SphereComponent->GetOverlappingActors(OverlappingActors);
		
		for (AActor* Actor : OverlappingActors)
		{
			if (Actor && Actor != this && Actor->IsA(ACharacter::StaticClass()))
			{
				PlayProximityVFX(Actor);
				break;
			}
		}
	}
	
	if (EndInteractionVFX && InteractNiagaraComponent)
	{
		InteractNiagaraComponent->SetAsset(EndInteractionVFX);
		InteractNiagaraComponent->Activate(true);
	}
	
	if (IsValid(StartInteractionAudioComponent))
	{
		if (StartInteractionSound && StartInteractionSound->IsLooping())
		{
			if (FadeOutDuration > 0.0f)
			{
				StartInteractionAudioComponent->FadeOut(FadeOutDuration, 0.0f);
			}
			else
			{
				StartInteractionAudioComponent->Stop();
			}
		}
	}

	if (IsValid(EndInteractionAudioComponent))
	{
		EndInteractionAudioComponent->Stop();
	}
	if (EndInteractionSound)
	{
		EndInteractionAudioComponent = UGameplayStatics::SpawnSoundAtLocation(this, EndInteractionSound, GetActorLocation(), FRotator::ZeroRotator, 1.0f, 1.0f, 0.0f, nullptr, nullptr, true);
		if (IsValid(EndInteractionAudioComponent))
		{
			if (FadeInDuration > 0.0f)
			{
				EndInteractionAudioComponent->FadeIn(FadeInDuration, 1.0f);
			}
			else
			{
				EndInteractionAudioComponent->Play();
			}
		}
	}

	if (ShakeComponent)
	{
		ShakeComponent->SetInteracting(false);
		ShakeComponent->PlayAfterShake();
	}

	if (bDestroyOnEnd)
	{
		if (DissolveCurve)
		{
			// Disable interactions and collisions immediately
			InteractableTags.Reset();
			if (SphereComponent)
				SphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
			
			// Ensure tick is enabled for the timeline to play
			SetActorTickEnabled(true);
			DissolveTimeline.Play();
		}
		else
		{
			SetActorHiddenInGame(true);
			SetActorTickEnabled(false);
			if (SphereComponent)
				SphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
			if (StaticMeshComponent)
				StaticMeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
		}
	}
}

void AInteractableObj::HandleDissolveUpdate(const float Value)
{
	for (TObjectPtr Mesh : ResolvedDissolveMeshes)
	{
		if (Mesh)
			Mesh->SetScalarParameterValueOnMaterials(DissolveParameterName, Value);
	}
}

void AInteractableObj::HandleDissolveFinished()
{
	if (IndicatorComponent)
		IndicatorComponent->UnregisterIndicator();

	for (TObjectPtr Mesh : ResolvedDissolveMeshes)
	{
		if (Mesh)
		{
			Mesh->SetVisibility(false);
			Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}

	if (bDestroyActorOnDissolveFinished)
	{
		SetActorHiddenInGame(true);
		SetActorTickEnabled(false);
		if (SphereComponent)
			SphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
		if (StaticMeshComponent)
			StaticMeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	}
	else
	{
		SetActorTickEnabled(false);
	}
}

void AInteractableObj::TriggerInteraction(AActor* Triggerer)
{
	OnTriggeredBy.Broadcast(Triggerer);
	bIsTriggeredByChain = true;
	StartInteraction();
}

void AInteractableObj::SaveCheckpointState()
{
	if (!bResetObj)
		return;

	bHasCheckpointSnapshot = true;
	CheckpointLocation = GetActorLocation();
	CheckpointRotation = GetActorRotation();
	bCheckpointIsConsumed = bIsConsumed;
	bCheckpointIsInteracting = bIsInteracting;
	bCheckpointIsHidden = IsHidden();
	CheckpointInteractableTags = InteractableTags;
}

void AInteractableObj::ClearCheckpointState()
{
	bHasCheckpointSnapshot = false;
}

void AInteractableObj::OnPostLoad_Implementation()
{
	Reset();
}

void AInteractableObj::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	TagContainer.AppendTags(InteractableTags);
}

void AInteractableObj::PlayProximitySound(AActor* OtherActor)
{
	if (!OtherActor || OtherActor == this || IsHidden() || bIsConsumed)
		return;

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

void AInteractableObj::StopProximitySound(AActor* OtherActor)
{
	if (!OtherActor)
		return;

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

void AInteractableObj::PlayProximityVFX(AActor* OtherActor)
{
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

void AInteractableObj::StopProximityVFX(AActor* OtherActor)
{
	if (!OtherActor)
		return;

	if (ProximityNiagaraComponent && OtherActor->IsA(ACharacter::StaticClass()))
	{
		ProximityNiagaraComponent->Deactivate();
	}
}
#pragma endregion
