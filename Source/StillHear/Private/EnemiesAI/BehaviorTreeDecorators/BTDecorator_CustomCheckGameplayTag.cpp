#include "EnemiesAI/BehaviorTreeDecorators/BTDecorator_CustomCheckGameplayTag.h"

#include "AbilitySystemComponent.h"
#include "AIController.h"

#pragma region CONSTRUCTORS
UBTDecorator_CustomCheckGameplayTag::UBTDecorator_CustomCheckGameplayTag()
{
	NodeName = "Custom Check Gameplay Tag";
}
#pragma endregion
	
#pragma region METHODS
// Check if the controlled pawn has the specified gameplay tag
bool UBTDecorator_CustomCheckGameplayTag::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	APawn* ControlledPawn = OwnerComp.GetAIOwner() ? OwnerComp.GetAIOwner()->GetPawn() : nullptr;
	if (!ControlledPawn)
		return false;

	UAbilitySystemComponent* AbilitySystemComponent = Cast<UAbilitySystemComponent>(ControlledPawn->GetComponentByClass(UAbilitySystemComponent::StaticClass()));
	if (!AbilitySystemComponent)
		return false;
	
	const bool bHasTag = AbilitySystemComponent->HasMatchingGameplayTag(TagToCheck);
	
	return IsInversed() ? !bHasTag : bHasTag;
}
#pragma endregion