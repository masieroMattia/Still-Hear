#include "Flow/Nodes/FlowNode_ClearUILayer.h"
#include "UI/Subsystem/UISubsystem.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Engine/LocalPlayer.h"

UFlowNode_ClearUILayer::UFlowNode_ClearUILayer()
{
#if WITH_EDITOR
	Category = TEXT("UI");
	NodeDisplayStyle = FlowNodeStyle::Logic;
#endif

	InputPins.Empty();
	InputPins.Add(FFlowPin(FName("In")));

	OutputPins.Empty();
	OutputPins.Add(FFlowPin(FName("Out")));
}

void UFlowNode_ClearUILayer::ExecuteInput(const FName& PinName)
{
	if (PinName == FName("In"))
	{
		if (UWorld* World = GetWorld())
		{
			if (APlayerController* PC = World->GetFirstPlayerController())
			{
				if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
				{
					if (UUISubsystem* UISubsystem = LocalPlayer->GetSubsystem<UUISubsystem>())
					{
						if (LayerTag.IsValid())
						{
							UISubsystem->ClearLayer(LayerTag);
						}
						else
						{
							UE_LOG(LogTemp, Warning, TEXT("ClearUILayer: LayerTag is invalid!"));
						}
					}
				}
			}
		}

		TriggerOutput(FName("Out"), true);
	}
}

#if WITH_EDITOR
FString UFlowNode_ClearUILayer::GetNodeDescription() const
{
	return LayerTag.IsValid() ? LayerTag.ToString() : Super::GetNodeDescription();
}
#endif

