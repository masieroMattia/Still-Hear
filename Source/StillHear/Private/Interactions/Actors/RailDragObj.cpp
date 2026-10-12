#include "Interactions/Actors/RailDragObj.h"

#include "AbilitySystemGlobals.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "GameplayAbilitySystem/Tags/GameplayTags.h"
#include "GameFramework/CharacterMovementComponent.h"

#pragma region CONSTRUCTOR
ARailDragObj::ARailDragObj()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false; // Disable ticking by default, it will be enabled when the interaction starts and disabled when it ends
	
	Tags.Add(TAG_Climb.GetTag().GetTagName()); // Add the climb tag to this actor so it can be detected by the character's climbing ability
}
#pragma endregion

#pragma region METHODS
void ARailDragObj::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	if (!RailActor || !RailActor->GetSpline())
		return;

	const USplineComponent* Rail = RailActor->GetSpline();
	
	const float ClosestDistanceOnRail = Rail->GetDistanceAlongSplineAtLocation(GetActorLocation(), ESplineCoordinateSpace::World);
	
	const FVector RailLocation = Rail->GetLocationAtDistanceAlongSpline(ClosestDistanceOnRail, ESplineCoordinateSpace::World);
	const FRotator RailRotation = Rail->GetRotationAtDistanceAlongSpline(ClosestDistanceOnRail, ESplineCoordinateSpace::World);
	const FVector RailUpVector = Rail->GetUpVectorAtDistanceAlongSpline(ClosestDistanceOnRail, ESplineCoordinateSpace::World);
	
	const FRotator FinalRotation = (RailRotation.Quaternion() * RotationOffset.Quaternion()).Rotator();
	
	SetActorLocationAndRotation(RailLocation + (RailUpVector * HeightOffset), FinalRotation);
}

void ARailDragObj::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (!CurrentInteractor || !RailActor || !RailActor->GetSpline())
		return;

	const USplineComponent* Rail = RailActor->GetSpline();
	
	const FVector DesiredLocation = CurrentInteractor->GetActorLocation() + GrabOffset; // Calculate the desired location based on the interactor's current location and the initial grab offset
	const float TargetDistanceAlongRail = Rail->GetDistanceAlongSplineAtLocation(DesiredLocation, ESplineCoordinateSpace::World); // Find the distance along the rail that corresponds to the desired location
	
	// Smoothly interpolate the current distance along the rail towards the target distance
	CurrentDistanceAlongRail = FMath::FInterpTo(CurrentDistanceAlongRail, TargetDistanceAlongRail, DeltaSeconds, ObjectLerpSpeed);
	CurrentDistanceAlongRail = FMath::Clamp(CurrentDistanceAlongRail, 0.0f, Rail->GetSplineLength()); // Clamp the distance along the rail to ensure it stays within the bounds of the spline
	
	// Update the object's position and rotation based on the new distance along the rail, and apply the height offset in the direction of the rail's up vector
	const FVector RailUpVector = Rail->GetUpVectorAtDistanceAlongSpline(CurrentDistanceAlongRail, ESplineCoordinateSpace::World);
	const FVector NewObjectLocation = Rail->GetLocationAtDistanceAlongSpline(CurrentDistanceAlongRail, ESplineCoordinateSpace::World) + (RailUpVector * HeightOffset);
	const FRotator NewObjectRotation = Rail->GetRotationAtDistanceAlongSpline(CurrentDistanceAlongRail, ESplineCoordinateSpace::World);
	
	const FRotator FinalObjectRotation = (NewObjectRotation.Quaternion() * RotationOffset.Quaternion()).Rotator();
	
	SetActorLocationAndRotation(NewObjectLocation, FinalObjectRotation);
	
	FVector InteractorLookDirection = NewObjectLocation - CurrentInteractor->GetActorLocation();
	InteractorLookDirection.Z = 0.0f; // Flatten the look direction to the horizontal plane to prevent the interactor from tilting up or down while dragging
	
	if (!InteractorLookDirection.IsNearlyZero())
	{
		const FRotator TargetInteractorRotation = InteractorLookDirection.Rotation();
		const FRotator SmoothInteractorRotation = FMath::RInterpTo(CurrentInteractor->GetActorRotation(), TargetInteractorRotation, DeltaSeconds, CurrentInteractor->GetCharacterMovement()->RotationRate.Yaw);
	
		CurrentInteractor->SetActorRotation(SmoothInteractorRotation);
	}
	
	const float CurrentSquareDistance = FVector::DistSquared(CurrentInteractor->GetActorLocation(), GetActorLocation());
	if (CurrentSquareDistance > MaxAllowedSquareDistance)
		EndInteraction(CurrentInteractor); // End the interaction if the interactor moves too far from the object
}

void ARailDragObj::ApplySpeedMultiplierToOwner(const TObjectPtr<ACharacter> Interactor, const float Multiplier) const
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
#pragma endregion

#pragma region INTERFACE METHODS
void ARailDragObj::ExecuteStartInteraction(TObjectPtr<ACharacter> Interactor)
{
	Super::ExecuteStartInteraction(Interactor);
	
	if (!Interactor || !RailActor || !RailActor->GetSpline())
		return;
	
	CurrentInteractor = Interactor;
	StaticMeshComponent->SetSimulatePhysics(false);

	const USplineComponent* Rail = RailActor->GetSpline(); 

	CurrentDistanceAlongRail = Rail->GetDistanceAlongSplineAtLocation(GetActorLocation(), ESplineCoordinateSpace::World);
	
	GrabOffset = GetActorLocation() - CurrentInteractor->GetActorLocation(); // Calculate the initial offset from the object's location to the interactor's location at the moment of grabbing, which will be used to maintain a consistent grip on the object as it moves along the rail
	GrabOffset.Z = 0.0f;
	
	const float InitialDistance = FVector::Distance(CurrentInteractor->GetActorLocation(), GetActorLocation());
	MaxAllowedSquareDistance = FMath::Square(InitialDistance + BreakGripTolerance);
	
	ApplySpeedMultiplierToOwner(Interactor, DragMultiplier); // Apply the speed multiplier to the interactor
	SetActorTickEnabled(true); // Enable ticking to update the object's position along the rail
}

void ARailDragObj::EndInteraction(const TObjectPtr<ACharacter> Interactor)
{
	Super::EndInteraction();
	
	SetActorTickEnabled(false);
	CurrentInteractor = nullptr;
	
	if (!Interactor)
		return;
	
	UAbilitySystemComponent* InteractorASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Interactor);
	if (!InteractorASC)
		return;
	
	InteractorASC->RemoveActiveGameplayEffectBySourceEffect(DragEffectClass, InteractorASC);
}
#pragma endregion