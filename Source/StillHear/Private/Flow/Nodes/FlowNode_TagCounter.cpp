#include "Flow/Nodes/FlowNode_TagCounter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "FlowSubsystem.h"

UFlowNode_TagCounter::UFlowNode_TagCounter()
{
#if WITH_EDITOR
	Category = TEXT("Logic");
	NodeDisplayStyle = FlowNodeStyle::Logic;
#endif

	InputPins.Empty();
	InputPins.Add(FFlowPin(FName("Start")));
	InputPins.Add(FFlowPin(FName("Stop")));
	InputPins.Add(FFlowPin(FName("Reset")));

	OutputPins.Empty();
	OutputPins.Add(FFlowPin(FName("Out")));
	OutputPins.Add(FFlowPin(FName("ThresholdReached")));
	
	Threshold = 3;
	CurrentCount = 0;
}

void UFlowNode_TagCounter::ExecuteInput(const FName& PinName)
{
	if (PinName == FName("Start"))
	{
		StartTracking();
		TriggerOutput(FName("Out"), false);
	}
	else if (PinName == FName("Stop"))
	{
		StopTracking();
	}
	else if (PinName == FName("Reset"))
	{
		CurrentCount = 0;
	}
}

void UFlowNode_TagCounter::OnLoad_Implementation()
{
	if (ActivationState == EFlowNodeState::Active)
	{
		StartTracking();
	}
}

void UFlowNode_TagCounter::StartTracking()
{
	UWorld* World = GetWorld();
	if (!World) return;

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

	bool bFoundAnyASC = false;
	for (AActor* Actor : TargetActors)
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor))
		{
			bFoundAnyASC = true;
			TWeakObjectPtr<UAbilitySystemComponent> WeakASC = ASC;
			
			if (!TrackedASCs.Contains(WeakASC))
			{
				FDelegateHandle Handle = ASC->RegisterGameplayTagEvent(TagToCount, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &UFlowNode_TagCounter::OnGameplayTagChanged);
				TrackedASCs.Add(WeakASC, Handle);
			}
		}
	}

	if (!bFoundAnyASC)
	{
		// Retry later if no actors with ASC were found
		FTimerHandle TempHandle;
		World->GetTimerManager().SetTimer(TempHandle, this, &UFlowNode_TagCounter::StartTracking, 0.5f, false);
	}
}

void UFlowNode_TagCounter::StopTracking()
{
	for (auto& Pair : TrackedASCs)
	{
		if (UAbilitySystemComponent* ASC = Pair.Key.Get())
		{
			if (Pair.Value.IsValid())
			{
				ASC->RegisterGameplayTagEvent(TagToCount, EGameplayTagEventType::NewOrRemoved).Remove(Pair.Value);
			}
		}
	}
	TrackedASCs.Empty();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearAllTimersForObject(this);
	}
}

void UFlowNode_TagCounter::OnGameplayTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (Tag == TagToCount && NewCount > 0)
	{
		CurrentCount++;

		if (CurrentCount >= Threshold)
		{
			TriggerOutput(FName("ThresholdReached"), true);
			StopTracking();
		}
	}
}

void UFlowNode_TagCounter::Cleanup()
{
	StopTracking();
	Super::Cleanup();
}
