#include "EnemiesAI/Animations/SoundEaterAnimInstanceBase.h"

#include "KismetAnimationLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnemiesAI/Pawns/Mantis/AIMantisCharacter.h"
#include "GameplayAbilitySystem/Tags/GameplayTags.h"

#pragma region METHODS
void USoundEaterAnimInstanceBase::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	Pawn = Cast<AAIMantisCharacter>(TryGetPawnOwner());
}

void USoundEaterAnimInstanceBase::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	
	if (!Pawn)
		return;
	
	CurrentVelocity = Pawn->GetVelocity(); // Get the current velocity of the pawn
	CurrentDirection = UKismetAnimationLibrary::CalculateDirection(CurrentVelocity, Pawn->GetActorRotation()); // Get the movement direction of the pawn
	Speed = Pawn->GetVelocity().Size(); // Get the speed of the pawn

	const UAbilitySystemComponent* AbilitySystemComponent = Pawn->GetAbilitySystemComponent();
	
	if (!AbilitySystemComponent)
		return;
	
	bIsStunned = AbilitySystemComponent->HasMatchingGameplayTag(TAG_Status_EnemyAI_Stunned);

	if (const UCharacterMovementComponent* CMC = Pawn->GetCharacterMovement())
		bIsFalling = CMC->IsFalling();
	
	bShouldMove = Speed > 3.0f;
}
#pragma endregion