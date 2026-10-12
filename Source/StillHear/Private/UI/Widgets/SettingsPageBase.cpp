#include "UI/Widgets/SettingsPageBase.h"

#include "Blueprint/WidgetTree.h"
#include "Components/ScrollBox.h"
#include "UI/Elements/ButtonBase.h"
#include "Components/PanelWidget.h"
#include "Input/CommonUIInputTypes.h"
#include "UI/Subsystem/UISubsystem.h"
#include "UI/Widgets/SettingsRowBase.h"
#include "UI/Widgets/ConfirmationWidget.h"

void USettingsPageBase::NativeConstruct()
{
	Super::NativeConstruct();

	if (ApplyButton)
	{
		ApplyButton->OnClicked().RemoveAll(this);
		ApplyButton->OnClicked().AddUObject(this, &USettingsPageBase::HandleApplyClicked);
		if (!ApplyInputAction.IsNull())
		{
			ApplyButton->SetTriggeringInputAction(ApplyInputAction);
		}
	}

	if (ResetButton)
	{
		ResetButton->OnClicked().RemoveAll(this);
		ResetButton->OnClicked().AddUObject(this, &USettingsPageBase::HandleResetClicked);
		if (!ResetInputAction.IsNull())
		{
			ResetButton->SetTriggeringInputAction(ResetInputAction);
		}
	}
	
	// Auto-discover and bind to all USettingsRowBase widgets on the page recursively
	TArray<UWidget*> FoundRootWidgets;
	WidgetTree->GetAllWidgets(FoundRootWidgets);

	// Find the scroll box on this page
	UScrollBox* ScrollBox = nullptr;
	for (UWidget* Widget : FoundRootWidgets)
	{
		if (UScrollBox* CastScroll = Cast<UScrollBox>(Widget))
		{
			ScrollBox = CastScroll;
			break;
		}
	}
	WeakScrollBox = ScrollBox;

	// Recursive discovery of settings rows (even inside sub-widgets)
	TArray<USettingsRowBase*> AllSettingsRows;
	for (UWidget* RootWidget : FoundRootWidgets)
	{
		FindSettingsRowsRecursive(RootWidget, AllSettingsRows);
	}

	for (USettingsRowBase* Row : AllSettingsRows)
	{
		if (Row)
		{
			Row->OnRowFocusChanged.RemoveAll(this);
			Row->OnRowFocusChanged.AddUniqueDynamic(this, &USettingsPageBase::HandleRowFocusChanged);
		}
	}
}

void USettingsPageBase::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnAddedToFocusPath(InFocusEvent);

	if (!ApplyInputAction.IsNull())
	{
		FBindUIActionArgs BindArgs(ApplyInputAction, FSimpleDelegate::CreateUObject(this, &USettingsPageBase::HandleApplyAction));
		BindArgs.bDisplayInActionBar = false;
		ApplyBindingHandle = RegisterUIActionBinding(BindArgs);
	}

	if (!ResetInputAction.IsNull())
	{
		FBindUIActionArgs BindArgs(ResetInputAction, FSimpleDelegate::CreateUObject(this, &USettingsPageBase::HandleResetAction));
		BindArgs.bDisplayInActionBar = false;
		ResetBindingHandle = RegisterUIActionBinding(BindArgs);
	}
}

void USettingsPageBase::NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnRemovedFromFocusPath(InFocusEvent);

	if (ApplyBindingHandle.IsValid())
	{
		ApplyBindingHandle.Unregister();
	}

	if (ResetBindingHandle.IsValid())
	{
		ResetBindingHandle.Unregister();
	}
}

void USettingsPageBase::HandleApplyAction()
{
	if (ApplyButton && ApplyButton->GetIsEnabled())
	{
		ShowApplyConfirmation();
	}
}

void USettingsPageBase::HandleResetAction()
{
	if (ResetButton && ResetButton->GetIsEnabled())
	{
		ShowResetConfirmation();
	}
}

