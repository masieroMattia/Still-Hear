#include "Audio/FlowNode_SetAudioState.h"
#include "Audio/GameAudioSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

UFlowNode_SetAudioState::UFlowNode_SetAudioState()
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

void UFlowNode_SetAudioState::ExecuteInput(const FName& PinName)
{
	Super::ExecuteInput(PinName);

	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UGameAudioSubsystem* AudioSubsystem = GI ? GI->GetSubsystem<UGameAudioSubsystem>() : nullptr;


	if (AudioSubsystem)
	{
		if (Operation == EAudioStateOperation::Push)
		{
			AudioSubsystem->PushAudioState(AudioState);
		}
		else
		{
			AudioSubsystem->PopAudioState(AudioState);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("FlowNode_SetAudioState: UGameAudioSubsystem is null!"));
	}

	TriggerOutput(TEXT("Out"), true);
	Finish();
}
