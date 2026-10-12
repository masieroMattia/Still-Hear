#include "Interactions/Components/InteractableLinkTimerComponent.h"

#include "StillHearGameInstance.h"
#include "Interactions/Actors/InteractableObj.h"
#include "Interactions/Actors/TargetableDrivableObj.h"
#include "PCG/Actors/SplineSmoothConnectedMeshes.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"

#pragma region CONSTRUCTOR
UInteractableLinkTimerComponent::UInteractableLinkTimerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}
#pragma endregion

#pragma region METHODS
void UInteractableLinkTimerComponent::BeginPlay()
{
	Super::BeginPlay();

	const int32 Count = LinkedSources.Num();
	ActivatedFlags.Init(false, Count);
	EmissiveCurrentValues.Init(0.0f, Count);
	EmissiveTargetValues.Init(0.0f, Count);
	TimerCurrentValues.Init(0.0f, Count);
	ResolvedSources.SetNum(Count);
	EmissiveMaterials.SetNum(Count);
	TimerMaterials.SetNum(Count);

	for (int32 i = 0; i < Count; ++i)
	{
		// SetNum() above doesn't clear each element's inner Mats array, so repeated BeginPlay calls
		// (level-streaming reloads) kept appending materials instead of replacing them.
		EmissiveMaterials[i].Mats.Empty();
		TimerMaterials[i].Mats.Empty();

		AInteractableObj* Source = LinkedSources[i].Source.Get();
		ResolvedSources[i] = Source;

		if (IsValid(Source))
		{
			Source->OnInteractionStarted.AddUniqueDynamic(this, &UInteractableLinkTimerComponent::OnAnySourceTriggered);
		}

		// Emissive material
		AActor* EmissiveActor = LinkedSources[i].EmissiveMesh.OtherActor.Get() ? LinkedSources[i].EmissiveMesh.OtherActor.Get() : GetOwner();
		if (ASplineSmoothConnectedMeshes* SplineActor = Cast<ASplineSmoothConnectedMeshes>(EmissiveActor))
		{
			TArray<USplineMeshComponent*> SplineMeshes;
			SplineActor->GetComponents<USplineMeshComponent>(SplineMeshes);
			
			if (SplineActor->Spline)
			{
				SplineMeshes.Sort([SplineActor](const USplineMeshComponent& A, const USplineMeshComponent& B) {
					const FVector WorldA = A.GetStartPosition();
					const FVector WorldB = B.GetStartPosition();
					const float DistA = SplineActor->Spline->GetDistanceAlongSplineAtLocation(WorldA, ESplineCoordinateSpace::World);
					const float DistB = SplineActor->Spline->GetDistanceAlongSplineAtLocation(WorldB, ESplineCoordinateSpace::World);
					return DistA < DistB;
				});
			}

			for (USplineMeshComponent* SMC : SplineMeshes)
			{
				// Reuse an already-created dynamic instance instead of always creating a fresh one: recreating
				// the MID on every reload restarts any time-based effect baked into the material graph.
				if (UMaterialInstanceDynamic* ExistingMat = Cast<UMaterialInstanceDynamic>(SMC->GetMaterial(LinkedSources[i].EmissiveMaterialIndex)))
				{
					EmissiveMaterials[i].Mats.Add(ExistingMat);
				}
				else if (UMaterialInstanceDynamic* DynMat = SMC->CreateAndSetMaterialInstanceDynamic(LinkedSources[i].EmissiveMaterialIndex))
				{
					DynMat->SetScalarParameterValue(EmissiveParameterName, 0.0f);
					EmissiveMaterials[i].Mats.Add(DynMat);
				}
			}
		}
		else if (UMeshComponent* Mesh = Cast<UMeshComponent>(LinkedSources[i].EmissiveMesh.GetComponent(GetOwner())))
		{
			if (UMaterialInstanceDynamic* ExistingMat = Cast<UMaterialInstanceDynamic>(Mesh->GetMaterial(LinkedSources[i].EmissiveMaterialIndex)))
			{
				EmissiveMaterials[i].Mats.Add(ExistingMat);
			}
			else if (UMaterialInstanceDynamic* DynMat = Mesh->CreateAndSetMaterialInstanceDynamic(LinkedSources[i].EmissiveMaterialIndex))
			{
				DynMat->SetScalarParameterValue(EmissiveParameterName, 0.0f);
				EmissiveMaterials[i].Mats.Add(DynMat);
			}
		}

		// Timer material
		AActor* TimerActor = LinkedSources[i].TimerMesh.OtherActor.Get() ? LinkedSources[i].TimerMesh.OtherActor.Get() : GetOwner();
		if (const ASplineSmoothConnectedMeshes* SplineActor = Cast<ASplineSmoothConnectedMeshes>(TimerActor))
		{
			TArray<USplineMeshComponent*> SplineMeshes;
			SplineActor->GetComponents<USplineMeshComponent>(SplineMeshes);
			
			if (SplineActor->Spline)
			{
				SplineMeshes.Sort([SplineActor](const USplineMeshComponent& A, const USplineMeshComponent& B) {
					const FVector WorldA = A.GetStartPosition();
					const FVector WorldB = B.GetStartPosition();
					const float DistA = SplineActor->Spline->GetDistanceAlongSplineAtLocation(WorldA, ESplineCoordinateSpace::World);
					const float DistB = SplineActor->Spline->GetDistanceAlongSplineAtLocation(WorldB, ESplineCoordinateSpace::World);
					return DistA < DistB;
				});
			}

			for (USplineMeshComponent* SMC : SplineMeshes)
			{
				// Reuse an existing dynamic instance for the same reason as the emissive material above.
				if (UMaterialInstanceDynamic* ExistingMat = Cast<UMaterialInstanceDynamic>(SMC->GetMaterial(LinkedSources[i].TimerMaterialIndex)))
				{
					TimerMaterials[i].Mats.Add(ExistingMat);
				}
				else if (UMaterialInstanceDynamic* DynMat = SMC->CreateAndSetMaterialInstanceDynamic(LinkedSources[i].TimerMaterialIndex))
				{
					DynMat->SetScalarParameterValue(TimerParameterName, 0.0f);
					TimerMaterials[i].Mats.Add(DynMat);
				}
			}
		}
		else if (UMeshComponent* Mesh = Cast<UMeshComponent>(LinkedSources[i].TimerMesh.GetComponent(GetOwner())))
		{
			if (UMaterialInstanceDynamic* ExistingMat = Cast<UMaterialInstanceDynamic>(Mesh->GetMaterial(LinkedSources[i].TimerMaterialIndex)))
			{
				TimerMaterials[i].Mats.Add(ExistingMat);
			}
			else if (UMaterialInstanceDynamic* DynMat = Mesh->CreateAndSetMaterialInstanceDynamic(LinkedSources[i].TimerMaterialIndex))
			{
				DynMat->SetScalarParameterValue(TimerParameterName, 0.0f);
				TimerMaterials[i].Mats.Add(DynMat);
			}
		}
	}

	if (const UWorld* World = GetWorld())
	{
		if (UStillHearGameInstance* GI = World->GetGameInstance<UStillHearGameInstance>())
		{
			GI->OnRequestWorldReset.AddUObject(this, &UInteractableLinkTimerComponent::Reset);
			GI->OnCheckpointSnapshot.AddUObject(this, &UInteractableLinkTimerComponent::SaveCheckpointState);
			GI->OnClearCheckpointState.AddUObject(this, &UInteractableLinkTimerComponent::ClearCheckpointState);
		}
	}
}

