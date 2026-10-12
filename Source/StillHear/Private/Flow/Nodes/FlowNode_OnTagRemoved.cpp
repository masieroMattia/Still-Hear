#include "Flow/Nodes/FlowNode_OnTagRemoved.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "FlowSubsystem.h"

UFlowNode_OnTagRemoved::UFlowNode_OnTagRemoved()
{
#if WITH_EDITOR
	Category = TEXT("Logic");
	NodeDisplayStyle = FlowNodeStyle::Logic;
#endif

	InputPins.Empty();
	InputPins.Add(FFlowPin(FName("Start")));
	InputPins.Add(FFlowPin(FName("Stop")));

	OutputPins.Empty();
	OutputPins.Add(FFlowPin(FName("Removed")));
}

void UFlowNode_OnTagRemoved::ExecuteInput(const FName& PinName)
{
	if (PinName == FName("Start"))
	{
		StartMonitoring();
	}
	else if (PinName == FName("Stop"))
	{
		StopMonitoring();
	}
}

void UFlowNode_OnTagRemoved::OnLoad_Implementation()
{
	if (ActivationState == EFlowNodeState::Active)
	{
		StartMonitoring();
	}
}

void UFlowNode_OnTagRemoved::StartMonitoring()
{
	UWorld* World = GetWorld();
	if (!World) return;

	AActor* TargetActor = nullptr;

	if (TargetIdentityTag.IsValid())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (UFlowSubsystem* FlowSubsystem = GI->GetSubsystem<UFlowSubsystem>())
			{
				const TSet<AActor*>& FoundActors = FlowSubsystem->GetFlowActorsByTag(TargetIdentityTag, AActor::StaticClass());
				if (FoundActors.Num() > 0)
				{
					TargetActor = *FoundActors.CreateConstIterator();
				}
			}
		}
	}
	else
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			TargetActor = PC->GetPawn();
		}
	}

	if (TargetActor)
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor))
		{
			WeakASC = ASC;

			// Check if already removed
			if (ASC->GetTagCount(TagToWatch) == 0)
			{
				TriggerOutput(FName("Removed"), true);
				return;
			}

			if (!TagDelegateHandle.IsValid())
			{
				TagDelegateHandle = ASC->RegisterGameplayTagEvent(TagToWatch, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &UFlowNode_OnTagRemoved::OnGameplayTagChanged);
			}
			return;
		}
	}

	FTimerHandle TempHandle;
	World->GetTimerManager().SetTimer(TempHandle, this, &UFlowNode_OnTagRemoved::StartMonitoring, 0.5f, false);
}

void UFlowNode_OnTagRemoved::StopMonitoring()
{
	if (UAbilitySystemComponent* ASC = WeakASC.Get())
	{
		if (TagDelegateHandle.IsValid())
		{
			ASC->RegisterGameplayTagEvent(TagToWatch, EGameplayTagEventType::NewOrRemoved).Remove(TagDelegateHandle);
			TagDelegateHandle.Reset();
		}
	}
	WeakASC.Reset();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearAllTimersForObject(this);
	}
}

void UFlowNode_OnTagRemoved::OnGameplayTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (Tag == TagToWatch && NewCount == 0)
	{
		TriggerOutput(FName("Removed"), true);
		StopMonitoring();
	}
}

void UFlowNode_OnTagRemoved::Cleanup()
{
	StopMonitoring();
	Super::Cleanup();
}
