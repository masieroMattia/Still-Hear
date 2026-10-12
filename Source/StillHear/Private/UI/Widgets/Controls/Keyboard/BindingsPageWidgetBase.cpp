// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widgets/Controls/Keyboard/BindingsPageWidgetBase.h"

#include "UI/Elements/ButtonBase.h"
#include "Input/BindingData.h"
#include "Input/InputSubsystem.h"
#include "UI/Subsystem/UISubsystem.h"
#include "UI/Widgets/Controls/Keyboard/BindingRowWidget.h"
#include "UI/Widgets/Controls/Keyboard/BindingsListWidget.h"
#include "UI/Widgets/Controls/Keyboard/PressAnyKeyWidget.h"

void UBindingsPageWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	if (IsValid(ApplyButton))
	{
		ApplyButton->SetVisibility(ESlateVisibility::Hidden);
	}

	BindingsList->InitializeRows();
}

void UBindingsPageWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (IsValid(ResetButton))
	{
		ResetButton->SetVisibility(ESlateVisibility::Hidden);
	}
	
	BindingsList->OnFoundNotDefaultKey.AddUniqueDynamic(this, &ThisClass::DisplayResetToDefaultButton);
	BindingsList->InitializeRows();
	RowWidgets = BindingsList->GetRowWidgets();
	for (int i = 0; i < RowWidgets.Num(); i++)
	{
		RowWidgets[i]->OnRebindRequested.AddUniqueDynamic(this, &ThisClass::HandleRebindRequested);
	}
}

void UBindingsPageWidgetBase::OnApplyConfirmed()
{
	UInputSubsystem* InputSubsystem = GetGameInstance()->GetSubsystem<UInputSubsystem>();
	if (!InputSubsystem)
	{
		return;
	}

	TArray<FBindingData> Changes;
	PendingChanges.GenerateValueArray(Changes);

	InputSubsystem->AddToKeysToRebindArray(Changes);
	InputSubsystem->RebindKeys();
	InputSubsystem->SaveBindings();

	PendingChanges.Empty();

	if (ApplyButton)
	{
		ApplyButton->SetVisibility(ESlateVisibility::Hidden);
	}

	BindingsList->InitializeRows();
	RowWidgets = BindingsList->GetRowWidgets();
}

void UBindingsPageWidgetBase::OnResetConfirmed()
{
	UInputSubsystem* InputSubsystem = GetGameInstance()->GetSubsystem<UInputSubsystem>();

	if (!InputSubsystem)
		return;

	InputSubsystem->ResetSavedBindingsToDefault();

	PendingChanges.Empty();
	BindingsList->InitializeRows();
	
	if (ApplyButton)
		ApplyButton->SetVisibility(ESlateVisibility::Hidden);
		
	if (ResetButton)
		ResetButton->SetVisibility(ESlateVisibility::Hidden);
}

void UBindingsPageWidgetBase::UpdateApplyButton()
{
	if (PendingChanges.Num() > 0)
	{
		if (ApplyButton)
		{
			ApplyButton->SetVisibility(ESlateVisibility::Visible);
			// Force CommonUI to rebuild the InputActionWidget from scratch
			// This is necessary because when the button was Hidden, the
			// CommonActionWidget's internal state (CommonInputSubsystem, glyph)
			// was not being updated. SetTriggeringInputAction triggers a full rebuild.
			if (!ApplyInputAction.IsNull())
			{
				ApplyButton->SetTriggeringInputAction(ApplyInputAction);
			}
		}
	}
	else
	{
		if (ApplyButton)
			ApplyButton->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UBindingsPageWidgetBase::DisplayResetToDefaultButton()
{
	if (IsValid(ResetButton))
	{
		ResetButton->SetVisibility(ESlateVisibility::Visible);
		if (!ResetInputAction.IsNull())
		{
			ResetButton->SetTriggeringInputAction(ResetInputAction);
		}
	}
}

void UBindingsPageWidgetBase::HandleRebindRequested(UBindingRowWidget* Row, UInputAction* InputAction, const FKey& CurrentKey, const EInputDeviceType DeviceType)
{
	if (!Row || !PressAnyKeyWidgetClass)
		return;

	ActiveRebindRow = Row;

	const ULocalPlayer* LP = GetOwningLocalPlayer();
	UUISubsystem* UISubsystem = LP ? LP->GetSubsystem<UUISubsystem>() : nullptr;
	if (!UISubsystem)
		return;

	UCommonActivatableWidget* Widget = UISubsystem->PushWidgetToLayer(RebindLayerTag, PressAnyKeyWidgetClass, true);
	UPressAnyKeyWidget* Popup = Cast<UPressAnyKeyWidget>(Widget);
	if (!Popup)
		return;

	Popup->InitializeForDevice(DeviceType);
	Popup->OnKeySelected.AddUniqueDynamic(this, &ThisClass::HandleKeySelected);
	Popup->OnCanceled.AddUniqueDynamic(this, &ThisClass::HandleRebindCanceled);
}

void UBindingsPageWidgetBase::HandleKeySelected(const FKey& NewKey)
{
	if (!ActiveRebindRow)
		return;

	UBindingRowWidget* Row = ActiveRebindRow;
	const FKey OldKey = Row->GetCurrentKey();
	UInputAction* InputAction = Row->GetInputAction();
	const EInputDeviceType DeviceType = Row->GetInputDeviceType();

	// >>> Questo prima lo faceva la riga dentro NativeOnKeyDown, PRIMA di
	//     broadcastare. Ora che la riga non cattura più, lo fa la page: <
	Row->SetReboundKey(NewKey);
	Row->SetGlyphFromKey(NewKey);

	// Da qui in giù è la tua AddToRebindList, identica parola per parola:
	SetPendingChange(Row, InputAction, OldKey, NewKey, DeviceType);
	Row->SetCurrentKey(NewKey);

	for (const auto& Binding : RowWidgets)
	{
		if (Binding != Row && NewKey == Binding->GetReboundKey())
		{
			Binding->SetGlyphFromKey(OldKey);
			Binding->SetReboundKey(OldKey);
			SetPendingChange(Binding, Binding->GetInputAction(), Binding->GetCurrentKey(), OldKey, DeviceType);
			Binding->SetCurrentKey(OldKey);
		}
	}

	UpdateApplyButton();

	ActiveRebindRow = nullptr;
}

void UBindingsPageWidgetBase::HandleRebindCanceled()
{
	ActiveRebindRow = nullptr;
}

void UBindingsPageWidgetBase::SetPendingChange(UBindingRowWidget* Row, UInputAction* InputAction, const FKey& ContextKey, const FKey& NewKey, EInputDeviceType DeviceType)
{
	// If the row already has a pending change, "DefaultBoundKey" must remain
	// the original key already in the mapping context (not the intermediate one).
	const FBindingData* Existing = PendingChanges.Find(Row);
	const FKey OriginalContextKey = Existing ? Existing->DefaultBoundKey : ContextKey;

	if (NewKey == OriginalContextKey)
	{
		PendingChanges.Remove(Row); // back to the original: no change
		return;
	}

	FBindingData Data;
	Data.InputAction = InputAction;
	Data.DefaultBoundKey = OriginalContextKey; // key to remove in the context
	Data.CurrentBoundKey = NewKey;             // new key
	Data.DeviceType = DeviceType;
	PendingChanges.Add(Row, Data);
}
