#include "Audio/FlowNode_StopAudio.h"
#include "Audio/GameAudioSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

UFlowNode_StopAudio::UFlowNode_StopAudio()
{
#if WITH_EDITOR
	Category = TEXT("Audio");
	NodeDisplayStyle = FlowNodeStyle::Default;
#endif

	InputPins.Empty();
	AddInputPins({ TEXT("In") });

	OutputPins.Empty();
	AddOutputPins({ TEXT("Out") });
}

void UFlowNode_StopAudio::ExecuteInput(const FName& PinName)
{
	Super::ExecuteInput(PinName);

	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UGameAudioSubsystem* AudioSubsystem = GI ? GI->GetSubsystem<UGameAudioSubsystem>() : nullptr;

	if (AudioSubsystem)
	{
		switch (StopType)
		{
		case EStopAudioType::Music:
			AudioSubsystem->StopMusic(FadeOutTime);
			break;
		case EStopAudioType::Ambience:
			AudioSubsystem->StopAmbience(FadeOutTime);
			break;
		case EStopAudioType::Both:
			AudioSubsystem->StopMusic(FadeOutTime);
			AudioSubsystem->StopAmbience(FadeOutTime);
			break;
		}
	}

	TriggerOutput(TEXT("Out"), true);
	Finish();
}
