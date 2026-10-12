#include "UI/Widgets/Controls/Keyboard/PressAnyKeyWidget.h"
#include "UI/Widgets/Controls/Keyboard/PressAnyKeyInputProcessor.h"
#include "Input/InputDeviceType.h"
#include "Framework/Application/SlateApplication.h"

void UPressAnyKeyWidget::InitializeForDevice(EInputDeviceType InDeviceType)
{
	bAcceptGamepadKeys = (InDeviceType == EInputDeviceType::Controller);
	if (InputProcessor.IsValid())
		InputProcessor->bAcceptGamepadKeys = bAcceptGamepadKeys;
}

void UPressAnyKeyWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	if (!FSlateApplication::IsInitialized())
		return;

	InputProcessor = MakeShared<FPressAnyKeyInputProcessor>();
	InputProcessor->bAcceptGamepadKeys = bAcceptGamepadKeys;
	InputProcessor->OnKeySelected.AddUObject(this, &UPressAnyKeyWidget::HandleKeySelected);
	InputProcessor->OnKeySelectionCanceled.AddUObject(this, &UPressAnyKeyWidget::HandleCanceled);

	// This ensures that any input we receive as a key to rebind, we are the FIRST to see it.
	// By doing this, the input will not get intercepted by other systems before us.
	FSlateApplication::Get().RegisterInputPreProcessor(InputProcessor, 0);
}

void UPressAnyKeyWidget::NativeOnDeactivated()
{
	if (InputProcessor.IsValid() && FSlateApplication::IsInitialized())
		FSlateApplication::Get().UnregisterInputPreProcessor(InputProcessor);

	// Must do this: avoids leaks/crashes
	InputProcessor.Reset();

	Super::NativeOnDeactivated();
}

void UPressAnyKeyWidget::HandleKeySelected(const FKey& Key)
{
	OnKeySelected.Broadcast(Key);
	DeactivateWidget();
}

void UPressAnyKeyWidget::HandleCanceled()
{
	OnCanceled.Broadcast();
	DeactivateWidget();
}
