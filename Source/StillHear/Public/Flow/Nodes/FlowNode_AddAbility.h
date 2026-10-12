#pragma once

#include "Nodes/FlowNode.h"
#include "Character/MainCharacterAbilityType.h"
#include "GameplayTagContainer.h"
#include "FlowNode_AddAbility.generated.h"

/**
 * FlowNode that grants a specific ability to a target actor (usually the main character).
 */
UCLASS(NotBlueprintable, meta = (DisplayName = "Add Ability"))
class STILLHEAR_API UFlowNode_AddAbility : public UFlowNode
{
	GENERATED_BODY()

public:
	UFlowNode_AddAbility();

protected:
	/** The ability to grant */
	UPROPERTY(EditAnywhere, Category = "Ability")
	EMainCharacterAbilityType AbilityToGrant;

	/** Optional: The Identity Tag of the actor to monitor. If empty, monitors the Player. */
	UPROPERTY(EditAnywhere, Category = "Ability")
	FGameplayTag TargetIdentityTag;

	/** If true, the ability will be saved in the permanent save data for this slot */
	UPROPERTY(EditAnywhere, Category = "Ability")
	bool bPermanentUnlock = true;

public:
	virtual void ExecuteInput(const FName& PinName) override;
};
