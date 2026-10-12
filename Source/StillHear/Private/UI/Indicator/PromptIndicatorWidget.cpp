#include "UI/Indicator/PromptIndicatorWidget.h"

#include "Blueprint/UserWidget.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "UI/Widgets/InputActionWidget.h"
#include "UI/Indicator/IndicatorDescriptor.h"

#pragma region METHODS
void UPromptIndicatorWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (Descriptor)
    {
        Descriptor->OnIndicatorUpdated.AddUniqueDynamic(this, &UPromptIndicatorWidget::UpdateInputPrompt);
    }

    UpdateInputPrompt();
}

void UPromptIndicatorWidget::NativeDestruct()
{
    if (Descriptor)
    {
        Descriptor->OnIndicatorUpdated.RemoveDynamic(this, &UPromptIndicatorWidget::UpdateInputPrompt);
    }

    Super::NativeDestruct();
}
#pragma endregion

#pragma region UFUNCTIONS
void UPromptIndicatorWidget::UpdateInputPrompt()
{
	if (!Descriptor || !ActionContainer || !InputActionWidgetClass)
		return;

	ActionContainer->ClearChildren();

	const TArray<TObjectPtr<UInputAction>>& Actions = Descriptor->InputActions;

	for (int32 i = 0; i < Actions.Num(); ++i)
	{
		UInputAction* Action = Actions[i];
		if (!Action) 
			continue;

		// Add separator if not the first element
		const TSubclassOf<UUserWidget> ActualSeparatorClass = Descriptor->SeparatorClass ? Descriptor->SeparatorClass : SeparatorWidgetClass;
		if (i > 0 && ActualSeparatorClass)
		{
			if (UUserWidget* SeparatorWidget = CreateWidget<UUserWidget>(this, ActualSeparatorClass))
			{
				if (UHorizontalBoxSlot* HBoxSlot = ActionContainer->AddChildToHorizontalBox(SeparatorWidget))
				{
					HBoxSlot->SetVerticalAlignment(VAlign_Center);
				}
			}
		}

		// Spawn InputActionWidget
		if (UInputActionWidget* ActionWidget = CreateWidget<UInputActionWidget>(this, InputActionWidgetClass))
		{
			ActionWidget->SetupInputAction(Action);
			
			if (UHorizontalBoxSlot* HBoxSlot = ActionContainer->AddChildToHorizontalBox(ActionWidget))
			{
				HBoxSlot->SetVerticalAlignment(VAlign_Center);
				HBoxSlot->SetHorizontalAlignment(HAlign_Center);
			}
		}
	}
}
#pragma endregion