void UInteractableLinkTimerComponent::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Clamp against frame hitches (e.g. a level streaming load stalling the game thread): an unclamped
	// DeltaTime would let FInterpConstantTo/FInterpTo snap almost instantly instead of animating.
	const float ClampedDeltaTime = FMath::Min(DeltaTime, 0.1f);

	const float TimerRemaining = (Timeout > 0.0f) ? FMath::Clamp(1.0f - (GetWorld()->GetTimeSeconds() - TimerStartTime) / Timeout, 0.0f, 1.0f) : 0.0f;

	bool bNeedsTick = false;

	for (int32 i = 0; i < EmissiveCurrentValues.Num(); ++i)
	{
		float TargetTimerVal = 0.0f;
		if (bIsCompleted)
		{
			TargetTimerVal = 1.0f;
		}
		else if (ActivatedFlags[i])
		{
			TargetTimerVal = (Timeout > 0.0f) ? TimerRemaining : 1.0f;
			
			if (Timeout > 0.0f)
				bNeedsTick = true;
		}

		// Interpolate Timer Progress (fills up sequentially on activation, drains on timeout)
		if (!FMath::IsNearlyEqual(TimerCurrentValues[i], TargetTimerVal, KINDA_SMALL_NUMBER))
		{
			const float Speed = FMath::Max(EmissiveInterpSpeed, (Timeout > 0.0f ? 1.0f / Timeout : 1.0f) * 1.5f);
			TimerCurrentValues[i] = FMath::FInterpConstantTo(TimerCurrentValues[i], TargetTimerVal, ClampedDeltaTime, Speed);
			bNeedsTick = true;
		}

		// Sequence logic: Emissive triggers when Timer finishes filling or draining
		if (ActivatedFlags[i])
		{
			// Reached the target or fully filled
			if (TimerCurrentValues[i] >= TargetTimerVal - 0.01f)
				EmissiveTargetValues[i] = LinkedSources[i].EmissiveTargetValue;
		}
		else
		{
			// Finished draining completely
			if (TimerCurrentValues[i] <= 0.01f)
				EmissiveTargetValues[i] = 0.0f;
		}

		// Update Timer Materials
		const int32 NumTimerMats = TimerMaterials[i].Mats.Num();
		if (NumTimerMats == 1 && TimerMaterials[i].Mats[0])
		{
			TimerMaterials[i].Mats[0]->SetScalarParameterValue(TimerParameterName, TimerCurrentValues[i]);
		}
		else if (NumTimerMats > 1)
		{
			const float Mat = TimerCurrentValues[i] * NumTimerMats;
			for (int32 m = 0; m < NumTimerMats; ++m)
			{
				if (TimerMaterials[i].Mats[m])
				{
					const float MatVal = FMath::Clamp(Mat - m, 0.0f, 1.0f);
					TimerMaterials[i].Mats[m]->SetScalarParameterValue(TimerParameterName, MatVal);
				}
			}
		}

		// Interpolate Emissive
		if (!FMath::IsNearlyEqual(EmissiveCurrentValues[i], EmissiveTargetValues[i], KINDA_SMALL_NUMBER))
		{
			EmissiveCurrentValues[i] = FMath::FInterpTo(EmissiveCurrentValues[i], EmissiveTargetValues[i], ClampedDeltaTime, EmissiveInterpSpeed);

			const int32 NumMats = EmissiveMaterials[i].Mats.Num();
			if (NumMats == 1 && EmissiveMaterials[i].Mats[0])
			{
				EmissiveMaterials[i].Mats[0]->SetScalarParameterValue(EmissiveParameterName, EmissiveCurrentValues[i]);
			}
			else if (NumMats > 1)
			{
				const float Mat = EmissiveCurrentValues[i] * NumMats;
				for (int32 m = 0; m < NumMats; ++m)
				{
					if (EmissiveMaterials[i].Mats[m])
					{
						const float MatVal = FMath::Clamp(Mat - m, 0.0f, 1.0f);
						EmissiveMaterials[i].Mats[m]->SetScalarParameterValue(EmissiveParameterName, MatVal);
					}
				}
			}

			bNeedsTick = true;
		}
	}

	if (!bNeedsTick)
		SetComponentTickEnabled(false);
}

void UInteractableLinkTimerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
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

void UInteractableLinkTimerComponent::OnAnySourceTriggered()
{
	for (int32 i = 0; i < ResolvedSources.Num(); ++i)
	{
		if (ActivatedFlags[i])
			continue;

		const AInteractableObj* Source = ResolvedSources[i].Get();
		if (IsValid(Source) && Source->IsInteracting())
		{
			bool bShouldTrigger = true;

			if (const ATargetableDrivableObj* Targetable = Cast<ATargetableDrivableObj>(Source))
			{
				const ELinkTriggerType Type = LinkedSources[i].TriggerType;
				const bool bIsHit = Targetable->IsHitInteraction();

				if (Type == ELinkTriggerType::Interaction && bIsHit)
				{
					bShouldTrigger = false;
				}
				else if (Type == ELinkTriggerType::Hit && !bIsHit)
				{
					bShouldTrigger = false;
				}
			}
			else
			{
				// Standard interactables can't be hit in this logic, so filter out if looking for Hit Only
				if (LinkedSources[i].TriggerType == ELinkTriggerType::Hit)
				{
					bShouldTrigger = false;
				}
			}

			if (bShouldTrigger)
			{
				OnSourceTriggered(i);
			}
		}
	}
}

void UInteractableLinkTimerComponent::OnSourceTriggered(const int32 SourceIndex)
{
	if (!ActivatedFlags.IsValidIndex(SourceIndex))
		return;

	ActivatedFlags[SourceIndex] = true;

	if (Timeout > 0.0f)
		StartTimer();

	SetComponentTickEnabled(true);

	if (AreAllSourcesActive())
	{
		bIsCompleted = true;
		StopTimer();

		if (AInteractableObj* Owner = Cast<AInteractableObj>(GetOwner()))
		{
			Owner->TriggerInteraction(GetOwner());
		}
	}
}

