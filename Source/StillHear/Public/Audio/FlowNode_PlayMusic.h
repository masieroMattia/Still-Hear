
#pragma once

#include "CoreMinimal.h"
#include "Nodes/FlowNode.h"
#include "UObject/Object.h"
#include "FlowNode_PlayMusic.generated.h"

UCLASS(meta = (DisplayName = "Play Music"))
class STILLHEAR_API UFlowNode_PlayMusic : public UFlowNode
{
	GENERATED_BODY()
	
public:
	UFlowNode_PlayMusic();
	
	// Music to play
	UPROPERTY(EditAnywhere, Category = "Flow|Audio")
	USoundBase* Music;

	// If true, the music will restart even if it's already playing
	UPROPERTY(EditAnywhere, Category = "Flow|Audio")
	bool bForceRestart = true;

	/** Time in seconds to fade out the previous track */
	UPROPERTY(EditAnywhere, Category = "Flow|Audio")
	float FadeOutTime = 1.0f;

	/** Time in seconds to fade in the new track */
	UPROPERTY(EditAnywhere, Category = "Flow|Audio")
	float FadeInTime = 1.0f;

protected:
	virtual void ExecuteInput(const FName& PinName) override;
};
