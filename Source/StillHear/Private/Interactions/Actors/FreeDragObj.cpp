#include "Interactions/Actors/FreeDragObj.h"

#include "AbilitySystemGlobals.h"
#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameplayAbilitySystem/Tags/GameplayTags.h"
#include "Interactions/Components/InteractableShakeComponent.h"

#pragma region CONSTRUCTOR
AFreeDragObj::AFreeDragObj()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false; // Disable ticking by default, it will be enabled when the interaction starts and disabled when it ends
	
	Tags.Add(TAG_Climb.GetTag().GetTagName()); // Add the climb tag to this actor so it can be detected by the character's climbing ability
	Tags.Add(TAG_Counterweight.GetTag().GetTagName());
}
#pragma endregion
	
#pragma region METHODS
void AFreeDragObj::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (bIsFalling)
	{
		HandleFalling();
		return;
	}
	
	if (!CurrentInteractor)
		return;
	
	CheckGripBreak();
	
	if (CurrentInteractor) // Still valid after grip check
		CheckInteractorAbove();
	
	if (CurrentInteractor) // Still valid after above check
	{
		UpdateFollowing(DeltaSeconds);
		UpdateBobbing(DeltaSeconds);
		UpdateUprighting(DeltaSeconds);
		UpdateSpinning(DeltaSeconds);
	}
}

// Monitor the object's velocity and distance from the ground to determine when it has landed
void AFreeDragObj::HandleFalling()
{
	const float Speed = StaticMeshComponent->GetComponentVelocity().Size();
	if (Speed > LandingVelocityThreshold)
		return; // Still moving too fast
	
	// Line trace downward from the bottom of the mesh to check ground proximity
	const FBoxSphereBounds Bounds = StaticMeshComponent->CalcBounds(StaticMeshComponent->GetComponentTransform());
	const FVector TraceStart = FVector(Bounds.Origin.X, Bounds.Origin.Y, Bounds.Origin.Z - Bounds.BoxExtent.Z);
	
	FHitResult Hit;
	const bool bHitGround = TraceGround(TraceStart, LandingDistanceThreshold + 1.0f, Hit);
	if (bHitGround && Hit.Distance <= LandingDistanceThreshold)
	{
		bIsFalling = false;
		StaticMeshComponent->SetSimulatePhysics(false);
		SetActorTickEnabled(false);
	}
}

// Check if the interactor has moved too far from the object, and if so, end the interaction
void AFreeDragObj::CheckGripBreak()
{
	const float CurrentDistance = FVector::Distance(CurrentInteractor->GetActorLocation(), GetActorLocation());
	if (CurrentDistance > (GripDistance + BreakGripTolerance))
		EndInteraction(CurrentInteractor);
}

// Check if the interactor is standing on top of the object, and if so, end the interaction
void AFreeDragObj::CheckInteractorAbove()
{
	const FBoxSphereBounds Bounds = StaticMeshComponent->CalcBounds(StaticMeshComponent->GetComponentTransform());
	const float ObjectTopZ = Bounds.Origin.Z + Bounds.BoxExtent.Z;
	const float InteractorZ = CurrentInteractor->GetActorLocation().Z;
	
	if (InteractorZ > ObjectTopZ)
		EndInteraction(CurrentInteractor);
}

