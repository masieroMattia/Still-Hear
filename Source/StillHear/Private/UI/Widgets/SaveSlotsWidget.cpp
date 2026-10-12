#include "UI/Widgets/SaveSlotsWidget.h"

#include "Flow/SceneManager.h"
#include "Blueprint/WidgetTree.h"
#include "UI/Elements/ButtonBase.h"
#include "Kismet/GameplayStatics.h"
#include "UI/Subsystem/UISubsystem.h"
#include "SaveSystem/SaveSubsystem.h"
#include "Input/CommonUIInputTypes.h"
#include "UI/Widgets/ConfirmationWidget.h"
#include "UI/Elements/SaveSlotButtonBase.h"

void USaveSlotsWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	if (WidgetTree)
	{
		WidgetTree->ForEachWidget([this](UWidget* Widget) {
			if (USaveSlotButtonBase* SlotButton = Cast<USaveSlotButtonBase>(Widget))
			{
				SlotButton->RefreshSlotInfo();
				SlotButton->OnDoubleClicked().RemoveAll(this);
				SlotButton->OnDoubleClicked().AddUObject(this, &USaveSlotsWidget::HandleSlotDoubleClicked);
			}
		});
	}

	if (DeleteButton)
	{
		DeleteButton->OnClicked().RemoveAll(this);
		DeleteButton->OnClicked().AddUObject(this, &USaveSlotsWidget::HandleDeleteClicked);
		if (!DeleteInputAction.IsNull())
		{
			DeleteButton->SetIsPersistentBinding(true);
			DeleteButton->SetTriggeringInputAction(DeleteInputAction);
		}
	}

	if (OverwriteButton)
	{
		OverwriteButton->OnClicked().RemoveAll(this);
		OverwriteButton->OnClicked().AddUObject(this, &USaveSlotsWidget::HandleOverwriteClicked);
		if (!OverwriteInputAction.IsNull())
		{
			OverwriteButton->SetIsPersistentBinding(true);
			OverwriteButton->SetTriggeringInputAction(OverwriteInputAction);
		}
	}

	if (LoadButton)
	{
		LoadButton->OnClicked().RemoveAll(this);
		LoadButton->OnClicked().AddUObject(this, &USaveSlotsWidget::HandleLoadClicked);
		if (!LoadInputAction.IsNull())
		{
			LoadButton->SetIsPersistentBinding(true);
			LoadButton->SetTriggeringInputAction(LoadInputAction);
		}
	}

	RegisterInputBindings();

	// Direct focus to the active slot (i.e. slot 1 by default, or current slot if valid)
	if (const UGameInstance* GI = GetGameInstance())
	{
		if (const USaveSubsystem* SaveSub = GI->GetSubsystem<USaveSubsystem>())
		{
			int32 TargetSlot = SaveSub->GetCurrentSlotSave();
			if (TargetSlot <= 0)
			{
				TargetSlot = 1; // Default to Slot 1
			}

			if (USaveSlotButtonBase* SlotButton = FindSlotButtonByIndex(TargetSlot))
			{
				SlotButton->SetFocus();
			}
		}
	}
}

void USaveSlotsWidget::NativeOnDeactivated()
{
	UnregisterInputBindings();

	// Background preview removed as it's no longer supported

	Super::NativeOnDeactivated();
}

UWidget* USaveSlotsWidget::NativeGetDesiredFocusTarget() const
{
	if (const UGameInstance* GI = GetGameInstance())
	{
		if (const USaveSubsystem* SaveSub = GI->GetSubsystem<USaveSubsystem>())
		{
			int32 TargetSlot = SaveSub->GetCurrentSlotSave();
			if (TargetSlot <= 0)
			{
				TargetSlot = 1; // Default to Slot 1
			}

			if (USaveSlotButtonBase* SlotButton = FindSlotButtonByIndex(TargetSlot))
			{
				return SlotButton;
			}
		}
	}

	return Super::NativeGetDesiredFocusTarget();
}

