#include "Audio/FlowNode_PlayAmbience.h"
#include "Audio/GameAudioSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

UFlowNode_PlayAmbience::UFlowNode_PlayAmbience()
{
#if WITH_EDITOR
	Category = TEXT("Audio");
	NodeDisplayStyle = FlowNodeStyle::Default;
#endif

	// Define input and output pins
	InputPins.Empty();
	AddInputPins({ TEXT("In") });
	
	OutputPins.Empty();
	AddOutputPins({ TEXT("Out"), TEXT("Error") });
}

void UFlowNode_PlayAmbience::ExecuteInput(const FName& PinName)
{
	Super::ExecuteInput(PinName);
	
	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UGameAudioSubsystem* AudioSubsystem = GI ? GI->GetSubsystem<UGameAudioSubsystem>() : nullptr;

	// Safety check
	if (!AudioSubsystem || !Ambience)
	{
		TriggerOutput(TEXT("Error"), true);
		Finish();
		return;
	}
	
	// Execute crossfade through the subsystem
	AudioSubsystem->CrossfadeAmbience(Ambience, FadeOutTime, FadeInTime);

	TriggerOutput(TEXT("Out"), true);
	Finish();
}