// Update the object's XY position to follow the interactor in front of them
// maintaining a consistent horizontal distance and smoothly interpolating along the circular arc around the player
void AFreeDragObj::UpdateFollowing(const float DeltaSeconds)
{
	const FVector InteractorLocation = CurrentInteractor->GetActorLocation();
	FVector Forward = CurrentInteractor->GetActorForwardVector();
	Forward.Z = 0.0f;
	Forward.Normalize();
	
	// TargetAngle: the angle (in radians) on the XY plane where the player is looking.
	// This is the point on the orbit circle where we WANT the object to end up.
	const float TargetAngle = FMath::Atan2(Forward.Y, Forward.X);
	
	// CurrentAngle: the angle (in radians) on the XY plane where the object currently sits,
	// measured from the player's position. This is where the object IS right now on the orbit.
	FVector CurrentLocation = GetActorLocation();
	if (ShakeComponent)
	{
		CurrentLocation -= ShakeComponent->GetShakeOffsetForComponent(GetRootComponent());
	}

	FVector ToObject = CurrentLocation - InteractorLocation;
	ToObject.Z = 0.0f;
	const float CurrentAngle = FMath::Atan2(ToObject.Y, ToObject.X);
	
	// DeltaAngle: how far (in radians) the object needs to travel along the arc.
	// We normalize it to the range [-PI, PI] == [-180°, 180°] so it always takes the shortest path
	// around the circle. For example, if DeltaAngle = 350° it becomes -10° (go the other way).
	float DeltaAngle = TargetAngle - CurrentAngle;
	
	while (DeltaAngle > PI) 
		DeltaAngle -= 2.0f * PI;
	while (DeltaAngle < -PI) 
		DeltaAngle += 2.0f * PI;
	
	// NewAngle: slide the object along the arc by a fraction of DeltaAngle this frame.
	// FollowInterpSpeed * DeltaSeconds controls how fast; clamped to [0, 1] so we never overshoot.
	// At 0 the object doesn't move; at 1 it snaps instantly to the target.
	const float NewAngle = CurrentAngle + DeltaAngle * FMath::Clamp(FollowInterpSpeed * DeltaSeconds, 0.0f, 1.0f);
	
	// Convert the new angle back to a world-space XY position on the orbit circle.
	// Cos/Sin give us the unit direction, multiplied by DragDistance to place it on the circle,
	// then offset by the player's position to get the final world coordinate.
	const FVector TargetPos = InteractorLocation + FVector(FMath::Cos(NewAngle), FMath::Sin(NewAngle), 0.0f) * DragDistance;
	
	// Apply only XY — the Z is handled separately by UpdateBobbing.
	// bSweep = true so the object stops if it hits a wall instead of passing through.
	FVector LogicalLocation = GetActorLocation();
	if (ShakeComponent)
	{
		LogicalLocation -= ShakeComponent->GetShakeOffsetForComponent(GetRootComponent());
	}

	LogicalLocation.X = TargetPos.X;
	LogicalLocation.Y = TargetPos.Y;
	
	// Re-add shake before setting world location to maintain visual offset
	if (ShakeComponent)
	{
		LogicalLocation += ShakeComponent->GetShakeOffsetForComponent(GetRootComponent());
	}

	SetActorLocation(LogicalLocation, true);
}

void AFreeDragObj::UpdateBobbing(const float DeltaSeconds)
{
	// Update FloatBaseZ to follow terrain changes (stairs, slopes) while dragging
	FVector LogicalLocation = GetActorLocation();
	if (ShakeComponent)
	{
		LogicalLocation -= ShakeComponent->GetShakeOffsetForComponent(GetRootComponent());
	}
	
	FHitResult Hit;
	const bool bHitGround = TraceGround(FVector(LogicalLocation.X, LogicalLocation.Y, LogicalLocation.Z + 50.0f), GroundTraceDistance, Hit);
	
	float InteractorBottomZ = CurrentInteractor->GetActorLocation().Z;
	if (const UCapsuleComponent* InteractorCapsule = CurrentInteractor->GetCapsuleComponent())
		InteractorBottomZ -= InteractorCapsule->GetScaledCapsuleHalfHeight();
	
	float TargetBaseZ = InteractorBottomZ;
	
	if (bHitGround)
		TargetBaseZ = FMath::Max(Hit.ImpactPoint.Z, InteractorBottomZ);
	
	FloatBaseZ = FMath::FInterpTo(FloatBaseZ, TargetBaseZ, DeltaSeconds, FloatInterpSpeed);
	
	float BobOffset = 0.0f;
	if (bEnableBobbing)
	{
		BobTimer += DeltaSeconds;
		BobOffset = FMath::Sin(BobTimer * BobFrequency * 2.0f * PI) * BobAmplitude;
	}
	const float TargetZ = FloatBaseZ + FloatHeight + BobOffset;
	
	FVector FinalLocation = LogicalLocation;
	FinalLocation.Z = FMath::FInterpTo(FinalLocation.Z, TargetZ, DeltaSeconds, FloatInterpSpeed);

	if (ShakeComponent)
	{
		FinalLocation += ShakeComponent->GetShakeOffsetForComponent(GetRootComponent());
	}

	SetActorLocation(FinalLocation, true); // Sweep to respect collisions
}

// Gradually straighten the object (Roll and Pitch to 0) while preserving Yaw, rotating via the shortest path
void AFreeDragObj::UpdateUprighting(const float DeltaSeconds)
{
	const FQuat CurrentQuat = GetActorRotation().Quaternion();
	// When spinning, preserve the current Yaw so the spin isn't fought; otherwise restore the original Yaw
	const float TargetYaw = bEnableSpinning ? GetActorRotation().Yaw : UprightTargetYaw;
	const FQuat TargetQuat = FRotator(0.0f, TargetYaw, 0.0f).Quaternion();
	
	const FQuat NewQuat = FQuat::Slerp(CurrentQuat, TargetQuat, FMath::Clamp(UprightInterpSpeed * DeltaSeconds, 0.0f, 1.0f));
	SetActorRotation(NewQuat.Rotator());
}

