#include "Flow/Nodes/FlowNode_ShowWidget.h"

#include "TimerManager.h"
#include "Engine/World.h"
#include "FlowSubsystem.h"
#include "Engine/LocalPlayer.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemComponent.h"
#include "CommonActivatableWidget.h"
#include "UI/Subsystem/UISubsystem.h"
#include "UI/Widgets/UIWidgetInterface.h"
#include "GameFramework/PlayerController.h"

UFlowNode_ShowWidget::UFlowNode_ShowWidget()
{
#if WITH_EDITOR
	Category = TEXT("UI");
	NodeDisplayStyle = FlowNodeStyle::Logic;
#endif

	InputPins.Empty();
	InputPins.Add(FFlowPin(FName("In")));
	InputPins.Add(FFlowPin(FName("Hide")));

	OutputPins.Empty();
	OutputPins.Add(FFlowPin(FName("Out")));
	OutputPins.Add(FFlowPin(FName("Completed")));
	
	WaitCondition = EWidgetWaitCondition::Timer;
	TimerDuration = 5.0f;
}

void UFlowNode_ShowWidget::ExecuteInput(const FName& PinName)
{
	if (PinName == FName("In"))
	{
		ShowWidget();
		TriggerOutput(FName("Out"), false);
	}
	else if (PinName == FName("Hide"))
	{
		HideWidget();

		// Without this the node stays ActivationState::Active, gets saved as such, and
		// OnLoad_Implementation re-shows the widget on load even with no sequence playing.
		Finish();
	}
}

void UFlowNode_ShowWidget::OnLoad_Implementation()
{
	if (ActivationState == EFlowNodeState::Active)
	{
		ShowWidget();
	}
}

void UFlowNode_ShowWidget::ShowWidget()
{
	if (!WidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShowWidget: No WidgetClass selected!"));
		return;
	}

	// If a widget is already active, hide it before showing a new one. 
	// This ensures that we don't end up with multiple widgets if the node is triggered again before the first one finishes
	if (ActiveWidget)
	{
		HideWidget();
	}

	UWorld* World = GetWorld();
	if (!World) return;

	// 1. Show UI
	if (!ActiveWidget)
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
			{
				if (UUISubsystem* UISubsystem = LocalPlayer->GetSubsystem<UUISubsystem>())
				{
					if (!LayerTag.IsValid())
					{
						UE_LOG(LogTemp, Error, TEXT("ShowWidget: LayerTag is invalid! Please select a UI Layer in the node settings."));
						return;
					}

					ActiveWidget = UISubsystem->PushWidgetToLayer(LayerTag, WidgetClass, false, bPauseGame);

					if (ActiveWidget)
					{
						
						if (ActiveWidget->Implements<UUIWidgetInterface>())
						{
							const float DurationToSend = (WaitCondition == EWidgetWaitCondition::Timer) ? TimerDuration : 0.0f;
							IUIWidgetInterface::Execute_InitializeWidget(ActiveWidget, MessageText, DurationToSend, InputActions);
						}
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("ShowWidget: UISubsystem failed to push widget. Layer might not be ready. Retrying..."));
						// Fall through to retry logic
					}
				}
			}
		}

		if (!ActiveWidget)
		{
			// Retry for dynamic UI initialization
			FTimerHandle TempHandle;
			World->GetTimerManager().SetTimer(TempHandle, this, &UFlowNode_ShowWidget::ShowWidget, 0.5f, false);
			return;
		}
	}

	// 2. Setup Wait Condition
	if (WaitCondition == EWidgetWaitCondition::Timer)
	{
		if (!World->GetTimerManager().IsTimerActive(TimerHandle_Widget))
		{
			World->GetTimerManager().SetTimer(TimerHandle_Widget, this, &UFlowNode_ShowWidget::OnWaitConditionMet, TimerDuration, false);
		}
	}
	else if (WaitCondition == EWidgetWaitCondition::GameplayTag)
	{
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

				if (ASC->GetTagCount(ConditionTag) > 0)
				{
					OnWaitConditionMet();
					return;
				}

				if (!TagDelegateHandle.IsValid())
				{
					TagDelegateHandle = ASC->RegisterGameplayTagEvent(ConditionTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &UFlowNode_ShowWidget::OnGameplayTagChanged);
				}
				return;
			}
		}

		FTimerHandle TempHandle;
		World->GetTimerManager().SetTimer(TempHandle, this, &UFlowNode_ShowWidget::ShowWidget, 0.5f, false);
	}
}

void UFlowNode_ShowWidget::HideWidget()
{
	if (ActiveWidget)
	{
		ActiveWidget->DeactivateWidget();
		ActiveWidget = nullptr;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TimerHandle_Widget);
		// Rimosso ClearAllTimersForObject perché troppo aggressivo e causa lag/conflitti col sistema di retry
	}

	if (UAbilitySystemComponent* ASC = WeakASC.Get())
	{
		if (TagDelegateHandle.IsValid())
		{
			ASC->RegisterGameplayTagEvent(ConditionTag, EGameplayTagEventType::NewOrRemoved).Remove(TagDelegateHandle);
			TagDelegateHandle.Reset();
		}
	}
	WeakASC.Reset();
}

void UFlowNode_ShowWidget::OnWaitConditionMet()
{
	HideWidget();
	TriggerOutput(FName("Completed"), true);
}

void UFlowNode_ShowWidget::OnGameplayTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount > 0)
	{
		OnWaitConditionMet();
	}
}

void UFlowNode_ShowWidget::Cleanup()
{
	HideWidget();
	Super::Cleanup();
}
