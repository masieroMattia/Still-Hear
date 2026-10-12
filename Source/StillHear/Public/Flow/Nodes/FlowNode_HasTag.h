#pragma once

#include "Nodes/FlowNode.h"
#include "GameplayTagContainer.h"
#include "FlowNode_HasTag.generated.h"

/**
 * Instant node that checks if an actor has a specific Gameplay Tag.
 * Branches the flow into True or False.
 */
UCLASS(NotBlueprintable, meta = (DisplayName = "Has Tag"))
class STILLHEAR_API UFlowNode_HasTag : public UFlowNode
{
	GENERATED_BODY()

public:
	UFlowNode_HasTag();

protected:
	/** The Gameplay Tag to check for */
	UPROPERTY(EditAnywhere, Category = "Tag")
	FGameplayTag TagToCheck;

	/** Optional: The Identity Tag of the actor to check. If empty, checks the Player. */
	UPROPERTY(EditAnywhere, Category = "Tag")
	FGameplayTag TargetIdentityTag;

public:
	virtual void ExecuteInput(const FName& PinName) override;
};
