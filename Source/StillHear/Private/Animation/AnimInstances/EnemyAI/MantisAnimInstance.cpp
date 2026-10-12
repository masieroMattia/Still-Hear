#include "Animation/AnimInstances/EnemyAI/MantisAnimInstance.h"

#include "KismetAnimationLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnemiesAI/Pawns/Mantis/AIMantisCharacter.h"
#include "GameplayAbilitySystem/Tags/GameplayTags.h"

void UMantisAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	MantisRef = Cast<AAIMantisCharacter>(GetOwningActor());
	
	if (!MantisRef)
		return;
	
	MantisASC = MantisRef->GetAbilitySystemComponent();
	MantisCMC = MantisRef->GetCharacterMovement();
}

void UMantisAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	
	if (!MantisRef) 
		return;

	// Calculate Ground Speed and Direction
	Velocity = MantisRef->GetVelocity();
	GroundSpeed = Velocity.Size2D();
	Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, MantisRef->GetActorRotation());
	
	bIsMoving = GroundSpeed > 3.0f;

	if (MantisASC)
		bIsStunned = MantisASC->HasMatchingGameplayTag(TAG_Status_EnemyAI_Stunned);
	
	if (MantisCMC)
		bIsFalling = MantisCMC->IsFalling();
}
