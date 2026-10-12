#include "Interactions/Components/InteractableShakeComponent.h"

#include "GameFramework/Actor.h"
#include "Components/MeshComponent.h"

#pragma region CONSTRUCTOR
UInteractableShakeComponent::UInteractableShakeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}
#pragma endregion

#pragma region METHODS
void UInteractableShakeComponent::BeginPlay()
{
	Super::BeginPlay();

	ResolveDefaultTarget();
	CacheAllTargets();
}

void UInteractableShakeComponent::ResolveDefaultTarget()
{
	if (ResolvedDefaultTarget)
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Resolve the DefaultTargetComponent reference if set
	if (DefaultTargetComponent.ComponentProperty != NAME_None)
	{
		ResolvedDefaultTarget = Cast<USceneComponent>(DefaultTargetComponent.GetComponent(Owner));
	}
	
	// Fallback: try to find a MeshComponent or Root
	if (!ResolvedDefaultTarget)
	{
		ResolvedDefaultTarget = Cast<USceneComponent>(Owner->GetComponentByClass(UMeshComponent::StaticClass()));
		if (!ResolvedDefaultTarget)
		{
			ResolvedDefaultTarget = Owner->GetRootComponent();
		}
	}
}

void UInteractableShakeComponent::CacheAllTargets()
{
	AlwaysShakeTargets.Empty();
	InRangeShakeTargets.Empty();
	PreInteractionShakeTargets.Empty();
	InteractingShakeTargets.Empty();
	AfterShakeTargets.Empty();
	DefaultShakeTargets.Empty();

	if (ResolvedDefaultTarget)
	{
		DefaultShakeTargets.Add(ResolvedDefaultTarget);
	}

	CacheTargetsForParams(&AlwaysShake, AlwaysShakeTargets);
	CacheTargetsForParams(&InRangeShake, InRangeShakeTargets);
	CacheTargetsForParams(&PreInteractionShake.Settings, PreInteractionShakeTargets);
	CacheTargetsForParams(&InteractingShake, InteractingShakeTargets);
	CacheTargetsForParams(&AfterShake.Settings, AfterShakeTargets);
}

void UInteractableShakeComponent::CacheTargetsForParams(const FInteractableShakeParams* Params, TArray<TObjectPtr<USceneComponent>>& OutTargets)
{
	AActor* Owner = GetOwner();
	if (!Params || !Owner)
	{
		return;
	}

	if (Params->TargetComponents.Num() > 0)
	{
		for (const FComponentReference& Ref : Params->TargetComponents) 
		{ 
			if (USceneComponent* Resolved = Cast<USceneComponent>(Ref.GetComponent(Owner)))
			{
				OutTargets.Add(Resolved);
			}
		}
	}

	// Fallback to default if no valid targets were resolved
	if (OutTargets.Num() == 0 && ResolvedDefaultTarget)
	{
		OutTargets.Add(ResolvedDefaultTarget);
	}
}

const TArray<TObjectPtr<USceneComponent>>* UInteractableShakeComponent::GetCachedTargets(const FInteractableShakeParams* Params) const
{
	if (!Params)
	{
		return &DefaultShakeTargets;
	}

	if (Params == &AlwaysShake)
		return &AlwaysShakeTargets;
	if (Params == &InRangeShake)
		return &InRangeShakeTargets;
	if (Params == &PreInteractionShake.Settings)
		return &PreInteractionShakeTargets;
	if (Params == &InteractingShake)
		return &InteractingShakeTargets;
	if (Params == &AfterShake.Settings)
		return &AfterShakeTargets;

	return &DefaultShakeTargets;
}

void UInteractableShakeComponent::UpdateTimers(const float DeltaTime)
{
	if (AfterShakeTimer > 0.0f)
	{
		AfterShakeTimer -= DeltaTime;
	}
}

UInteractableShakeComponent::FShakeTargetState UInteractableShakeComponent::CalculateTargetState(const FInteractableShakeParams* Params) const
{
	FShakeTargetState State;

	if (!Params)
	{
		return State;
	}

	State.Amplitude = Params->bShakeLocation ? Params->Amplitude : 0.0f;
	State.Frequency = Params->Frequency;
	State.RotationAmplitude = Params->bShakeRotation ? Params->RotationAmplitude : 0.0f;

	switch (Params->Pattern)
	{
		case EInteractableShakePattern::Mechanical:
			State.FreqY = 2.0f; 
			State.FreqZ = 0.5f;
			State.PhaseY = 0.0f; 
			State.PhaseZ = 0.0f;
			break;
		case EInteractableShakePattern::Sway:
			State.FreqY = 0.43f; 
			State.FreqZ = 0.21f;
			State.PhaseY = 1.0f; 
			State.PhaseZ = 0.5f;
			break;
		case EInteractableShakePattern::Jitter:
			State.FreqY = 2.71f; 
			State.FreqZ = 3.14f;
			State.PhaseY = 0.5f; 
			State.PhaseZ = 1.2f;
			break;
		case EInteractableShakePattern::VerticalOnly:
			State.AxisMult = FVector(0, 0, 1);
			break;
		case EInteractableShakePattern::HorizontalOnly:
			State.AxisMult = FVector(1, 1, 0);
			break;
		case EInteractableShakePattern::Pendulum:
			State.FreqY = 1.0f; 
			State.FreqZ = 2.0f;
			State.PhaseY = 0.0f; 
			State.PhaseZ = 1.57f;
			State.AxisMult = FVector(0, 1, 0.3f);
			break;
		case EInteractableShakePattern::Wild:
			State.FreqY = 0.81f;
			State.FreqZ = 1.23f;
			State.PhaseY = 1.57f;
			State.PhaseZ = 0.78f;
			break;
		default:
			break;
	}

	return State;
}

