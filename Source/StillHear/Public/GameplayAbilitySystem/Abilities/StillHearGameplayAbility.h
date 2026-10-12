#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "StillHearGameplayAbility.generated.h"

UCLASS()
class STILLHEAR_API UStillHearGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

#pragma region CONSTRUCTOR
public:
	UStillHearGameplayAbility();
#pragma endregion

#pragma region METHODS
public:
	/*
		 * In Unreal Engine 5.5, a bug was introduced where if you put a tag into the Block Abilities with Tag
		 * category, and this tag is actually a Parent tag, then the tag is not recognized as blocked.
		 * For example, if I have an ability that has asset tag GameplayAbility.AbilityOne and then I have an ability
		 * that has the tag GameplayAbility into the BlockAbilitiesWithTag container, then it wouldn't be blocked correctly
		 * (while this happened in previous versions of Unreal Engine. For this reason I'm overriding this function directly
		 * and fixing the check causing the problem
		 */
	virtual bool DoesAbilitySatisfyTagRequirements(const UAbilitySystemComponent& AbilitySystemComponent, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
#pragma endregion 
};
