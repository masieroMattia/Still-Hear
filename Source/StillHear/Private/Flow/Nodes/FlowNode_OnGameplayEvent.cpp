#include "Flow/Nodes/FlowNode_OnGameplayEvent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "FlowSubsystem.h"
#include "GameplayTagAssetInterface.h"

UFlowNode_OnGameplayEvent::UFlowNode_OnGameplayEvent()
{
#if WITH_EDITOR
	Category = TEXT("Events");
	NodeDisplayStyle = FlowNodeStyle::Logic;
#endif

	InputPins.Empty();
	InputPins.Add(FFlowPin(FName("Start")));
	InputPins.Add(FFlowPin(FName("Stop")));

	OutputPins.Empty();
	OutputPins.Add(FFlowPin(FName("EventReceived")));
}

void UFlowNode_OnGameplayEvent::ExecuteInput(const FName& PinName)
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

void UFlowNode_OnGameplayEvent::OnLoad_Implementation()
{
	if (ActivationState == EFlowNodeState::Active)
	{
		StartMonitoring();
	}
}

void UFlowNode_OnGameplayEvent::StartMonitoring()
{
	UWorld* World = GetWorld();
	if (!World) return;

	AActor* TargetActor = nullptr;

	// 1. Resolve the Target Actor (who receives the event)
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
		// Default to Player
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			TargetActor = PC->GetPawn();
		}
	}

	// 2. Subscribe to the event if the actor has an ASC
	if (TargetActor)
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor))
		{
			WeakASC = ASC;

			if (!EventDelegateHandle.IsValid())
			{
				EventDelegateHandle = ASC->GenericGameplayEventCallbacks.FindOrAdd(EventTag).AddUObject(this, &UFlowNode_OnGameplayEvent::OnGameplayEventReceived);
			}
			return;
		}
	}

	// 3. Retry logic if the actor is not yet available
	FTimerHandle TempHandle;
	World->GetTimerManager().SetTimer(TempHandle, this, &UFlowNode_OnGameplayEvent::StartMonitoring, 0.5f, false);
}

void UFlowNode_OnGameplayEvent::StopMonitoring()
{
	if (UAbilitySystemComponent* ASC = WeakASC.Get())
	{
		if (EventDelegateHandle.IsValid())
		{
			ASC->GenericGameplayEventCallbacks.FindOrAdd(EventTag).Remove(EventDelegateHandle);
			EventDelegateHandle.Reset();
		}
	}
	WeakASC.Reset();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearAllTimersForObject(this);
	}
}

void UFlowNode_OnGameplayEvent::OnGameplayEventReceived(const FGameplayEventData* Payload)
{
	if (!Payload) return;

	// Optional filtering: check if the source object matches a specific Identity Tag
	if (RequiredPayloadSourceTag.IsValid())
	{
		bool bSourceMatches = false;
		if (Payload->OptionalObject)
		{
			if (const IGameplayTagAssetInterface* TagInterface = Cast<const IGameplayTagAssetInterface>(Payload->OptionalObject))
			{
				FGameplayTagContainer SourceTags;
				TagInterface->GetOwnedGameplayTags(SourceTags);
				if (SourceTags.HasTag(RequiredPayloadSourceTag))
				{
					bSourceMatches = true;
				}
			}
		}

		if (!bSourceMatches)
		{
			return; // Received event from a different source than requested
		}
	}

	TriggerOutput(FName("EventReceived"), true);
	StopMonitoring();
}

void UFlowNode_OnGameplayEvent::Cleanup()
{
	StopMonitoring();
	Super::Cleanup();
}