// Continuously spin the object around the vertical axis (Yaw) while floating
void AFreeDragObj::UpdateSpinning(const float DeltaSeconds)
{
	if (!bEnableSpinning)
		return;
	
	const FQuat SpinDelta = FQuat(FVector::UpVector, FMath::DegreesToRadians(SpinSpeed * DeltaSeconds));
	const FQuat CurrentQuat = GetActorRotation().Quaternion();
	SetActorRotation((CurrentQuat * SpinDelta).Rotator());
}

void AFreeDragObj::ApplySpeedMultiplierToOwner(const TObjectPtr<ACharacter> Interactor, const float Multiplier) const
{
	UAbilitySystemComponent* InteractorASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Interactor);
	if (!InteractorASC)
		return;

	const FGameplayEffectSpecHandle SpecHandle = InteractorASC->MakeOutgoingSpec(
		DragEffectClass,
		1.0f,
		InteractorASC->MakeEffectContext()
	);

	if (!SpecHandle.IsValid())
		return;

	SpecHandle.Data->SetSetByCallerMagnitude(
		FGameplayTag::RequestGameplayTag(FName("Data.SpeedMultiplier")),
		Multiplier
	);

	// Apply the effect to change the values in the character class
	InteractorASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
}
// Line trace downward from Origin for TraceDistance units; returns true if ground was hit
bool AFreeDragObj::TraceGround(const FVector& Origin, const float TraceDistance, FHitResult& OutHit) const
{
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	if (CurrentInteractor)
		Params.AddIgnoredActor(CurrentInteractor);
	
	const FVector TraceEnd = Origin - FVector(0.0f, 0.0f, TraceDistance);
	return GetWorld()->LineTraceSingleByChannel(OutHit, Origin, TraceEnd, ECC_Visibility, Params);
}
#pragma endregion
	
#pragma region INTERFACE METHODS
void AFreeDragObj::ExecuteStartInteraction(TObjectPtr<ACharacter> Interactor)
{
	Super::ExecuteStartInteraction(Interactor);
	
	CurrentInteractor = Interactor;
	bIsFalling = false; // Reset in case the previous falling state was still active
	bIsDragging = true;
	
	const FVector ObjectLocation = GetActorLocation();
	const FVector InteractorLocation = Interactor->GetActorLocation();
	
	UprightTargetYaw = GetActorRotation().Yaw; // Preserve the current Yaw while straightening
	BobTimer = 0.0f;
	
	FHitResult Hit;
	FloatBaseZ = TraceGround(ObjectLocation, GroundTraceDistance, Hit) ? Hit.ImpactPoint.Z : ObjectLocation.Z; // The ground is our reference point for floating
	
	// Calculate horizontal drag distance, ensuring a minimum distance from the player
	FVector ToObject = ObjectLocation - InteractorLocation;
	ToObject.Z = 0.0f;
	DragDistance = FMath::Max(ToObject.Size(), MinDragDistance);
	
	GripDistance = FVector::Distance(InteractorLocation, ObjectLocation);
	
	// Disable physics during drag — we control position and rotation manually
	StaticMeshComponent->SetSimulatePhysics(false);
	StaticMeshComponent->SetEnableGravity(false);
	
	ApplySpeedMultiplierToOwner(Interactor, DragMultiplier); // Apply the speed multiplier to the interactor
	
	SetActorTickEnabled(true); // Enable ticking to check the distance between the interactor and the object
}

void AFreeDragObj::EndInteraction(const TObjectPtr<ACharacter> Interactor)
{
	Super::EndInteraction();
	
	if (!bIsDragging) 
		return; // Not currently dragging, ignore
	
	bIsDragging = false;
	CurrentInteractor = nullptr;
	
	// Enable physics with gravity for the falling phase
	StaticMeshComponent->SetSimulatePhysics(true);
	StaticMeshComponent->SetEnableGravity(true);
	
	bIsFalling = true; // Keep tick alive to monitor when the object lands
	
	if (!Interactor)
		return;
	
	UAbilitySystemComponent* InteractorASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Interactor);
	if (!InteractorASC)
		return;
	
	InteractorASC->RemoveActiveGameplayEffectBySourceEffect(DragEffectClass, InteractorASC);
}

void AFreeDragObj::Reset()
{
	Super::Reset();
}
#pragma endregion
