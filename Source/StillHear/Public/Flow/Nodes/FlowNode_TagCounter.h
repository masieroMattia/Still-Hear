#pragma once

#include "Nodes/FlowNode.h"
#include "GameplayTagContainer.h"
#include "FlowNode_TagCounter.generated.h"

class UAbilitySystemComponent;

/**
 * FlowNode that counts occurrences of a Gameplay Tag on a target actor (Player by default).
 * Triggers an output when a threshold is reached.
 */
UCLASS(NotBlueprintable, meta = (DisplayName = "Tag Counter"))
class STILLHEAR_API UFlowNode_TagCounter : public UFlowNode
{
	GENERATED_BODY()

public:
	UFlowNode_TagCounter();

protected:
	/** The Gameplay Tag to track/count */
	UPROPERTY(EditAnywhere, Category = "Counter")
	FGameplayTag TagToCount;

	/** Optional: The Identity Tag of the actor to monitor. If empty, monitors the Player. */
	UPROPERTY(EditAnywhere, Category = "Counter")
	FGameplayTag TargetIdentityTag;

	/** How many times the tag must occur before triggering the output */
	UPROPERTY(EditAnywhere, Category = "Counter", meta = (ClampMin = "1"))
	int32 Threshold;

	/** Current count of tag occurrences (Saved) */
	UPROPERTY(SaveGame)
	int32 CurrentCount;

	/** Map of tracked ASCs and their respective delegate handles for the tag listener */
	TMap<TWeakObjectPtr<UAbilitySystemComponent>, FDelegateHandle> TrackedASCs;

public:
	virtual void ExecuteInput(const FName& PinName) override;
	virtual void OnLoad_Implementation() override;
	virtual void Cleanup() override;

protected:
	/** Starts monitoring the tag */
	void StartTracking();
	
	/** Stops monitoring the tag */
	void StopTracking();

	/** Triggered when the tracked tag is added to the ASC */
	void OnGameplayTagChanged(const FGameplayTag Tag, int32 NewCount);
};
