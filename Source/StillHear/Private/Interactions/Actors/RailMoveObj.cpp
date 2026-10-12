#include "Interactions/Actors/RailMoveObj.h"

#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/Character.h"
#include "SaveSystem/SaveSubsystem.h"
#include "Components/AudioComponent.h"
#include "Components/SplineComponent.h"

#pragma region CONSTRUCTOR
ARailMoveObj::ARailMoveObj()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
}
#pragma endregion

#pragma region METHODS
void ARailMoveObj::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	if (!RailActor || !RailActor->GetSpline())
		return;

	// Snap the actor to the nearest point on the spline in the editor
	const float ClosestDistanceOnRail = RailActor->GetSpline()->GetDistanceAlongSplineAtLocation(GetActorLocation(), ESplineCoordinateSpace::World);
	UpdateLocationOnRail(ClosestDistanceOnRail);
}

void ARailMoveObj::BeginPlay()
{
	Super::BeginPlay();
	
	if (!RailActor || !RailActor->GetSpline())
		return;
	
	const USplineComponent* Rail = RailActor->GetSpline();
	CurrentDistance = Rail->GetDistanceAlongSplineAtLocation(GetActorLocation(), ESplineCoordinateSpace::World);
	SplineLength = Rail->GetSplineLength();
	
	// Determine initial direction based on starting position
	if (CurrentDistance >= SplineLength - 1.0f)
		MoveDirection = -1.0f;
	else
		MoveDirection = 1.0f;

	if (MovementCurve)
	{
		FOnTimelineFloat TimelineProgress;
		TimelineProgress.BindUFunction(this, FName("HandleMovementProgress"));
		MovementTimeline.AddInterpFloat(MovementCurve, TimelineProgress);
		
		FOnTimelineEvent TimelineFinished;
		TimelineFinished.BindUFunction(this, FName("HandleMovementFinished"));
		MovementTimeline.SetTimelineFinishedFunc(TimelineFinished);
	}
}

void ARailMoveObj::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (!bIsMoving || !RailActor || !RailActor->GetSpline() || !MovementCurve)
		return;
	
	if (MovementTimeline.IsPlaying())
		MovementTimeline.TickTimeline(DeltaSeconds);
}

void ARailMoveObj::UpdateLocationOnRail(const float Distance)
{
	const USplineComponent* Rail = RailActor->GetSpline();
	
	const FVector RailLocation = Rail->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
	FRotator RailRotation = Rail->GetRotationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
	FVector RailUpVector = Rail->GetUpVectorAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
	
	if (!bFollowSlope)
	{
		RailRotation.Pitch = 0.0f;
		RailRotation.Roll = 0.0f;
		RailUpVector = FVector::UpVector;
	}

	const FRotator FinalRotation = (RailRotation.Quaternion() * RotationOffset.Quaternion()).Rotator();
	
	SetActorLocationAndRotation(RailLocation + (RailUpVector * HeightOffset), FinalRotation);
}

void ARailMoveObj::HandleMovementProgress(const float Value)
{
	CurrentDistance = Value * SplineLength;
	UpdateLocationOnRail(CurrentDistance);
}

void ARailMoveObj::HandleMovementFinished()
{
	bIsMoving = false;
	MoveDirection = (CurrentDistance >= SplineLength * 0.5f) ? -1.0f : 1.0f;
	SetActorTickEnabled(false);
	
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
	if (SpawnedNiagara)
		SpawnedNiagara->Deactivate();
		
	if (ArriveSound)
	{
		UAudioComponent* ArriveAudio = UGameplayStatics::SpawnSoundAtLocation(this, ArriveSound, GetActorLocation(), FRotator::ZeroRotator, 1.0f, 1.0f, 0.0f, nullptr, nullptr, true);
		if (IsValid(ArriveAudio))
		{
			if (FadeInDuration > 0.0f)
			{
				ArriveAudio->FadeIn(FadeInDuration, 1.0f);
			}
			else
			{
				ArriveAudio->Play();
			}
		}
	}
	if (ArriveEffect)
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ArriveEffect, GetActorLocation());

	if (CurrentDistance >= SplineLength - 1.0f)
	{
		SaveCheckpointStateAtRailEnd();

		if (const UWorld* World = GetWorld())
		{
			if (UGameInstance* GI = World->GetGameInstance())
			{
				if (USaveSubsystem* SaveSubsystem = GI->GetSubsystem<USaveSubsystem>())
				{
					if (SaveSubsystem->GetCurrentSlotSave() > 0)
					{
						SaveSubsystem->RequestSaveSlotAsync(SaveSubsystem->GetCurrentSlotSave());
					}
				}
			}
		}
	}

	EndInteraction();
}
#pragma endregion

