#include "UI/Elements/PopupButtonBase.h"

#include "UI/Subsystem/UISubsystem.h"
#include "UI/Widgets/PopupWidget.h"

#pragma region METHODS
void UPopupButtonBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
    
	// Bind the popup generation to the click event
	OnClicked().AddUObject(this, &UPopupButtonBase::ShowPopup);
}

void UPopupButtonBase::ShowPopup() const
{
	if (!PopupClass || !PopupLayerTag.IsValid())
		return;

	const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	if (!LocalPlayer)
		return;

	UUISubsystem* UISubsystem = LocalPlayer->GetSubsystem<UUISubsystem>();
	if (!UISubsystem)
		return;

	// Push the widget and pass 'true' to clear the layer first
	UCommonActivatableWidget* Widget = UISubsystem->PushWidgetToLayer(PopupLayerTag, PopupClass, true);
	UPopupWidget* Popup = Cast<UPopupWidget>(Widget);
    
	if (Popup)
		Popup->InitializePopup(PopupMessage, PopupDuration);
}
#pragma endregion