#include "UI/Widgets/ActivatableWidgetBase.h"

#include "UI/Elements/ButtonBase.h"
#include "Input/CommonUIInputTypes.h"
#include "UI/Subsystem/UISubsystem.h"

#pragma region METHODS
void UActivatableWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (BackButton)
	{
		BackButton->OnClicked().RemoveAll(this);
		BackButton->OnClicked().AddUObject(this, &ThisClass::HandleBackActionBinding);
		if (!BackInputAction.IsNull())
		{
			BackButton->SetTriggeringInputAction(BackInputAction);
		}
	}
}

void UActivatableWidgetBase::NativeOnActivated()
{
	Super::NativeOnActivated();
	
	// Register the back action dynamically if a data table row is provided and no physical BackButton exists
	if (!BackButton && !BackInputAction.IsNull())
	{
		FBindUIActionArgs BindArgs(BackInputAction, FSimpleDelegate::CreateUObject(this, &ThisClass::HandleBackActionBinding));
		BindArgs.bDisplayInActionBar = true;
        
		BackHandle = RegisterUIActionBinding(BindArgs);
	}
}

void UActivatableWidgetBase::NativeOnDeactivated()
{
	// Unregister the back action
	if (BackHandle.IsValid())
		BackHandle.Unregister();
	
	Super::NativeOnDeactivated();
}

void UActivatableWidgetBase::HandleBackActionBinding()
{
	if (bBackActionBlocked)
		return;

	if (IsActivated())
		DeactivateWidget();
}

void UActivatableWidgetBase::SetBackActionBlocked(bool bBlocked)
{
	bBackActionBlocked = bBlocked;

	if (BackButton)
		BackButton->SetIsEnabled(!bBlocked);
}

TOptional<FUIInputConfig> UActivatableWidgetBase::GetDesiredInputConfig() const
{
	switch (InputConfig)
	{
		case E_WidgetInputMode::GameAndMenu:
			return FUIInputConfig(ECommonInputMode::All, GameMouseCaptureMode);
		case E_WidgetInputMode::Game:
			return FUIInputConfig(ECommonInputMode::Game, GameMouseCaptureMode);
		case E_WidgetInputMode::Menu:
			return FUIInputConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture);
		case E_WidgetInputMode::Default:
		default:
			return TOptional<FUIInputConfig>();
	}
}
#pragma endregion

#pragma region UFUNCTIONS
void UActivatableWidgetBase::RouteAction_Implementation(FGameplayTag ActionTag, UButtonBase* ClickedButton)
{
}

UWidget* UActivatableWidgetBase::GetPreferredFocusTarget_Implementation() const
{
	return BP_GetDesiredFocusTarget();
}
#pragma endregion