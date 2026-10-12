#pragma once

#include "Nodes/FlowNode.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "FlowNode_OnGameplayEvent.generated.h"

class UAbilitySystemComponent;

/**
 * Generic FlowNode that waits for a specific Gameplay Event on a target actor.
 * Useful for tutorials (e.g., crystal broken, trap closed, lever pulled).
 */
UCLASS(NotBlueprintable, meta = (DisplayName = "On Gameplay Event"))
class STILLHEAR_API UFlowNode_OnGameplayEvent : public UFlowNode
{
	GENERATED_BODY()

public:
	UFlowNode_OnGameplayEvent();

protected:
	/** The Gameplay Event Tag to listen for */
	UPROPERTY(EditAnywhere, Category = "Event")
	FGameplayTag EventTag;

	/** Optional: The Identity Tag of the actor to monitor for the event. If empty, monitors the Player. */
	UPROPERTY(EditAnywhere, Category = "Event")
	FGameplayTag TargetIdentityTag;

	/** Optional: If set, the node only triggers if the event's OptionalObject (the source) has this Identity Tag. */
	UPROPERTY(EditAnywhere, Category = "Event")
	FGameplayTag RequiredPayloadSourceTag;

	/** Handle for the GAS event listener */
	FDelegateHandle EventDelegateHandle;

	/** Weak pointer to the monitored ASC */
	TWeakObjectPtr<UAbilitySystemComponent> WeakASC;

public:
	virtual void ExecuteInput(const FName& PinName) override;
	virtual void OnLoad_Implementation() override;
	virtual void Cleanup() override;

protected:
	void StartMonitoring();
	void StopMonitoring();

	/** Callback for when the event is broadcast on the ASC */
	void OnGameplayEventReceived(const FGameplayEventData* Payload);
};
