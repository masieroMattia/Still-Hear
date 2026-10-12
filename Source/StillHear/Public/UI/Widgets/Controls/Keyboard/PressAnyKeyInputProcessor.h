#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "Framework/Application/IInputProcessor.h"
#include "Framework/Application/SlateApplication.h"

class FPressAnyKeyInputProcessor : public IInputProcessor
{
public:
	// Accepted key as new binding
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnKeySelected, const FKey&);
	
	// Canceled (ESC or pad's button face right)
	DECLARE_MULTICAST_DELEGATE(FOnKeySelectionCanceled);
	
	FOnKeySelected OnKeySelected;
	FOnKeySelectionCanceled OnKeySelectionCanceled;

	// This is useful to ignore any gamepad button pressed.
	// In this project we are not rebinding gamepad buttons one by one, but if we decide to do that,
	// then in that case this would be true. We already have the logic for that
	bool bAcceptGamepadKeys = false;

	// Ready to capture becomes true during the first tick and then stays true, so we don't risk capturing
	// left mouse button at the same time we click the input row to rebind the button
	virtual void Tick(const float, FSlateApplication&, TSharedRef<ICursor>) override;

	virtual bool HandleKeyDownEvent(FSlateApplication&, const FKeyEvent& Ev) override;

	virtual bool HandleMouseButtonDownEvent(FSlateApplication&, const FPointerEvent& Ev) override;

	virtual bool HandleMouseWheelOrGestureEvent(FSlateApplication&, const FPointerEvent& Ev, const FPointerEvent*) override;

	// We still process gamepad keys by basically ignoring it, just because by doing so it will not get processed by the game underneath
	virtual bool HandleAnalogInputEvent(FSlateApplication&, const FAnalogInputEvent&) override;

private:
	bool bReadyToCapture = false;

	bool ProcessKey(const FKey& Key);
};
