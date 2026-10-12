#include "Audio/FlowNode_PlayMusic.h"
#include "Audio/GameAudioSubsystem.h"
#include "FlowTags.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

UFlowNode_PlayMusic::UFlowNode_PlayMusic()
{
#if WITH_EDITOR
	Category = TEXT("Audio");
	NodeDisplayStyle = FlowNodeStyle::Default;
#endif
	OutputPins.Empty();
	AddOutputPins({ TEXT("Out"), TEXT("Error") });
}

void UFlowNode_PlayMusic::ExecuteInput(const FName& PinName)
{
	Super::ExecuteInput(PinName);
	
	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UGameAudioSubsystem* AudioSubsystem = GI ? GI->GetSubsystem<UGameAudioSubsystem>() : nullptr;

	if (!AudioSubsystem || !Music)
	{
		TriggerOutput(TEXT("Error"), true);
		Finish();
		return;
	}
	
	//AudioSubsystem->PlayMusic(Music);
	AudioSubsystem->CrossfadeMusic(Music, FadeOutTime, FadeInTime);

	TriggerOutput(TEXT("Out"), true);
	Finish();
}