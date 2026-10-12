#include "UI/Widgets/ConfirmationWidget.h"

#include "CommonTextBlock.h"
#include "UI/Elements/ButtonBase.h"
#include "Input/CommonUIInputTypes.h"

#pragma region UFUNCTIONS
void UConfirmationWidget::InitializeConfirmation(const FText& Message)
{
	if (MessageText)
		MessageText->SetText(Message);
}
#pragma endregion

#pragma region METHODS
void UConfirmationWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	if (ConfirmButton)
	{
		ConfirmButton->OnClicked().RemoveAll(this);
		ConfirmButton->OnClicked().AddUObject(this, &UConfirmationWidget::HandleConfirmClicked);
		if (!ConfirmInputAction.IsNull())
			ConfirmButton->SetTriggeringInputAction(ConfirmInputAction);
	}

	if (CancelButton)
	{
		CancelButton->OnClicked().RemoveAll(this);
		CancelButton->OnClicked().AddUObject(this, &UConfirmationWidget::HandleCancelClicked);
		if (!CancelInputAction.IsNull())
			CancelButton->SetTriggeringInputAction(CancelInputAction);
	}

	if (!ConfirmInputAction.IsNull())
	{
		FBindUIActionArgs BindArgs(ConfirmInputAction, FSimpleDelegate::CreateUObject(this, &UConfirmationWidget::ExecuteConfirm));
		BindArgs.bDisplayInActionBar = false;
		ConfirmBindingHandle = RegisterUIActionBinding(BindArgs);
	}

	if (!CancelInputAction.IsNull())
	{
		FBindUIActionArgs BindArgs(CancelInputAction, FSimpleDelegate::CreateUObject(this, &UConfirmationWidget::ExecuteCancel));
		BindArgs.bDisplayInActionBar = false;
		CancelBindingHandle = RegisterUIActionBinding(BindArgs);
	}
}

void UConfirmationWidget::NativeOnDeactivated()
{
	if (ConfirmBindingHandle.IsValid())
		ConfirmBindingHandle.Unregister();

	if (CancelBindingHandle.IsValid())
		CancelBindingHandle.Unregister();

	Super::NativeOnDeactivated();
}

void UConfirmationWidget::HandleConfirmClicked()
{
	ExecuteConfirm();
}

void UConfirmationWidget::HandleCancelClicked()
{
	ExecuteCancel();
}

void UConfirmationWidget::ExecuteConfirm()
{
	OnConfirmed.Broadcast();
	DeactivateWidget();
}

void UConfirmationWidget::ExecuteCancel()
{
	OnCancelled.Broadcast();
	DeactivateWidget();
}
#pragma endregion
