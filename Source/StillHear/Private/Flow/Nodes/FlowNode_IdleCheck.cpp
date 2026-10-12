#include "Flow/Nodes/FlowNode_IdleCheck.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "FlowSubsystem.h"

UFlowNode_IdleCheck::UFlowNode_IdleCheck()
{
#if WITH_EDITOR
	Category = TEXT("Tutorial");
	NodeDisplayStyle = FlowNodeStyle::Logic;
#endif

	InputPins.Empty();
	InputPins.Add(FFlowPin(FName("Start")));
	InputPins.Add(FFlowPin(FName("Cancel")));

	OutputPins.Empty();
	OutputPins.Add(FFlowPin(FName("TimeoutReached")));
	OutputPins.Add(FFlowPin(FName("TagDetected")));

	IdleTime = 5.0f;
}

void UFlowNode_IdleCheck::ExecuteInput(const FName& PinName)
{
	if (PinName == FName("Start"))
	{
		StartCheck();
	}
	else if (PinName == FName("Cancel"))
	{
		StopCheck();
	}
}

void UFlowNode_IdleCheck::OnLoad_Implementation()
{
	if (ActivationState == EFlowNodeState::Active)
	{
		StartCheck();
	}
}

void UFlowNode_IdleCheck::StartCheck()
{
	UWorld* World = GetWorld();
	if (!World) return;

	// Reset trigger guard
	bHasTriggered = false;

	if (!World->GetTimerManager().IsTimerActive(TimerHandle_Idle))
	{
		World->GetTimerManager().SetTimer(TimerHandle_Idle, this, &UFlowNode_IdleCheck::OnIdleTimeout, IdleTime, false);
	}

	AActor* TargetActor = nullptr;

	// 1. Find Target Actor: Either by Identity Tag or default to Player
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

	// 2. If we have an actor, try to get its ASC
	if (TargetActor)
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor))
		{
			WeakASC = ASC;

			// Check initial state (Exact Match)
			if (ASC->GetTagCount(EventTagToCancel) > 0)
			{
				// Trigger TagDetected because tag present immediately
				if (!bHasTriggered)
				{
					// Only trigger if timer still active (i.e. timeout not reached)
					if (World && World->GetTimerManager().IsTimerActive(TimerHandle_Idle))
					{
						bHasTriggered = true;
						TriggerOutput(FName("TagDetected"), true);
					}
				}
				StopCheck();
				return;
			}

			// Register for Gameplay Event (Signals)
			if (!EventDelegateHandle.IsValid())
			{
				EventDelegateHandle = ASC->GenericGameplayEventCallbacks.FindOrAdd(EventTagToCancel).AddUObject(this, &UFlowNode_IdleCheck::OnGameplayEventReceived);
			}

			// Register for Tag Changes (State)
			if (!TagDelegateHandle.IsValid())
			{
				TagDelegateHandle = ASC->RegisterGameplayTagEvent(EventTagToCancel, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &UFlowNode_IdleCheck::OnGameplayTagChanged);
			}

			return;
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("IdleCheck: Target Actor/ASC not ready yet. Retrying in 0.5s..."));
	FTimerHandle TempHandle;
	World->GetTimerManager().SetTimer(TempHandle, this, &UFlowNode_IdleCheck::StartCheck, 0.5f, false);
}

void UFlowNode_IdleCheck::StopCheck()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TimerHandle_Idle);
		World->GetTimerManager().ClearAllTimersForObject(this);
	}

	if (UAbilitySystemComponent* ASC = WeakASC.Get())
	{
		if (EventDelegateHandle.IsValid())
		{
			ASC->GenericGameplayEventCallbacks.FindOrAdd(EventTagToCancel).Remove(EventDelegateHandle);
			EventDelegateHandle.Reset();
		}

		if (TagDelegateHandle.IsValid())
		{
			ASC->RegisterGameplayTagEvent(EventTagToCancel, EGameplayTagEventType::NewOrRemoved).Remove(TagDelegateHandle);
			TagDelegateHandle.Reset();
		}
	}
	WeakASC.Reset();
}

void UFlowNode_IdleCheck::OnIdleTimeout()
{
	StopCheck();
	if (!bHasTriggered)
	{
		bHasTriggered = true;
		TriggerOutput(FName("TimeoutReached"), true);
	}
}

void UFlowNode_IdleCheck::OnGameplayEventReceived(const FGameplayEventData* Payload)
{
	// Trigger TagDetected output to signal cancellation before timeout
	UWorld* World = GetWorld();
	if (!bHasTriggered && World && World->GetTimerManager().IsTimerActive(TimerHandle_Idle))
	{
		bHasTriggered = true;
		TriggerOutput(FName("TagDetected"), true);
	}
	StopCheck();
}

void UFlowNode_IdleCheck::OnGameplayTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount > 0)
	{
		// Trigger TagDetected output to signal cancellation before timeout
		UWorld* World = GetWorld();
		if (!bHasTriggered && World && World->GetTimerManager().IsTimerActive(TimerHandle_Idle))
		{
			bHasTriggered = true;
			TriggerOutput(FName("TagDetected"), true);
		}
		StopCheck();
	}
}

void UFlowNode_IdleCheck::Cleanup()
{
	StopCheck();
	Super::Cleanup();
}
