// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/Controls/Keyboard/KeyboardMoveRowWidget.h"

#include "Components/Image.h"
#include "Input/InputSubsystem.h"

void UKeyboardMoveRowWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UKeyboardMoveRowWidget::SetInitialGlyph()
{
	UInputSubsystem* InputSubsystem = GetGameInstance()->GetSubsystem<UInputSubsystem>();

	if (!InputSubsystem)
		return;
	
	CurrentKey = InputSubsystem->GetCurrentKeyForMoveDirection(InputAction, Direction);
	ReboundKey = CurrentKey;

	SetGlyphFromKey(CurrentKey);

	if (!InputSubsystem->IsInputActionSetToDefault(InputAction, CurrentKey))
	{
		OnKeyIsNotDefault.Broadcast();
	}
}
