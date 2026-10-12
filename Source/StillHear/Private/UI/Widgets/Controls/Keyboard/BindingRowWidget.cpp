// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/Controls/Keyboard/BindingRowWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Input/InputSubsystem.h"
#include "UI/Elements/ButtonBase.h"

void UBindingRowWidget::NativeConstruct()
{
	Super::NativeConstruct();	
	SetIsFocusable(true);
}

void UBindingRowWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (ActionNameText)
		ActionNameText->SetText(DisplayText);
}

void UBindingRowWidget::NativeOnClicked()
{
	Super::NativeOnClicked();
	OnRebindRequested.Broadcast(this, InputAction, CurrentKey, DeviceType);
}

void UBindingRowWidget::SetGlyphFromKey(const FKey& Key) const
{
	const UInputSubsystem* InputSubsystem = GetGameInstance()->GetSubsystem<UInputSubsystem>();

	if (!InputSubsystem)
		return;
	
	const FSlateBrush GlyphBrush = InputSubsystem->GetBrushFromKey(Key, DeviceType);
	KeyGlyph->SetBrush(GlyphBrush);
}

void UBindingRowWidget::SetInitialGlyph()
{
	UInputSubsystem* InputSubsystem = GetGameInstance()->GetSubsystem<UInputSubsystem>();

	if (!InputSubsystem)
		return;
	
	CurrentKey = InputSubsystem->GetCurrentKeyForAction(InputAction, DeviceType);
	ReboundKey = CurrentKey;

	SetGlyphFromKey(CurrentKey);

	if (!InputSubsystem->IsInputActionSetToDefault(InputAction, CurrentKey))
	{
		OnKeyIsNotDefault.Broadcast();
	}
}
