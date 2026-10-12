#include "GameplayAbilitySystem/Attributes/BasicAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UBasicAttributeSet::UBasicAttributeSet()
{
	InitBaseSpeed(600.0f);
	InitSpeedMultiplier(1.0f);
}

void UBasicAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetBaseSpeedAttribute())
		UpdateCharacterSpeed(NewValue, GetSpeedMultiplier());
	
	else if (Attribute == GetSpeedMultiplierAttribute())
		UpdateCharacterSpeed(GetBaseSpeed(), NewValue);
	
	// THIS IS WHERE WE CLAMP ATTRIBUTES IF NEEDED, IF WE HAVE AN ATTRIBUTE THAT WHEN MODIFIED WE
	// HAVE TO MAKE SURE IT STAYS IN A CERTAIN RANGE, WE DO IT HERE
}

void UBasicAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetBaseSpeedAttribute())
		SetBaseSpeed(GetBaseSpeed());
	
	else if (Data.EvaluatedData.Attribute == GetSpeedMultiplierAttribute())
		SetSpeedMultiplier(GetSpeedMultiplier());
	
} 

void UBasicAttributeSet::UpdateCharacterSpeed(const float Base, const float Multiplier) const
{
	const ACharacter* Character = Cast<ACharacter>(GetOwningActor());
	
	if (!Character)
		return;

	const float FinalSpeed = Base * Multiplier;
		
	Character->GetCharacterMovement()->MaxWalkSpeed = FinalSpeed;
	Character->GetCharacterMovement()->MaxFlySpeed = FinalSpeed;
}


