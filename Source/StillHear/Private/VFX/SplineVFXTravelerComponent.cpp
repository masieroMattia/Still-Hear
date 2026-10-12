#include "VFX/SplineVFXTravelerComponent.h"

#include "NiagaraComponent.h"
#include "Curves/CurveFloat.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Components/SplineComponent.h"
#include "Interactions/Actors/InteractableObj.h"

#pragma region CONSTRUCTOR
USplineVFXTravelerComponent::USplineVFXTravelerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}
#pragma endregion

#pragma region OVERRIDE METHODS
void USplineVFXTravelerComponent::BeginPlay()
{
	Super::BeginPlay();
	FindSpline();
}

void USplineVFXTravelerComponent::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (ActiveTravelers.Num() == 0)
		return;

	FindSpline();
	if (!CachedSpline)
		return;

	const float SplineLength = CachedSpline->GetSplineLength();

	for (int32 i = ActiveTravelers.Num() - 1; i >= 0; --i)
	{
		FSplineTravelerInstance& Traveler = ActiveTravelers[i];
		Traveler.ElapsedTime += DeltaTime;

		float NormalizedProgress = 0.0f;
		bool bFinished;

		if (Traveler.InstanceDuration > 0.0f)
		{
			const float TimeAlpha = FMath::Clamp(Traveler.ElapsedTime / Traveler.InstanceDuration, 0.0f, 1.0f);
			
			if (TravelCurve)
			{
				// Use the curve's value as normalized distance (0..1)
				NormalizedProgress = FMath::Clamp(TravelCurve->GetFloatValue(Traveler.ElapsedTime), 0.0f, 1.0f);
			}
			else
			{
				NormalizedProgress = TimeAlpha;
			}

			bFinished = (Traveler.ElapsedTime >= Traveler.InstanceDuration);
		}
		else
		{
			// If duration is 0 we just finish
			bFinished = true;
		}

		// Calculate actual distance on spline based on direction
		const float ActualDistance = Traveler.bIsReversed ? (1.0f - NormalizedProgress) * SplineLength : NormalizedProgress * SplineLength;

		if (bFinished)
		{
			// Final position depends on direction
			const float FinalDistance = Traveler.bIsReversed ? 0.0f : SplineLength;
			const FVector EndLoc = CachedSpline->GetLocationAtDistanceAlongSpline(FinalDistance, ESplineCoordinateSpace::World);
			const FRotator EndRot = CachedSpline->GetRotationAtDistanceAlongSpline(FinalDistance, ESplineCoordinateSpace::World);

			// Spawn End VFX & Sound
			if (EndVFX)
			{
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), EndVFX, EndLoc, EndRot);
			}
			if (EndSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this, EndSound, EndLoc);
			}

			// Clean up traveler VFX
			if (Traveler.VFXComponent)
			{
				Traveler.VFXComponent->Deactivate();
			}

			// Trigger Linked Interactable Objects based on direction
			const TArray<TObjectPtr<AInteractableObj>>& ObjectsToTrigger = Traveler.bIsReversed ? ReverseLinkedObjects : ForwardLinkedObjects;

			for (const TObjectPtr LinkedObj : ObjectsToTrigger)
			{
				if (LinkedObj)
				{
					LinkedObj->TriggerInteraction(GetOwner());
				}
			}

			ActiveTravelers.RemoveAt(i);
		}
		else
		{
			// Update traveler position and rotation
			const FVector CurrentLoc = CachedSpline->GetLocationAtDistanceAlongSpline(ActualDistance, ESplineCoordinateSpace::World);
			const FRotator CurrentRot = CachedSpline->GetRotationAtDistanceAlongSpline(ActualDistance, ESplineCoordinateSpace::World);

			if (Traveler.VFXComponent)
			{
				Traveler.VFXComponent->SetWorldLocationAndRotation(CurrentLoc, CurrentRot);
			}
		}
	}

	UpdateLoopSoundState();
}
#pragma endregion

#pragma region PUBLIC METHODS
void USplineVFXTravelerComponent::TriggerTravel(bool bReverse)
{
	FindSpline();
	if (!CachedSpline)
		return;

	const float SplineLength = CachedSpline->GetSplineLength();
	float InstanceDuration = 0.0f;

	// Determine duration
	if (TravelCurve)
	{
		float MinTime, MaxTime;
		TravelCurve->GetTimeRange(MinTime, MaxTime);
		InstanceDuration = MaxTime;
	}
	else if (FallbackTravelSpeed > 0.0f)
	{
		InstanceDuration = SplineLength / FallbackTravelSpeed;
	}

	// Determine starting point
	const float StartDistance = bReverse ? SplineLength : 0.0f;
	const FVector StartLoc = CachedSpline->GetLocationAtDistanceAlongSpline(StartDistance, ESplineCoordinateSpace::World);
	const FRotator StartRot = CachedSpline->GetRotationAtDistanceAlongSpline(StartDistance, ESplineCoordinateSpace::World);

	// Spawn Start VFX & Sound
	if (StartVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), StartVFX, StartLoc, StartRot);
	}
	if (StartSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, StartSound, StartLoc);
	}

	// Spawn Travel VFX
	UNiagaraComponent* NewVFXComponent = nullptr;
	if (TravelVFX)
	{
		NewVFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), TravelVFX, StartLoc, StartRot);
	}
	
	// Track the traveler instance
	ActiveTravelers.Add(FSplineTravelerInstance(NewVFXComponent, InstanceDuration, bReverse));

	UpdateLoopSoundState();
}
#pragma endregion

#pragma region PRIVATE METHODS
void USplineVFXTravelerComponent::FindSpline()
{
	if (CachedSpline)
		return;

	CachedSpline = GetOwner()->FindComponentByClass<USplineComponent>();
}

void USplineVFXTravelerComponent::UpdateLoopSoundState()
{
	if (ActiveTravelers.Num() > 0)
	{
		if (TravelLoopSound)
		{
			if (!LoopAudioComponent)
			{
				// Spawn looping sound and attach to owner root
				LoopAudioComponent = UGameplayStatics::SpawnSoundAttached(TravelLoopSound, GetOwner()->GetRootComponent());
			}
			else if (!LoopAudioComponent->IsPlaying())
			{
				LoopAudioComponent->Play();
			}
		}
	}
	else
	{
		// Stop looping sound if no travelers are active
		if (LoopAudioComponent && LoopAudioComponent->IsPlaying())
		{
			// Using FadeOut for a smoother transition
			LoopAudioComponent->FadeOut(0.3f, 0.0f);
		}
	}
}
#pragma endregion
