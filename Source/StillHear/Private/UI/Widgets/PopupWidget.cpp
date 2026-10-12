#include "UI/Widgets/PopupWidget.h"

#include "TimerManager.h"
#include "Engine/World.h"
#include "CommonTextBlock.h"

#pragma region METHODS
void UPopupWidget::NativeOnActivated()
{
	Super::NativeOnActivated();
}

void UPopupWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	// Clean up the timer to prevent memory leaks or unwanted calls
	GetWorld()->GetTimerManager().ClearTimer(LifetimeTimerHandle);
}

void UPopupWidget::OnLifetimeExpired()
{
	// Deactivate the widget, removing it from the active layer
	DeactivateWidget();
}
	
void UPopupWidget::InitializePopup(const FText& InMessage, const float InDuration)
{
	PopupMessage = InMessage;
	PopupDuration = InDuration;

	if (MessageText)
		MessageText->SetText(PopupMessage);

	// Start a timer to remove or hide the widget automatically
	if (PopupDuration > 0.0f)
		GetWorld()->GetTimerManager().SetTimer(LifetimeTimerHandle, this, &UPopupWidget::OnLifetimeExpired, PopupDuration, false);
}
#pragma endregion
	
#pragma region INTERFACE METHODS
void UPopupWidget::InitializeWidget_Implementation(const FText& Text, const float Duration, const TArray<UInputAction*>& InputActions)
{
	// Call your existing initialization logic
	InitializePopup(Text, Duration);
}
#pragma endregion
