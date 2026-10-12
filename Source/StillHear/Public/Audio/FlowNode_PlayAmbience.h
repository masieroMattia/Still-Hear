#pragma once

#include "CoreMinimal.h"
#include "Nodes/FlowNode.h"
#include "UObject/Object.h"
#include "FlowNode_PlayAmbience.generated.h"

/**
 * FlowNode to play or crossfade ambient sounds via the GameAudioSubsystem.
 */
UCLASS(meta = (DisplayName = "Play Ambience"))
class STILLHEAR_API UFlowNode_PlayAmbience : public UFlowNode
{
	GENERATED_BODY()
	
public:
	UFlowNode_PlayAmbience();
	
	/** The ambient sound track to play */
	UPROPERTY(EditAnywhere, Category = "Flow|Audio")
	TObjectPtr<USoundBase> Ambience;

	/** Time in seconds to fade out the previous track */
	UPROPERTY(EditAnywhere, Category = "Flow|Audio")
	float FadeOutTime = 1.0f;

	/** Time in seconds to fade in the new track */
	UPROPERTY(EditAnywhere, Category = "Flow|Audio")
	float FadeInTime = 1.0f;

protected:
	virtual void ExecuteInput(const FName& PinName) override;
};