void USettingsPageBase::HandleApplyClicked()
{
	ShowApplyConfirmation();
}

void USettingsPageBase::HandleResetClicked()
{
	ShowResetConfirmation();
}

void USettingsPageBase::HandleApplyConfirmed()
{
	OnApplyConfirmed();
}

void USettingsPageBase::HandleResetConfirmed()
{
	OnResetConfirmed();
}

void USettingsPageBase::ShowApplyConfirmation()
{
	if (!ConfirmationWidgetClass || !ConfirmationLayerTag.IsValid())
	{
		OnApplyConfirmed();
		return;
	}

	const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	if (!LocalPlayer) 
		return;

	UUISubsystem* UISubsystem = LocalPlayer->GetSubsystem<UUISubsystem>();
	if (!UISubsystem) 
		return;

	UCommonActivatableWidget* Widget = UISubsystem->PushWidgetToLayer(ConfirmationLayerTag, ConfirmationWidgetClass, true);
	UConfirmationWidget* ConfirmWidget = Cast<UConfirmationWidget>(Widget);
	if (ConfirmWidget)
	{
		ConfirmWidget->InitializeConfirmation(ApplyConfirmationMessage);
		ConfirmWidget->OnConfirmed.RemoveAll(this);
		ConfirmWidget->OnConfirmed.AddUniqueDynamic(this, &USettingsPageBase::HandleApplyConfirmed);
	}
}

void USettingsPageBase::ShowResetConfirmation()
{
	if (!ConfirmationWidgetClass || !ConfirmationLayerTag.IsValid())
	{
		OnResetConfirmed();
		return;
	}

	const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	if (!LocalPlayer) return;

	UUISubsystem* UISubsystem = LocalPlayer->GetSubsystem<UUISubsystem>();
	if (!UISubsystem) return;

	UCommonActivatableWidget* Widget = UISubsystem->PushWidgetToLayer(ConfirmationLayerTag, ConfirmationWidgetClass, true);
	UConfirmationWidget* ConfirmWidget = Cast<UConfirmationWidget>(Widget);
	if (ConfirmWidget)
	{
		ConfirmWidget->InitializeConfirmation(ResetConfirmationMessage);
		ConfirmWidget->OnConfirmed.RemoveAll(this);
		ConfirmWidget->OnConfirmed.AddUniqueDynamic(this, &USettingsPageBase::HandleResetConfirmed);
	}
}

void USettingsPageBase::FindSettingsRowsRecursive(UWidget* RootWidget, TArray<USettingsRowBase*>& OutRows)
{
	if (!RootWidget)
		return;

	if (USettingsRowBase* Row = Cast<USettingsRowBase>(RootWidget))
	{
		OutRows.AddUnique(Row);
	}

	if (UUserWidget* UserWidget = Cast<UUserWidget>(RootWidget))
	{
		if (UserWidget->WidgetTree)
		{
			TArray<UWidget*> ChildWidgets;
			UserWidget->WidgetTree->GetAllWidgets(ChildWidgets);
			for (UWidget* Child : ChildWidgets)
			{
				FindSettingsRowsRecursive(Child, OutRows);
			}
		}
	}
	else if (UPanelWidget* PanelWidget = Cast<UPanelWidget>(RootWidget))
	{
		const int32 ChildCount = PanelWidget->GetChildrenCount();
		for (int32 i = 0; i < ChildCount; ++i)
		{
			FindSettingsRowsRecursive(PanelWidget->GetChildAt(i), OutRows);
		}
	}
}

void USettingsPageBase::HandleRowFocusChanged(USettingsRowBase* Row, const FText& Description, const bool bIsFocused)
{
	if (bIsFocused && Row)
	{
		if (UScrollBox* ScrollBox = WeakScrollBox.Get())
		{
			ScrollBox->ScrollWidgetIntoView(Row, true, EDescendantScrollDestination::IntoView, 0.0f);
		}
	}
}