void USaveSlotsWidget::HandleDeleteClicked()
{
	PendingDeleteIndex = USaveSlotButtonBase::GetSelectedSlotIndex();
	if (PendingDeleteIndex < 0)
		return;

	const FString TargetSlotName = FString::Printf(TEXT("Save_Slot%d"), PendingDeleteIndex);
	if (UGameplayStatics::DoesSaveGameExist(TargetSlotName, 0))
	{
		if (UConfirmationWidget* ConfirmWidget = ShowConfirmation(DeleteConfirmMessage))
		{
			ConfirmWidget->OnConfirmed.RemoveAll(this);
			ConfirmWidget->OnConfirmed.AddUniqueDynamic(this, &USaveSlotsWidget::OnDeleteConfirmed);
		}
		else
		{
			OnDeleteConfirmed();
		}
	}
}

void USaveSlotsWidget::HandleOverwriteClicked()
{
	PendingOverwriteIndex = USaveSlotButtonBase::GetSelectedSlotIndex();
	if (PendingOverwriteIndex < 0)
		return;

	const FString TargetSlotName = FString::Printf(TEXT("Save_Slot%d"), PendingOverwriteIndex);
	if (UGameplayStatics::DoesSaveGameExist(TargetSlotName, 0))
	{
		if (UConfirmationWidget* ConfirmWidget = ShowConfirmation(OverwriteConfirmMessage))
		{
			ConfirmWidget->OnConfirmed.RemoveAll(this);
			ConfirmWidget->OnConfirmed.AddUniqueDynamic(this, &USaveSlotsWidget::OnOverwriteConfirmed);
		}
		else
		{
			OnOverwriteConfirmed();
		}
	}
	else
	{
		OnOverwriteConfirmed();
	}
}

void USaveSlotsWidget::HandleLoadClicked()
{
	int32 SelectedIndex = USaveSlotButtonBase::GetSelectedSlotIndex();
	if (SelectedIndex < 0)
		return;

	AActor* SceneManagerActor = UGameplayStatics::GetActorOfClass(GetWorld(), ASceneManager::StaticClass());
	if (ASceneManager* SceneManager = Cast<ASceneManager>(SceneManagerActor))
	{
		if (SceneManager->LoadSlotAndTransition(SelectedIndex))
		{
			DeactivateWidget();
		}
	}
}

void USaveSlotsWidget::HandleSlotDoubleClicked()
{
	int32 SelectedIndex = USaveSlotButtonBase::GetSelectedSlotIndex();
	if (SelectedIndex < 0)
		return;

	const FString TargetSlotName = FString::Printf(TEXT("Save_Slot%d"), SelectedIndex);
	if (UGameplayStatics::DoesSaveGameExist(TargetSlotName, 0))
	{
		HandleLoadClicked();
	}
	else
	{
		HandleOverwriteClicked();
	}
}

void USaveSlotsWidget::OnDeleteConfirmed()
{
	if (PendingDeleteIndex < 0)
		return;

	if (UGameInstance* GI = GetGameInstance())
	{
		if (USaveSubsystem* SaveSubsystem = GI->GetSubsystem<USaveSubsystem>())
		{
			if (SaveSubsystem->DeleteSlot(PendingDeleteIndex))
			{
				if (USaveSlotButtonBase* SlotButton = FindSlotButtonByIndex(PendingDeleteIndex))
				{
					SlotButton->RefreshSlotInfo();
				}


			}
		}
	}
	PendingDeleteIndex = -1;
}

void USaveSlotsWidget::OnOverwriteConfirmed()
{
	if (PendingOverwriteIndex < 0)
		return;

	if (UGameInstance* GI = GetGameInstance())
	{
		if (USaveSubsystem* SaveSubsystem = GI->GetSubsystem<USaveSubsystem>())
		{
			(void)SaveSubsystem->DeleteSlot(PendingOverwriteIndex);

			AActor* SceneManagerActor = UGameplayStatics::GetActorOfClass(GetWorld(), ASceneManager::StaticClass());
			if (ASceneManager* SceneManager = Cast<ASceneManager>(SceneManagerActor))
			{
				if (SceneManager->LoadSlotAndTransition(PendingOverwriteIndex))
				{
					DeactivateWidget();
				}
			}
		}
	}
	PendingOverwriteIndex = -1;
}

