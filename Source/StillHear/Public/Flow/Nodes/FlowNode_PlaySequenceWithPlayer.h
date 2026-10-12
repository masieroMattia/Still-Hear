#pragma once

#include "CoreMinimal.h"
#include "Nodes/Actor/FlowNode_PlayLevelSequence.h"
#include "FlowNode_PlaySequenceWithPlayer.generated.h"

/**
 * FlowNode that plays a Level Sequence and automatically binds the Player Character at runtime.
 */
UCLASS(NotBlueprintable, meta = (DisplayName = "Play Sequence (With Player Bound)"))
class STILLHEAR_API UFlowNode_PlaySequenceWithPlayer : public UFlowNode_PlayLevelSequence
{
	GENERATED_BODY()

public:
	UFlowNode_PlaySequenceWithPlayer(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void ExecuteInput(const FName& PinName) override;
};