void UInteractableLinkTimerComponent::OnTimeout()
{
	ResetAllSources();
}

bool UInteractableLinkTimerComponent::AreAllSourcesActive() const
{
	for (const bool bActive : ActivatedFlags)
	{
		if (!bActive)
			return false;
	}
	return ActivatedFlags.Num() > 0;
}

void UInteractableLinkTimerComponent::ResetAllSources()
{
	StopTimer();
	bIsCompleted = false;

	for (int32 i = 0; i < ActivatedFlags.Num(); ++i)
	{
		if (ActivatedFlags[i])
		{
			ActivatedFlags[i] = false;

			if (AInteractableObj* Source = ResolvedSources[i].Get())
			{
				if (ATargetableDrivableObj* Targetable = Cast<ATargetableDrivableObj>(Source))
				{
					Targetable->StopHitTarget();
				}
				else
				{
					Source->EndInteraction();
				}
			}
		}
	}

	SetComponentTickEnabled(true);
}

void UInteractableLinkTimerComponent::StartTimer()
{
	TimerStartTime = GetWorld()->GetTimeSeconds();
	GetWorld()->GetTimerManager().SetTimer(TimeoutHandle, this, &UInteractableLinkTimerComponent::OnTimeout, Timeout, false);
	SetComponentTickEnabled(true);
}

void UInteractableLinkTimerComponent::StopTimer()
{
	if (const UWorld* World = GetWorld())
		World->GetTimerManager().ClearTimer(TimeoutHandle);
}

void UInteractableLinkTimerComponent::Reset()
{
	StopTimer();

	const bool bFromSnapshot = bHasTimerCheckpointSnapshot;
	bIsCompleted = bFromSnapshot ? bCheckpointIsCompleted : false;
	
	const int32 Count = ActivatedFlags.Num();
	for (int32 i = 0; i < Count; ++i)
	{
		const bool bNewFlag = bFromSnapshot && CheckpointActivatedFlags.IsValidIndex(i) ? CheckpointActivatedFlags[i] : false;
		
		// When resetting to initial state (no snapshot), end active source interactions
		if (!bFromSnapshot && ActivatedFlags[i])
		{
			if (AInteractableObj* Source = ResolvedSources[i].Get())
			{
				if (ATargetableDrivableObj* Targetable = Cast<ATargetableDrivableObj>(Source))
				{
					Targetable->StopHitTarget();
				}
				else
				{
					Source->EndInteraction();
				}
			}
		}
		
		ActivatedFlags[i] = bNewFlag;

		// If this entry is being deactivated, snap its visuals to the off state immediately instead of
		// leaving Tick to animate there, since a level reload can interrupt a countdown mid-flight.
		if (!bNewFlag)
		{
			if (TimerCurrentValues.IsValidIndex(i))
				TimerCurrentValues[i] = 0.0f;
			if (EmissiveTargetValues.IsValidIndex(i))
				EmissiveTargetValues[i] = 0.0f;
			if (EmissiveCurrentValues.IsValidIndex(i))
				EmissiveCurrentValues[i] = 0.0f;

			if (TimerMaterials.IsValidIndex(i))
			{
				for (UMaterialInstanceDynamic* Mat : TimerMaterials[i].Mats)
				{
					if (Mat)
						Mat->SetScalarParameterValue(TimerParameterName, 0.0f);
				}
			}
			if (EmissiveMaterials.IsValidIndex(i))
			{
				for (UMaterialInstanceDynamic* Mat : EmissiveMaterials[i].Mats)
				{
					if (Mat)
						Mat->SetScalarParameterValue(EmissiveParameterName, 0.0f);
				}
			}
		}
	}

	SetComponentTickEnabled(true);
}

void UInteractableLinkTimerComponent::SaveCheckpointState()
{
	bHasTimerCheckpointSnapshot = true;
	CheckpointActivatedFlags = ActivatedFlags;
	bCheckpointIsCompleted = bIsCompleted;
}

void UInteractableLinkTimerComponent::ClearCheckpointState()
{
	bHasTimerCheckpointSnapshot = false;
}

void UInteractableLinkTimerComponent::OnPostLoad_Implementation()
{
	Reset();
}
#pragma endregion
