#pragma once

#include "CoreMinimal.h"
#include "Nodes/FlowNode.h"
#include "FlowNode_StopAudio.generated.h"

UENUM(BlueprintType)
enum class EStopAudioType : uint8
{
	Music,
	Ambience,
	Both
};

/**
 * FlowNode to stop music and/or ambience with a fade out.
 */
UCLASS(meta = (DisplayName = "Stop Audio"))
class STILLHEAR_API UFlowNode_StopAudio : public UFlowNode
{
	GENERATED_BODY()

public:
	UFlowNode_StopAudio();

	/** Which audio type to stop */
	UPROPERTY(EditAnywhere, Category = "Flow|Audio")
	EStopAudioType StopType = EStopAudioType::Both;

	/** Time in seconds for the fade out */
	UPROPERTY(EditAnywhere, Category = "Flow|Audio")
	float FadeOutTime = 1.0f;

protected:
	virtual void ExecuteInput(const FName& PinName) override;
};
