#include "Interactions/Actors/DrivableObj.h"

#include "AbilitySystemGlobals.h"
#include "StillHearGameInstance.h"
#include "AbilitySystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Components/AudioComponent.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "TraceAndCollision/CustomCollision.h"
#include "GameplayAbilitySystem/Tags/GameplayTags.h"

#pragma region CONSTRUCTOR
ADrivableObj::ADrivableObj()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	bOn = false;

	DrivablePartsRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DrivablePartsRoot"));
	DrivablePartsRoot->SetupAttachment(RootComponent);
}
#pragma endregion

#pragma region METHODS
void ADrivableObj::BeginPlay()
{
	Super::BeginPlay();
	
	if (DrivablePartsRoot)
		RootInitialTransform = DrivablePartsRoot->GetRelativeTransform();

	// Resolve all mesh references and cache their starting transforms
	ResolvedMeshes.SetNum(DrivableParts.Num());
	OriginalStartTransforms.SetNum(DrivableParts.Num());
	OriginalEndTransforms.SetNum(DrivableParts.Num());

	for (int32 i = 0; i < DrivableParts.Num(); i++)
	{
		FDrivablePart& Part = DrivableParts[i];
		
		UPrimitiveComponent* Mesh = Cast<UPrimitiveComponent>(Part.MeshComponent.GetComponent(this));
		ResolvedMeshes[i] = Mesh;
		
		if (!Mesh)
			continue;
		
		Part.StartTransform = Mesh->GetRelativeTransform();
		OriginalStartTransforms[i] = Part.StartTransform;
		OriginalEndTransforms[i] = Part.EndTransform;
		
		Mesh->SetMobility(EComponentMobility::Movable);
		Mesh->SetCollisionObjectType(ECustomCollision::Interactable);
	}

	if (InteractNiagaraComponent && NiagaraVFXPartIndex >= 0 && ResolvedMeshes.IsValidIndex(NiagaraVFXPartIndex) && IsValid(ResolvedMeshes[NiagaraVFXPartIndex]))
	{
		InteractNiagaraComponent->AttachToComponent(
			ResolvedMeshes[NiagaraVFXPartIndex],
			FAttachmentTransformRules::KeepWorldTransform
		);
	}

	if (!TransformCurve)
		return;
	
	// Pre-compute the delta between start and end for each part
	for (int32 i = 0; i < DrivableParts.Num(); i++)
	{
		if (!IsValid(ResolvedMeshes[i]))
			continue;

		FDrivablePart& Part = DrivableParts[i];
		Part.DeltaTransform.SetLocation(Part.EndTransform.GetLocation() - Part.StartTransform.GetLocation());
		Part.DeltaTransform.SetRotation((Part.EndTransform.Rotator() - Part.StartTransform.Rotator()).Quaternion());
		Part.DeltaTransform.SetScale3D(Part.EndTransform.GetScale3D() - Part.StartTransform.GetScale3D());
	}
	
	// Bind timeline callbacks
	FOnTimelineFloat TimelineProgress;
	TimelineProgress.BindUFunction(this, FName("HandleTimelineProgress"));
	TransformTimeLine.AddInterpFloat(TransformCurve, TimelineProgress);
	
	FOnTimelineEvent TimelineFinished;
	TimelineFinished.BindUFunction(this, FName("HandleTimelineFinished"));
	TransformTimeLine.SetTimelineFinishedFunc(TimelineFinished);
}

void ADrivableObj::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (TransformTimeLine.IsPlaying())
		TransformTimeLine.TickTimeline(DeltaSeconds);
}
#pragma endregion

