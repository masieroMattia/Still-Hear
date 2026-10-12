#include "UI/Widgets/Controls/Keyboard/PressAnyKeyInputProcessor.h"

void FPressAnyKeyInputProcessor::Tick(const float, FSlateApplication&, TSharedRef<ICursor>)
{
	bReadyToCapture = true;
}

bool FPressAnyKeyInputProcessor::HandleKeyDownEvent(FSlateApplication& SlateApplication, const FKeyEvent& Ev)
{
	return ProcessKey(Ev.GetKey());
}

bool FPressAnyKeyInputProcessor::HandleMouseButtonDownEvent(FSlateApplication& SlateApplication, const FPointerEvent& Ev)
{
	return ProcessKey(Ev.GetEffectingButton());
}

bool FPressAnyKeyInputProcessor::HandleMouseWheelOrGestureEvent(FSlateApplication& SlateApplication, const FPointerEvent& Ev, const FPointerEvent* PointerEvent)
{
	const FKey Wheel = Ev.GetWheelDelta() > 0.f ? EKeys::MouseScrollUp : EKeys::MouseScrollDown;
	return ProcessKey(Wheel);
}

bool FPressAnyKeyInputProcessor::HandleAnalogInputEvent(FSlateApplication& SlateApplication, const FAnalogInputEvent& AnalogInputEvent)
{
	return bReadyToCapture;
}

bool FPressAnyKeyInputProcessor::ProcessKey(const FKey& Key)
{
	if (!bReadyToCapture)
	{
		return true;
	}

	// Cancel: always works, doesn't matter the device
	if (Key == EKeys::Escape || Key == EKeys::Gamepad_FaceButton_Right)
	{
		OnKeySelectionCanceled.Broadcast();
		return true;
	}

	// Depending if we are rebinding keyboard or pad, we interrupt here if we receive the wrong kind of input
	if (!bAcceptGamepadKeys && Key.IsGamepadKey())
		return true;
	if (bAcceptGamepadKeys && !Key.IsGamepadKey())
		return true;

	OnKeySelected.Broadcast(Key);
	return true; // always true, so the action router doesn't see this button and we can't switch page from the content switcher
}