UConfirmationWidget* USaveSlotsWidget::ShowConfirmation(const FText& Message)
{
	if (!ConfirmationWidgetClass)
	{
		return nullptr;
	}
	if (!ConfirmationLayerTag.IsValid())
	{
		return nullptr;
	}

	const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	if (!LocalPlayer)
	{
		return nullptr;
	}

	UUISubsystem* UISubsystem = LocalPlayer->GetSubsystem<UUISubsystem>();
	if (!UISubsystem)
	{
		return nullptr;
	}

	if (UCommonActivatableWidget* Widget = UISubsystem->PushWidgetToLayer(ConfirmationLayerTag, ConfirmationWidgetClass, false))
	{
		if (UConfirmationWidget* ConfirmWidget = Cast<UConfirmationWidget>(Widget))
		{
			ConfirmWidget->InitializeConfirmation(Message);

			// Unregister input bindings and disable widget to block inputs while popup is open
			UnregisterInputBindings();
			bIsShowingConfirmation = true;
			SetIsEnabled(false);

			// Bind to its deactivated event to restore input bindings
			ConfirmWidget->OnDeactivated().AddWeakLambda(this, [this]() {
				SetIsEnabled(true);
				bIsShowingConfirmation = false;
				RegisterInputBindings();
			});

			return ConfirmWidget;
		}
	}
	
	return nullptr;
}

void USaveSlotsWidget::RegisterInputBindings()
{
	if (!DeleteInputAction.IsNull() && !DeleteBindingHandle.IsValid())
	{
		FBindUIActionArgs BindArgs(DeleteInputAction, FSimpleDelegate::CreateUObject(this, &USaveSlotsWidget::HandleDeleteClicked));
		BindArgs.bDisplayInActionBar = false;
		DeleteBindingHandle = RegisterUIActionBinding(BindArgs);
	}

	if (!OverwriteInputAction.IsNull() && !OverwriteBindingHandle.IsValid())
	{
		FBindUIActionArgs BindArgs(OverwriteInputAction, FSimpleDelegate::CreateUObject(this, &USaveSlotsWidget::HandleOverwriteClicked));
		BindArgs.bDisplayInActionBar = false;
		OverwriteBindingHandle = RegisterUIActionBinding(BindArgs);
	}

	if (!LoadInputAction.IsNull() && !LoadBindingHandle.IsValid())
	{
		FBindUIActionArgs BindArgs(LoadInputAction, FSimpleDelegate::CreateUObject(this, &USaveSlotsWidget::HandleLoadClicked));
		BindArgs.bDisplayInActionBar = false;
		LoadBindingHandle = RegisterUIActionBinding(BindArgs);
	}
}

void USaveSlotsWidget::UnregisterInputBindings()
{
	if (DeleteBindingHandle.IsValid())
	{
		DeleteBindingHandle.Unregister();
	}
	if (OverwriteBindingHandle.IsValid())
	{
		OverwriteBindingHandle.Unregister();
	}
	if (LoadBindingHandle.IsValid())
	{
		LoadBindingHandle.Unregister();
	}
}

USaveSlotButtonBase* USaveSlotsWidget::FindSlotButtonByIndex(int32 SlotIndex) const
{
	if (!WidgetTree)
		return nullptr;

	USaveSlotButtonBase* FoundButton = nullptr;
	WidgetTree->ForEachWidget([&FoundButton, SlotIndex](UWidget* Widget) {
		if (USaveSlotButtonBase* Button = Cast<USaveSlotButtonBase>(Widget))
		{
			if (Button->GetSlotIndex() == SlotIndex)
			{
				FoundButton = Button;
			}
		}
	});
	return FoundButton;
}