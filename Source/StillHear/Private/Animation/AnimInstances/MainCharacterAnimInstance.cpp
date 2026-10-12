#include "Animation/AnimInstances/MainCharacterAnimInstance.h"

#include "KismetAnimationLibrary.h"
#include "Character/StillHearMainCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayAbilitySystem/Tags/GameplayTags.h"

void UMainCharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Character = Cast<AStillHearMainCharacter>(GetOwningActor());

	if (Character)
		MovementComponent = Character->GetCharacterMovement();
}

void UMainCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!Character || !MovementComponent)
		return;
	
	Velocity = MovementComponent->Velocity;
	const FVector SpeedVector = FVector(Velocity.X, Velocity.Y, 0.0f);
	GroundSpeed = SpeedVector.Length();
	Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, Character->GetActorRotation());
	
	const FVector Acceleration = MovementComponent->GetCurrentAcceleration();
	bShouldMove = GroundSpeed > 3;

	bIsFalling = MovementComponent->IsFalling();
	bIsCrouching = MovementComponent->IsCrouching();

	bIsSprinting = Character->GetAbilitySystemComponent()->HasMatchingGameplayTag(TAG_GameplayAbility_MainCharacter_Sprint_Active);
	bIsDragging = Character->GetAbilitySystemComponent()->HasMatchingGameplayTag(TAG_Status_FreeDragging) || Character->GetAbilitySystemComponent()->HasMatchingGameplayTag(TAG_Status_RailDragging);
	
	if (bIsDragging)
		IKTraceAlpha = FMath::FInterpTo(IKTraceAlpha, 1.0f, DeltaSeconds, IKBlendInSpeed);
	else
		IKTraceAlpha = FMath::FInterpTo(IKTraceAlpha, bIsClimbing ? 0.0f : 1.0f, DeltaSeconds, IKBlendOutSpeed);
}

void UMainCharacterAnimInstance::UpdateIKTargets(const FTransform& LeftTarget, const FTransform& RightTarget)
{
	LeftEffectorTransform = LeftTarget;
	RightEffectorTransform = RightTarget;
}