#pragma region OVERRIDE METHODS
void ADrivableObj::ExecuteStartInteraction(const TObjectPtr<ACharacter> Interactor)
{
	if (TransformTimeLine.IsPlaying())
		return;
	
	Super::ExecuteStartInteraction(Interactor);

	// If no curve is assigned, skip animation and finish immediately
	if (!TransformCurve)
	{
		CurrentInteractor = Interactor;
		HandleTimelineFinished();
		return;
	}

	CurrentInteractor = Interactor;
	SetActorTickEnabled(true);
	
	if (bInteractBackwards)
	{
		if (!bOn)
		{
			TransformTimeLine.SetPlayRate(1.0f);
			TransformTimeLine.PlayFromStart();
			
			if (IsValid(SpawnedAudio))
			{
				if (FadeOutDuration > 0.0f)
				{
					SpawnedAudio->FadeOut(FadeOutDuration, 0.0f);
				}
				else
				{
					SpawnedAudio->Stop();
				}
			}
			SpawnedAudio = UGameplayStatics::SpawnSoundAtLocation(this, DriveInteractionSound, GetActorLocation(), FRotator::ZeroRotator, 1.0f, 1.0f, 0.0f, nullptr, nullptr, true);
			if (IsValid(SpawnedAudio))
			{
				if (FadeInDuration > 0.0f)
				{
					SpawnedAudio->FadeIn(FadeInDuration, 1.0f);
				}
				else
				{
					SpawnedAudio->Play();
				}
			}
			
			if (DriveInteractionVFX && InteractNiagaraComponent)
			{
				InteractNiagaraComponent->Deactivate();
				InteractNiagaraComponent->SetAsset(DriveInteractionVFX);
				InteractNiagaraComponent->Activate(true);
			}
		}
		else
		{
			TransformTimeLine.SetPlayRate(BackwardCurveRate);
			TransformTimeLine.Reverse();
			
			if (IsValid(SpawnedAudio))
			{
				if (FadeOutDuration > 0.0f)
				{
					SpawnedAudio->FadeOut(FadeOutDuration, 0.0f);
				}
				else
				{
					SpawnedAudio->Stop();
				}
			}
			SpawnedAudio = UGameplayStatics::SpawnSoundAtLocation(this, DriveBackInteractionSound, GetActorLocation(), FRotator::ZeroRotator, 1.0f, 1.0f, 0.0f, nullptr, nullptr, true);
			if (IsValid(SpawnedAudio))
			{
				if (FadeInDuration > 0.0f)
				{
					SpawnedAudio->FadeIn(FadeInDuration, 1.0f);
				}
				else
				{
					SpawnedAudio->Play();
				}
			}
			
			if (DriveBackInteractionVFX && InteractNiagaraComponent)
			{
				InteractNiagaraComponent->Deactivate();
				InteractNiagaraComponent->SetAsset(DriveBackInteractionVFX);
				InteractNiagaraComponent->Activate(true);
			}
		}

		bOn = !bOn;
	}
	else
	{
		// Accumulate transforms for consecutive non-reversing interactions
		for (int32 i = 0; i < DrivableParts.Num(); i++)
		{
			if (!ResolvedMeshes.IsValidIndex(i) || !IsValid(ResolvedMeshes[i]))
				continue;

			FDrivablePart& Part = DrivableParts[i];
			Part.StartTransform = ResolvedMeshes[i]->GetRelativeTransform();

			FTransform NewEndTransform;
			NewEndTransform.SetLocation(Part.StartTransform.GetLocation() + Part.DeltaTransform.GetLocation());
			NewEndTransform.SetRotation((Part.StartTransform.Rotator() + Part.DeltaTransform.Rotator()).Quaternion());
			NewEndTransform.SetScale3D(Part.StartTransform.GetScale3D() + Part.DeltaTransform.GetScale3D());
			Part.EndTransform = NewEndTransform;
		}
		
		TransformTimeLine.SetPlayRate(1.0f);
		TransformTimeLine.PlayFromStart();
		
		if (IsValid(SpawnedAudio))
		{
			if (FadeOutDuration > 0.0f)
			{
				SpawnedAudio->FadeOut(FadeOutDuration, 0.0f);
			}
			else
			{
				SpawnedAudio->Stop();
			}
		}
		SpawnedAudio = UGameplayStatics::SpawnSoundAtLocation(this, DriveInteractionSound, GetActorLocation(), FRotator::ZeroRotator, 1.0f, 1.0f, 0.0f, nullptr, nullptr, true);
		if (IsValid(SpawnedAudio))
		{
			if (FadeInDuration > 0.0f)
			{
				SpawnedAudio->FadeIn(FadeInDuration, 1.0f);
			}
			else
			{
				SpawnedAudio->Play();
			}
		}
			
		if (DriveInteractionVFX && InteractNiagaraComponent)
		{
			InteractNiagaraComponent->Deactivate();
			InteractNiagaraComponent->SetAsset(DriveInteractionVFX);
			InteractNiagaraComponent->Activate(true);
		}
	}
}

