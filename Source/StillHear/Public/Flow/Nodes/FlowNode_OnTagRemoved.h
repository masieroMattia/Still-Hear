#pragma once

#include "Nodes/FlowNode.h"
#include "GameplayTagContainer.h"
#include "FlowNode_OnTagRemoved.generated.h"

class UAbilitySystemComponent;

/**
 * FlowNode that waits for a specific Gameplay Tag to be removed from a target actor.
 */
UCLASS(NotBlueprintable, meta = (DisplayName = "On Tag Removed"))
class STILLHEAR_API UFlowNode_OnTagRemoved : public UFlowNode
{
	GENERATED_BODY()

public:
	UFlowNode_OnTagRemoved();

protected:
	/** The Gameplay Tag to watch for removal */
	UPROPERTY(EditAnywhere, Category = "Tag")
	FGameplayTag TagToWatch;

	/** Optional: The Identity Tag of the actor to monitor. If empty, monitors the Player. */
	UPROPERTY(EditAnywhere, Category = "Tag")
	FGameplayTag TargetIdentityTag;

	/** Handle for the GAS tag listener */
	FDelegateHandle TagDelegateHandle;

	/** Weak pointer to the target's ASC */
	TWeakObjectPtr<UAbilitySystemComponent> WeakASC;

public:
	virtual void ExecuteInput(const FName& PinName) override;
	virtual void OnLoad_Implementation() override;
	virtual void Cleanup() override;

protected:
	void StartMonitoring();
	void StopMonitoring();

	/** Triggered when the tracked tag changes */
	void OnGameplayTagChanged(const FGameplayTag Tag, int32 NewCount);
};
