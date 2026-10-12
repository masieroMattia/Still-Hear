#pragma once

#include "Nodes/FlowNode.h"
#include "GameplayTagContainer.h"
#include "FlowNode_ShowWidget.generated.h"

class UCommonActivatableWidget;
class UAbilitySystemComponent;
class UInputAction;

UENUM(BlueprintType)
enum class EWidgetWaitCondition : uint8
{
	Timer,
	GameplayTag,
	Manual
};

/**
 * FlowNode that pushes a widget to the UI Subsystem and waits for a condition to finish.
 * Useful for tutorials, hints, and generic UI popups.
 */
UCLASS(NotBlueprintable, meta = (DisplayName = "Show Widget"))
class STILLHEAR_API UFlowNode_ShowWidget : public UFlowNode
{
	GENERATED_BODY()

public:
	UFlowNode_ShowWidget();

protected:
	/** The widget class to display */
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UCommonActivatableWidget> WidgetClass;

	/** The layer where to push the widget (e.g. UI.Layer.Game) */
	UPROPERTY(EditAnywhere, Category = "UI", meta = (Categories = "UI.Layer"))
	FGameplayTag LayerTag;

	/** If true, pauses the game while the widget is visible and unpauses when it is hidden */
	UPROPERTY(EditAnywhere, Category = "UI")
	bool bPauseGame = false;

	/** The text to display in the widget (requires ITutorialWidgetInterface) */
	UPROPERTY(EditAnywhere, Category = "UI")
	FText MessageText;

	/** Input actions to bind to the placeholders in the MessageText */
	UPROPERTY(EditAnywhere, Category = "UI")
	TArray<UInputAction*> InputActions;

	/** Optional: The Identity Tag of the actor to monitor for the tag condition. If empty, monitors the Player. */
	UPROPERTY(EditAnywhere, Category = "UI")
	FGameplayTag TargetIdentityTag;

	/** Whether to wait for a timer, a Gameplay Tag, or manual hide to finish */
	UPROPERTY(EditAnywhere, Category = "UI")
	EWidgetWaitCondition WaitCondition;

	/** If WaitCondition is Timer: how long to show the widget */
	UPROPERTY(EditAnywhere, Category = "UI", meta = (EditCondition = "WaitCondition == EWidgetWaitCondition::Timer"))
	float TimerDuration;

	/** If WaitCondition is GameplayTag: which tag must be added to the target actor to finish */
	UPROPERTY(EditAnywhere, Category = "UI", meta = (EditCondition = "WaitCondition == EWidgetWaitCondition::GameplayTag"))
	FGameplayTag ConditionTag;

	/** The active widget instance */
	UPROPERTY()
	TObjectPtr<UCommonActivatableWidget> ActiveWidget;

	/** Handle for the GAS tag listener */
	FDelegateHandle TagDelegateHandle;

	/** Handle for the timer */
	FTimerHandle TimerHandle_Widget;

	/** Weak pointer to the target's ASC */
	TWeakObjectPtr<UAbilitySystemComponent> WeakASC;

public:
	virtual void ExecuteInput(const FName& PinName) override;
	virtual void OnLoad_Implementation() override;
	virtual void Cleanup() override;

protected:
	/** Instantiates and shows the widget */
	void ShowWidget();
	
	/** Deactivates and clears the widget and listeners */
	void HideWidget();

	/** Called when the wait condition (timer or tag) is satisfied */
	void OnWaitConditionMet();

	/** Triggered when the condition tag is detected */
	void OnGameplayTagChanged(const FGameplayTag Tag, int32 NewCount);
};