#pragma region INTERFACE METHODS
void ARailMoveObj::TriggerInteraction(AActor* Triggerer)
{
	if (bIsMoving || !RailActor || !RailActor->GetSpline())
		return;

	// Broadcast the triggered-by event without re-entering StartInteraction
	OnTriggeredBy.Broadcast(Triggerer);

	if (ActivationType == ERailActivationType::PlayerOnly || ActivationType == ERailActivationType::None)
		return;

	bIsMoving = true;
	bIsInteracting = true;
	SetActorTickEnabled(true);

	if (StartMoveSound)
	{
		UAudioComponent* StartAudio = UGameplayStatics::SpawnSoundAtLocation(this, StartMoveSound, GetActorLocation(), FRotator::ZeroRotator, 1.0f, 1.0f, 0.0f, nullptr, nullptr, true);
		if (IsValid(StartAudio))
		{
			if (FadeInDuration > 0.0f)
			{
				StartAudio->FadeIn(FadeInDuration, 1.0f);
			}
			else
			{
				StartAudio->Play();
			}
		}
	}
	if (StartMoveEffect)
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, StartMoveEffect, GetActorLocation());

	if (MoveSound)
	{
		if (!IsValid(SpawnedAudio))
			SpawnedAudio = UGameplayStatics::SpawnSoundAttached(MoveSound, RootComponent, NAME_None, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, true, 1.0f);
		
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
	}
	if (MoveEffect)
	{
		if (!SpawnedNiagara)
			SpawnedNiagara = UNiagaraFunctionLibrary::SpawnSystemAttached(MoveEffect, RootComponent, NAME_None, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, true);
		else
			SpawnedNiagara->Activate(true);
	}

	if (MovementCurve)
	{
		MovementTimeline.SetPlayRate(1.0f);

		const float StartTime = (CurrentDistance / FMath::Max(1.0f, SplineLength)) * MovementTimeline.GetTimelineLength();
		MovementTimeline.SetPlaybackPosition(StartTime, false);

		if (MoveDirection > 0.0f)
			MovementTimeline.Play();
		else
			MovementTimeline.Reverse();
	}
}

void ARailMoveObj::ExecuteStartInteraction(const TObjectPtr<ACharacter> Interactor)
{
	Super::ExecuteStartInteraction(Interactor);

	if (ActivationType == ERailActivationType::TriggerOnly || ActivationType == ERailActivationType::None)
	{
		// End interaction immediately if we aren't moving, so we don't get stuck in interacting state
		EndInteraction();
		return;
	}

	// Start movement along the rail when the player directly interacts
	if (!bIsMoving && RailActor && RailActor->GetSpline())
	{
		bIsMoving = true;
		SetActorTickEnabled(true);

		if (StartMoveSound)
		{
			UAudioComponent* StartAudio = UGameplayStatics::SpawnSoundAtLocation(this, StartMoveSound, GetActorLocation(), FRotator::ZeroRotator, 1.0f, 1.0f, 0.0f, nullptr, nullptr, true);
			if (IsValid(StartAudio))
			{
				if (FadeInDuration > 0.0f)
				{
					StartAudio->FadeIn(FadeInDuration, 1.0f);
				}
				else
				{
					StartAudio->Play();
				}
			}
		}
		if (StartMoveEffect)
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, StartMoveEffect, GetActorLocation());

		if (MoveSound)
		{
			if (!IsValid(SpawnedAudio))
				SpawnedAudio = UGameplayStatics::SpawnSoundAttached(MoveSound, RootComponent, NAME_None, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, true, 1.0f);
			
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
		}
		if (MoveEffect)
		{
			if (!SpawnedNiagara)
				SpawnedNiagara = UNiagaraFunctionLibrary::SpawnSystemAttached(MoveEffect, RootComponent, NAME_None, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, true);
			else
				SpawnedNiagara->Activate(true);
		}

		if (MovementCurve)
		{
			MovementTimeline.SetPlayRate(1.0f);

			const float StartTime = (CurrentDistance / FMath::Max(1.0f, SplineLength)) * MovementTimeline.GetTimelineLength();
			MovementTimeline.SetPlaybackPosition(StartTime, false);

			if (MoveDirection > 0.0f)
				MovementTimeline.Play();
			else
				MovementTimeline.Reverse();
		}
	}

	// If movement couldn't start, reset immediately to prevent getting stuck
	if (bIsInteracting && !bIsMoving)
	{
		EndInteraction();
	}
}

void ARailMoveObj::EndInteraction(const TObjectPtr<ACharacter> Interactor)
{
	Super::EndInteraction(Interactor);
}

void ARailMoveObj::SaveCheckpointStateAtRailEnd()
{
	if (!bResetObj) 
		return;

	bHasCheckpointSnapshot = true;
	FVector TargetLocation = GetActorLocation();
	FRotator TargetRotation = GetActorRotation();

	if (RailActor && RailActor->GetSpline())
	{
		const USplineComponent* Rail = RailActor->GetSpline();
		const float Length = Rail->GetSplineLength();
		FVector RailUpVector = Rail->GetUpVectorAtDistanceAlongSpline(Length, ESplineCoordinateSpace::World);
		TargetLocation = Rail->GetLocationAtDistanceAlongSpline(Length, ESplineCoordinateSpace::World) + (RailUpVector * HeightOffset);
		FRotator RailRotation = Rail->GetRotationAtDistanceAlongSpline(Length, ESplineCoordinateSpace::World);

		if (!bFollowSlope)
		{
			RailRotation.Pitch = 0.0f;
			RailRotation.Roll = 0.0f;
			RailUpVector = FVector::UpVector;
		}

		TargetRotation = (RailRotation.Quaternion() * RotationOffset.Quaternion()).Rotator();
	}

	CheckpointLocation = TargetLocation;
	CheckpointRotation = TargetRotation;
	bCheckpointIsConsumed = bIsConsumed;
	bCheckpointIsInteracting = bIsInteracting;
	bCheckpointIsHidden = IsHidden();
	CheckpointInteractableTags = InteractableTags;
}
#pragma endregion
