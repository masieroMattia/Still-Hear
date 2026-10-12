#pragma once

#include "CoreMinimal.h"
#include "Nodes/FlowNode.h"
#include "GameplayTagContainer.h"
#include "FlowNode_IdleCheck.generated.h"

class UAbilitySystemComponent;
struct FGameplayEventData;

/**
 * FlowNode that triggers an output if the player remains idle for a set time.
 * It is cancelled ONLY by receiving a specific Gameplay Event (Event-Driven).
 */
UCLASS(NotBlueprintable, meta = (DisplayName = "Idle Check"))
class STILLHEAR_API UFlowNode_IdleCheck : public UFlowNode
{
	GENERATED_BODY()

public:
	UFlowNode_IdleCheck();

protected:
	/** How many seconds to wait before triggering TimeoutReached */
	UPROPERTY(EditAnywhere, Category = "Tutorial")
	float IdleTime;

	/** Optional: The Identity Tag of the actor to monitor. If empty, monitors the Player. */
	UPROPERTY(EditAnywhere, Category = "Tutorial")
	FGameplayTag TargetIdentityTag;

	/** The Gameplay Event Tag to listen for (e.g., Status.MainCharacter.Moving) */
	UPROPERTY(EditAnywhere, Category = "Tutorial")
	FGameplayTag EventTagToCancel;

	/** Main timer for the idle timeout */
	FTimerHandle TimerHandle_Idle;
	
	/** Handle for the GAS event listener */
	FDelegateHandle EventDelegateHandle;

	/** Handle for the GAS tag listener */
	FDelegateHandle TagDelegateHandle;
	
	/** Weak pointer to the player's ASC */
	TWeakObjectPtr<UAbilitySystemComponent> WeakASC;

	/** Internal guard to avoid multiple output triggers */
	bool bHasTriggered = false;

public:
	virtual void ExecuteInput(const FName& PinName) override;
	virtual void OnLoad_Implementation() override;
	virtual void Cleanup() override;

protected:
	/** Starts the idle countdown and subscribes to the GAS event and tags */
	void StartCheck();
	
	/** Stops the timer and clears the delegates */
	void StopCheck();
	
	/** Called when the timer expires */
	void OnIdleTimeout();
	
	/** Triggered when the cancellation event is received */
	void OnGameplayEventReceived(const FGameplayEventData* Payload);

	/** Triggered when the cancellation tag is added */
	void OnGameplayTagChanged(const FGameplayTag Tag, int32 NewCount);
};