void ADrivableObj::EndInteraction(const TObjectPtr<ACharacter> Interactor)
{
	Super::EndInteraction(Interactor);
	
	if (IsValid(SpawnedAudio))
	{
		if (FadeOutDuration > 0.0f)
		{
			SpawnedAudio->FadeOut(FadeOutDuration, 0.0f);
		}
		else
		{
			SpawnedAudio->Stop();
		}
		SpawnedAudio = nullptr;
	}
	
	// If configured to stop on end, halt movement and notify the player via GAS
	if (bStopDriveOnEnd && TransformTimeLine.IsPlaying())
	{
		TransformTimeLine.Stop();
		SetActorTickEnabled(false);

		UAbilitySystemComponent* InteractorAsc = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Interactor.Get());
		if (!InteractorAsc)
			return;

		FGameplayEventData PayloadData;
		PayloadData.EventTag = TAG_Event_StopInteraction;

		InteractorAsc->HandleGameplayEvent(PayloadData.EventTag, &PayloadData);
	}
}

void ADrivableObj::Reset()
{
	Super::Reset();
	
	if (IsValid(SpawnedAudio))
	{
		SpawnedAudio->Stop();
		SpawnedAudio = nullptr;
	}
	
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
	
	if (TransformTimeLine.IsPlaying())
		TransformTimeLine.Stop();
	
	const bool bFromSnapshot = bHasDrivableCheckpointSnapshot;
	const bool bTargetOn = bFromSnapshot ? bCheckpointOn : false;
	const FTransform& TargetRootTransform = bFromSnapshot ? CheckpointRootTransform : RootInitialTransform;

	bOn = bTargetOn;

	if (DrivablePartsRoot)
		DrivablePartsRoot->SetRelativeTransform(TargetRootTransform);
	
	// Restore all parts to their cached start transforms or checkpoint transforms
	for (int32 i = 0; i < DrivableParts.Num(); i++)
	{
		if (!ResolvedMeshes.IsValidIndex(i) || !IsValid(ResolvedMeshes[i]))
			continue;

		if (bFromSnapshot && CheckpointPartsTransforms.IsValidIndex(i))
		{
			ResolvedMeshes[i]->SetRelativeTransform(CheckpointPartsTransforms[i]);
		}
		else
		{
			// Restore the original transforms in the DrivableParts array
			if (OriginalStartTransforms.IsValidIndex(i))
			{
				DrivableParts[i].StartTransform = OriginalStartTransforms[i];
			}
			if (OriginalEndTransforms.IsValidIndex(i))
			{
				DrivableParts[i].EndTransform = OriginalEndTransforms[i];
			}
			ResolvedMeshes[i]->SetRelativeTransform(DrivableParts[i].StartTransform);
		}
	}
}