void UInteractableShakeComponent::BlendCurrentState(const FShakeTargetState& TargetState, const float DeltaTime)
{
	// Interpolate core parameters
	BlendedAmplitude = FMath::FInterpTo(BlendedAmplitude, TargetState.Amplitude, DeltaTime, BlendSpeed);
	BlendedFrequency = FMath::FInterpTo(BlendedFrequency, TargetState.Frequency, DeltaTime, BlendSpeed);
	BlendedRotationAmplitude = FMath::FInterpTo(BlendedRotationAmplitude, TargetState.RotationAmplitude, DeltaTime, BlendSpeed);
	
	// Only blend pattern internals if shaking (either currently shaking or still blending down/up)
	const bool bIsShaking = TargetState.Amplitude > 0.0f || TargetState.RotationAmplitude > 0.0f || BlendedAmplitude > 0.01f || BlendedRotationAmplitude > 0.01f;
	if (bIsShaking)
	{
		BlendedFreqY = FMath::FInterpTo(BlendedFreqY, TargetState.FreqY, DeltaTime, BlendSpeed);
		BlendedFreqZ = FMath::FInterpTo(BlendedFreqZ, TargetState.FreqZ, DeltaTime, BlendSpeed);
		BlendedPhaseY = FMath::FInterpTo(BlendedPhaseY, TargetState.PhaseY, DeltaTime, BlendSpeed);
		BlendedPhaseZ = FMath::FInterpTo(BlendedPhaseZ, TargetState.PhaseZ, DeltaTime, BlendSpeed);
		BlendedAxisMult = FMath::VInterpTo(BlendedAxisMult, TargetState.AxisMult, DeltaTime, BlendSpeed);
	}
}

void UInteractableShakeComponent::UpdateShakeEffect(const float DeltaTime, const FInteractableShakeParams* ActiveParams)
{
	if (BlendedAmplitude > 0.001f || BlendedRotationAmplitude > 0.001f)
	{
		ApplyShake(DeltaTime, ActiveParams);
	}
	else if (TrackedComponents.Num() > 0)
	{
		ResetAllComponentTransforms();
	}
}

void UInteractableShakeComponent::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateTimers(DeltaTime);

	// Determine target parameters for the current active state
	const FInteractableShakeParams* ActiveParams = GetCurrentActiveParams();
	const FShakeTargetState TargetState = CalculateTargetState(ActiveParams);

	// Interpolate current values towards targets
	BlendCurrentState(TargetState, DeltaTime);

	// Apply shake or reset if inactive
	UpdateShakeEffect(DeltaTime, ActiveParams);
}

const FInteractableShakeParams* UInteractableShakeComponent::GetCurrentActiveParams() const
{
	// Priority: Interacting > PreInteraction > After > InRange > Always
	if (bIsInteracting && InteractingShake.bEnabled)
		return &InteractingShake;

	if (bIsPreInteracting && PreInteractionShake.Settings.bEnabled)
		return &PreInteractionShake.Settings;

	if (AfterShakeTimer > 0.0f && AfterShake.Settings.bEnabled)
		return &AfterShake.Settings;

	if (bIsInRange && InRangeShake.bEnabled)
		return &InRangeShake;

	if (AlwaysShake.bEnabled)
		return &AlwaysShake;

	return nullptr;
}

