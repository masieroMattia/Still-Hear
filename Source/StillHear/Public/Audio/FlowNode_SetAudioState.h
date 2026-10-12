#pragma once

#include "CoreMinimal.h"
#include "Nodes/FlowNode.h"
#include "GameplayTagContainer.h"
#include "FlowNode_SetAudioState.generated.h"

/**
 * Operation to perform on the audio state stack.
 */
UENUM(BlueprintType)
enum class EAudioStateOperation : uint8
{
	Push,
	Pop
};

/**
 * FlowNode to transition the game to a specific Audio State using the stack.
 */
UCLASS(meta = (DisplayName = "Audio State"))
class STILLHEAR_API UFlowNode_SetAudioState : public UFlowNode
{
	GENERATED_BODY()

public:
	UFlowNode_SetAudioState();

	/** The audio state to transition to (defined in AudioStateConfig) */
	UPROPERTY(EditAnywhere, Category = "Flow|Audio")
	FGameplayTag AudioState;

	/** Whether to add (Push) or remove (Pop) the state from the stack */
	UPROPERTY(EditAnywhere, Category = "Flow|Audio")
	EAudioStateOperation Operation = EAudioStateOperation::Push;

protected:
	virtual void ExecuteInput(const FName& PinName) override;
};