void ADrivableObj::SaveCheckpointState()
{
	Super::SaveCheckpointState();
	if (!bResetObj)
		return;

	bHasDrivableCheckpointSnapshot = true;
	bCheckpointOn = bOn;
	
	if (DrivablePartsRoot)
		CheckpointRootTransform = DrivablePartsRoot->GetRelativeTransform();
	
	CheckpointPartsTransforms.SetNum(DrivableParts.Num());
	const bool bIsCurrentlyMoving = TransformTimeLine.IsPlaying();

	for (int32 i = 0; i < DrivableParts.Num(); i++)
	{
		if (ResolvedMeshes.IsValidIndex(i) && IsValid(ResolvedMeshes[i]))
		{
			if (bIsCurrentlyMoving)
			{
				if (bInteractBackwards)
				{
					if (bOn)
					{
						CheckpointPartsTransforms[i] = OriginalEndTransforms.IsValidIndex(i) ? OriginalEndTransforms[i] : DrivableParts[i].EndTransform;
					}
					else
					{
						CheckpointPartsTransforms[i] = OriginalStartTransforms.IsValidIndex(i) ? OriginalStartTransforms[i] : DrivableParts[i].StartTransform;
					}
				}
				else
				{
					CheckpointPartsTransforms[i] = DrivableParts[i].EndTransform;
				}
			}
			else
			{
				CheckpointPartsTransforms[i] = ResolvedMeshes[i]->GetRelativeTransform();
			}
		}
	}
}

void ADrivableObj::ClearCheckpointState()
{
	Super::ClearCheckpointState();
	bHasDrivableCheckpointSnapshot = false;
}
#pragma endregion

#pragma region UFUNCTIONS
void ADrivableObj::HandleTimelineProgress(const float Value) const
{
	for (int32 i = 0; i < DrivableParts.Num(); i++)
	{
		if (!ResolvedMeshes.IsValidIndex(i) || !IsValid(ResolvedMeshes[i]))
			continue;

		FTransform CurrentTransform;
		CurrentTransform.Blend(DrivableParts[i].StartTransform, DrivableParts[i].EndTransform, Value);
		ResolvedMeshes[i]->SetRelativeTransform(CurrentTransform);
	}
}

void ADrivableObj::HandleTimelineFinished()
{
	SetActorTickEnabled(false);

	OnInteractionFinished.Broadcast();
	
	if (IsValid(SpawnedAudio))
	{
		if (FadeOutDuration > 0.0f)
		{
			SpawnedAudio->FadeOut(FadeOutDuration, 0.0f);
		}
		else
		{
			SpawnedAudio->Stop();
		}
		SpawnedAudio = nullptr;
	}
	
	if (FinishInteractionSound)
	{
		UAudioComponent* FinishAudio = UGameplayStatics::SpawnSoundAtLocation(this, FinishInteractionSound, GetActorLocation(), FRotator::ZeroRotator, 1.0f, 1.0f, 0.0f, nullptr, nullptr, true);
		if (IsValid(FinishAudio))
		{
			if (FadeInDuration > 0.0f)
			{
				FinishAudio->FadeIn(FadeInDuration, 1.0f);
			}
			else
			{
				FinishAudio->Play();
			}
		}
	}
	
	if (FinishInteractionVFX && InteractNiagaraComponent)
	{
		InteractNiagaraComponent->Deactivate();
		InteractNiagaraComponent->SetAsset(FinishInteractionVFX);
		InteractNiagaraComponent->Activate(true);
	}
	// Propagate trigger to all linked interactable objects
	for (const TObjectPtr LinkedObj : LinkedTriggeredObjs)
	{
		if (IsValid(LinkedObj) && LinkedObj->Implements<UInteractable>())
			LinkedObj->TriggerInteraction(this);
	}

	// Notify the player's GAS that the interaction is complete
	UAbilitySystemComponent* InteractorAsc = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(CurrentInteractor.Get());
	if (InteractorAsc)
	{
		FGameplayEventData PayloadData;
		PayloadData.EventTag = TAG_Event_StopInteraction;

		InteractorAsc->HandleGameplayEvent(PayloadData.EventTag, &PayloadData);
	}

	EndInteraction(CurrentInteractor);
}
#pragma endregion