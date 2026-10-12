#pragma once

#include "Nodes/FlowNode.h"
#include "Character/MainCharacterAbilityType.h"
#include "GameplayTagContainer.h"
#include "FlowNode_RemoveAbility.generated.h"

/**
 * FlowNode that removes a specific ability from a target actor (usually the main character).
 */
UCLASS(NotBlueprintable, meta = (DisplayName = "Remove Ability"))
class STILLHEAR_API UFlowNode_RemoveAbility : public UFlowNode
{
	GENERATED_BODY()

public:
	UFlowNode_RemoveAbility();

protected:
	/** The ability to remove */
	UPROPERTY(EditAnywhere, Category = "Ability")
	EMainCharacterAbilityType AbilityToRemove;

	/** Optional: The Identity Tag of the actor to remove the ability from. If empty, target the Player. */
	UPROPERTY(EditAnywhere, Category = "Ability")
	FGameplayTag TargetIdentityTag;

	/** If true, the ability will also be removed from the permanent save data for this slot */
	UPROPERTY(EditAnywhere, Category = "Ability")
	bool bPermanentRemove = true;


public:
	virtual void ExecuteInput(const FName& PinName) override;
};
