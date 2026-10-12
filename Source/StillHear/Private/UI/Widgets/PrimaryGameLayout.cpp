#include "UI/Widgets/PrimaryGameLayout.h"

#include "Flow/SceneManager.h"
#include "Kismet/GameplayStatics.h"
#include "CommonActivatableWidget.h"
#include "GameFramework/PlayerController.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

#pragma region UFUNCTIONS
void UPrimaryGameLayout::RegisterLayer(const FGameplayTag LayerTag, UCommonActivatableWidgetStack* LayerWidget)
{
	if (!LayerTag.IsValid() || !LayerWidget) 
		return;

	Layers.Add(LayerTag, LayerWidget);
}

UCommonActivatableWidget* UPrimaryGameLayout::PushWidgetToLayer(const FGameplayTag LayerTag, const TSubclassOf<UCommonActivatableWidget> WidgetClass, const bool bClearLayer, const bool bPauseGame)
{
	if (!WidgetClass || !Layers.Contains(LayerTag)) 
		return nullptr;

	UCommonActivatableWidgetStack* TargetStack = Layers[LayerTag];
	if (!TargetStack) 
		return nullptr;

	if (bClearLayer)
		TargetStack->ClearWidgets();
	
	UCommonActivatableWidget* PushedWidget = TargetStack->AddWidget(WidgetClass);

	if (PushedWidget && bPauseGame)
	{
		// Register before calling SetPause: SetPause can synchronously trigger side effects
		// (e.g. ability cancellation -> gameplay tag -> Flow ClearUILayer on this same layer)
		// that must see this widget as a pausing widget so it isn't removed by ClearLayer
		const bool bWasEmpty = PausingWidgets.Num() == 0;
		PausingWidgets.Add(PushedWidget);
		PushedWidget->OnDeactivated().AddUObject(this, &UPrimaryGameLayout::OnPausingWidgetDeactivated, PushedWidget);

		if (bWasEmpty)
		{
			if (APlayerController* PC = GetOwningPlayer())
			{
				PC->SetPause(true);
			}
		}
	}

	return PushedWidget;
}

void UPrimaryGameLayout::ClearLayer(FGameplayTag LayerTag)
{
	if (TObjectPtr<UCommonActivatableWidgetStack>* TargetStackPtr = Layers.Find(LayerTag))
	{
		if (UCommonActivatableWidgetStack* TargetStack = *TargetStackPtr)
		{
			const TArray<UCommonActivatableWidget*> WidgetsToClear = TargetStack->GetWidgetList();
			for (UCommonActivatableWidget* Widget : WidgetsToClear)
			{
				// Don't remove widgets that are currently holding the game paused
				if (Widget && !PausingWidgets.Contains(Widget))
					TargetStack->RemoveWidget(*Widget);
			}
		}
	}
}

void UPrimaryGameLayout::OnPausingWidgetDeactivated(UCommonActivatableWidget* DeactivatedWidget)
{
	if (DeactivatedWidget)
	{
		DeactivatedWidget->OnDeactivated().RemoveAll(this);
		PausingWidgets.Remove(DeactivatedWidget);
	}

	if (PausingWidgets.Num() == 0)
	{
		if (APlayerController* PC = GetOwningPlayer())
		{
			PC->SetPause(false);

			const ASceneManager* SM = Cast<ASceneManager>(
				UGameplayStatics::GetActorOfClass(GetWorld(), ASceneManager::StaticClass()));
			if (!SM || !SM->IsInMenuMode())
				PC->SetInputMode(FInputModeGameOnly());
		}
	}
}
#pragma endregion
