#include "Flow/Nodes/FlowNode_HasTag.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "FlowSubsystem.h"

UFlowNode_HasTag::UFlowNode_HasTag()
{
#if WITH_EDITOR
	Category = TEXT("Logic");
	NodeDisplayStyle = FlowNodeStyle::Condition;
#endif

	InputPins.Empty();
	InputPins.Add(FFlowPin(FName("In")));

	OutputPins.Empty();
	OutputPins.Add(FFlowPin(FName("True")));
	OutputPins.Add(FFlowPin(FName("False")));
}

void UFlowNode_HasTag::ExecuteInput(const FName& PinName)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		TriggerOutput(FName("False"), true);
		return;
	}

	// 1. Find target actors
	TArray<AActor*> TargetActors;
	if (TargetIdentityTag.IsValid())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (UFlowSubsystem* FlowSubsystem = GI->GetSubsystem<UFlowSubsystem>())
			{
				const TSet<AActor*>& FoundActors = FlowSubsystem->GetFlowActorsByTag(TargetIdentityTag, AActor::StaticClass());
				TargetActors = FoundActors.Array();
			}
		}
	}
	else
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			if (AActor* PlayerPawn = PC->GetPawn())
			{
				TargetActors.Add(PlayerPawn);
			}
		}
	}

	// 2. Check the tag via ASC on any actor
	bool bHasTag = false;
	for (AActor* Actor : TargetActors)
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor))
		{
			if (ASC->HasMatchingGameplayTag(TagToCheck))
			{
				bHasTag = true;
				break;
			}
		}
	}

	// 3. Branch the flow
	const FName OutputPin = bHasTag ? FName("True") : FName("False");
	if (!bHasTag)
	{
	}
		
	TriggerOutput(OutputPin, true);
}