void UInteractableShakeComponent::ApplyShake(const float DeltaTime, const FInteractableShakeParams* Params)
{
	const TArray<TObjectPtr<USceneComponent>>* Targets = GetCachedTargets(Params);
	if (!Targets)
	{
		return;
	}

	// Calculate new shake values for this frame using blended state
	TimeAccumulator += DeltaTime * BlendedFrequency;
	
	FVector NewOffset = FVector::ZeroVector;
	NewOffset.X = FMath::Sin(TimeAccumulator) * BlendedAmplitude * BlendedAxisMult.X;
	NewOffset.Y = FMath::Sin(TimeAccumulator * BlendedFreqY + BlendedPhaseY) * BlendedAmplitude * BlendedAxisMult.Y;
	NewOffset.Z = FMath::Sin(TimeAccumulator * BlendedFreqZ + BlendedPhaseZ) * BlendedAmplitude * BlendedAxisMult.Z;

	FRotator NewRot = FRotator::ZeroRotator;
	NewRot.Pitch = FMath::Sin(TimeAccumulator * 0.91f) * BlendedRotationAmplitude * BlendedAxisMult.Y;
	NewRot.Yaw = FMath::Sin(TimeAccumulator * 1.13f + 2.1f) * BlendedRotationAmplitude * BlendedAxisMult.Z;
	NewRot.Roll = FMath::Sin(TimeAccumulator * 1.37f + 1.2f) * BlendedRotationAmplitude * BlendedAxisMult.X;

	FQuat NewRotQuat = NewRot.Quaternion();

	// Identify components that are no longer being shaken in this frame and reset them
	for (auto It = TrackedComponents.CreateIterator(); It; ++It)
	{
		TObjectPtr<USceneComponent> Comp = It.Key();
		if (!Targets->Contains(Comp))
		{
			if (IsValid(Comp))
			{
				Comp->AddLocalOffset(-It.Value().LastOffset);
				Comp->AddLocalRotation(It.Value().LastRotation.Inverse());
			}
			It.RemoveCurrent();
		}
	}

	// Apply shake to all current targets
	for (const TObjectPtr<USceneComponent>& Target : *Targets)
	{
		if (!IsValid(Target)) 
			continue;

		auto& [LastOffset, LastRotation] = TrackedComponents.FindOrAdd(Target);

		// Combine undo old and apply new into a single delta transform step
		FVector DeltaOffset = NewOffset - LastOffset;
		FQuat DeltaRotation = LastRotation.Inverse() * NewRotQuat;

		Target->AddLocalOffset(DeltaOffset);
		Target->AddLocalRotation(DeltaRotation);

		// Cache current values
		LastOffset = NewOffset;
		LastRotation = NewRotQuat;
	}
}

void UInteractableShakeComponent::ResetAllComponentTransforms()
{
	for (auto& Elem : TrackedComponents)
	{
		TObjectPtr<USceneComponent> Comp = Elem.Key;
		if (IsValid(Comp))
		{
			Comp->AddLocalOffset(-Elem.Value.LastOffset);
			Comp->AddLocalRotation(Elem.Value.LastRotation.Inverse());
		}
	}
	
	TrackedComponents.Empty();
	TimeAccumulator = 0.0f;
}
#pragma endregion

#pragma region UFUNCTIONS
void UInteractableShakeComponent::SetInRange(const bool bInRange)
{
	bIsInRange = bInRange;
}

void UInteractableShakeComponent::SetPreInteracting(const bool bPreInteracting)
{
	bIsPreInteracting = bPreInteracting;
	if (bIsPreInteracting)
		AfterShakeTimer = 0.0f;
}

void UInteractableShakeComponent::SetInteracting(const bool bInteracting)
{
	bIsInteracting = bInteracting;
	if (bIsInteracting)
	{
		bIsPreInteracting = false; 
		AfterShakeTimer = 0.0f;
	}
	else
	{
		bIsPreInteracting = false;
	}
}

void UInteractableShakeComponent::PlayAfterShake()
{
	AfterShakeTimer = AfterShake.Duration;
}

void UInteractableShakeComponent::StopAllShakes()
{
	bIsInRange = false;
	bIsPreInteracting = false;
	bIsInteracting = false;
	AfterShakeTimer = 0.0f;
	ResetAllComponentTransforms();
}

void UInteractableShakeComponent::StopShakingComponent(USceneComponent* Component)
{
	if (!Component)
		return;

	// Undo any offset that was applied to this component
	if (FComponentShakeState* State = TrackedComponents.Find(Component))
	{
		if (IsValid(Component))
		{
			Component->AddLocalOffset(-State->LastOffset);
			Component->AddLocalRotation(State->LastRotation.Inverse());
		}
		TrackedComponents.Remove(Component);
	}

	// Blacklist it so it is never targeted again by any shake phase
	BlacklistedComponents.Add(Component);

	// Remove from caches
	AlwaysShakeTargets.Remove(Component);
	InRangeShakeTargets.Remove(Component);
	PreInteractionShakeTargets.Remove(Component);
	InteractingShakeTargets.Remove(Component);
	AfterShakeTargets.Remove(Component);
	DefaultShakeTargets.Remove(Component);
}

FVector UInteractableShakeComponent::GetShakeOffsetForComponent(USceneComponent* Component) const
{
	if (const FComponentShakeState* State = TrackedComponents.Find(Component))
	{
		return State->LastOffset;
	}
	return FVector::ZeroVector;
}

FRotator UInteractableShakeComponent::GetShakeRotationForComponent(USceneComponent* Component) const
{
	if (const FComponentShakeState* State = TrackedComponents.Find(Component))
	{
		return State->LastRotation.Rotator();
	}
	return FRotator::ZeroRotator;
}
#pragma endregion
