// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/Controls/ControlsSettingsWidget.h"

#include "Components/SizeBox.h"
#include "SaveSystem/SaveSubsystem.h"
#include "Input/InputSubsystem.h"
#include "UI/Elements/ButtonBase.h"
#include "UI/Widgets/Controls/Gamepad/GamepadBindingsWidget.h"
#include "UI/Widgets/Controls/Keyboard/BindingsPageWidgetBase.h"

void UControlsSettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (const UGameInstance* GI = GetGameInstance())
	{
		InputSubsystem = GI->GetSubsystem<UInputSubsystem>();
		SaveSubsystem = GI->GetSubsystem<USaveSubsystem>();
	}

	ControllerButton->OnClicked().AddUObject(this, &ThisClass::ShowControllerControls);
	KeyboardButton->OnClicked().AddUObject(this, &ThisClass::ShowKeyboardControls);
	
	KeyboardBindingsPage->SetVisibility(ESlateVisibility::Collapsed);
	GamepadBindingsWidget->SetVisibility(ESlateVisibility::Collapsed);
}

void UControlsSettingsWidget::CloseBindingsPage() const
{
	KeyboardBindingsPage->SetVisibility(ESlateVisibility::Collapsed);
	GamepadBindingsWidget->SetVisibility(ESlateVisibility::Collapsed);
	ButtonsContainer->SetVisibility(ESlateVisibility::Visible);
	ControllerButton->SetFocus();

	OnBindingsPageClosed.Broadcast();
}

bool UControlsSettingsWidget::IsBindingsPageOpen() const
{
	return GamepadBindingsWidget->IsVisible() || KeyboardBindingsPage->IsVisible();
}

void UControlsSettingsWidget::ShowControllerControls() const
{
	HideMainButtons();

	GamepadBindingsWidget->SetVisibility(ESlateVisibility::Visible);
	GamepadBindingsWidget->SetFocus();
	OnBindingsPageOpened.Broadcast();
}

void UControlsSettingsWidget::ShowKeyboardControls() const
{
	HideMainButtons();
	
	KeyboardBindingsPage->SetVisibility(ESlateVisibility::Visible);
	KeyboardBindingsPage->SetFocus();
	OnBindingsPageOpened.Broadcast();
}

void UControlsSettingsWidget::HideMainButtons() const
{
	ButtonsContainer->SetVisibility(ESlateVisibility::Collapsed);
}
