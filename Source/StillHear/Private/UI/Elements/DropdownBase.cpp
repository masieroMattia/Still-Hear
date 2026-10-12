#include "UI/Elements/DropdownBase.h"

#include "TimerManager.h"
#include "CommonUITypes.h"
#include "CommonInputSubsystem.h"
#include "Components/MenuAnchor.h"
#include "UI/Elements/ButtonBase.h"
#include "CommonActivatableWidget.h"
#include "UI/Elements/DropdownMenu.h"
#include "Input/CommonUIInputTypes.h"
#include "Animation/WidgetAnimation.h"

void UDropdownBase::NativeConstruct()
{
	Super::NativeConstruct();

	if (DropdownButton)
	{
		DropdownButton->OnClicked().RemoveAll(this);
		DropdownButton->OnClicked().AddUObject(this, &UDropdownBase::OpenDropdown);
	}

	if (DropdownMenuAnchor)
	{
		DropdownMenuAnchor->OnGetUserMenuContentEvent.BindDynamic(this, &UDropdownBase::HandleGetUserMenuContent);
		DropdownMenuAnchor->OnMenuOpenChanged.RemoveAll(this);
		DropdownMenuAnchor->OnMenuOpenChanged.AddUniqueDynamic(this, &UDropdownBase::HandleMenuOpenChanged);
	}

	if (const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		if (UCommonInputSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UCommonInputSubsystem>())
		{
			InputSubsystem->OnInputMethodChangedNative.RemoveAll(this);
			InputSubsystem->OnInputMethodChangedNative.AddUObject(this, &UDropdownBase::OnInputMethodChanged);
			OnInputMethodChanged(InputSubsystem->GetCurrentInputType());
		}
	}

	UpdateVisuals();
}

void UDropdownBase::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnAddedToFocusPath(InFocusEvent);

	if (!ForwardInputAction.IsNull())
	{
		FBindUIActionArgs BindArgs(ForwardInputAction, FSimpleDelegate::CreateUObject(this, &UDropdownBase::OpenDropdown));
		BindArgs.bDisplayInActionBar = false;
		ForwardBindingHandle = RegisterUIActionBinding(BindArgs);
	}
}

void UDropdownBase::NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnRemovedFromFocusPath(InFocusEvent);

	if (ForwardBindingHandle.IsValid())
	{
		ForwardBindingHandle.Unregister();
	}
}

void UDropdownBase::SetOptions(const TArray<FText>& InOptions)
{
	Options = InOptions;
	CurrentIndex = FMath::Clamp(CurrentIndex, 0, FMath::Max(0, Options.Num() - 1));
	UpdateVisuals();
}

void UDropdownBase::SetSelectedIndex(const int32 Index)
{
	if (Options.IsEmpty())
		return;

	CurrentIndex = FMath::Clamp(Index, 0, Options.Num() - 1);
	UpdateVisuals();

	OnSelectionChanged.Broadcast(CurrentIndex, Options[CurrentIndex]);
}

void UDropdownBase::OpenDropdown()
{
	if (!DropdownMenuAnchor)
		return;

	if (DropdownMenuAnchor->IsOpen())
	{
		CloseDropdown();
		return;
	}

	DropdownMenuAnchor->Open(true);
}

UUserWidget* UDropdownBase::HandleGetUserMenuContent()
{
	if (!MenuClass)
		return nullptr;

	APlayerController* PC = GetOwningPlayer();
	if (!PC && GetWorld())
		PC = GetWorld()->GetFirstPlayerController();

	OpenMenuInstance = CreateWidget<UDropdownMenu>(PC, MenuClass);
	if (OpenMenuInstance)
		OpenMenuInstance->InitializeMenu(this, Options, CurrentIndex);

	return OpenMenuInstance;
}

void UDropdownBase::CloseDropdown()
{
	if (bIsClosing)
		return;

	if (DropdownMenuAnchor && DropdownMenuAnchor->IsOpen())
	{
		bIsClosing = true;
		DropdownMenuAnchor->Close();

		if (GetWorld())
		{
			GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				if (DropdownButton && DropdownButton->IsFocusable())
				{
					DropdownButton->SetFocus();
				}
				else
				{
					SetFocus();
				}

				if (UCommonActivatableWidget* Activatable = GetTypedOuter<UCommonActivatableWidget>())
				{
					if (!Activatable->HasFocusedDescendants())
					{
						Activatable->SetFocus();
					}
				}
			}));
		}

		bIsClosing = false;
	}
}

FReply UDropdownBase::NativeOnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey PressedKey = InKeyEvent.GetKey();

	if (DropdownMenuAnchor && DropdownMenuAnchor->IsOpen())
	{
		if (IsForwardKey(PressedKey) && OpenMenuInstance)
		{
			OpenMenuInstance->ConfirmCurrentSelection();
			return FReply::Handled();
		}
		return FReply::Unhandled();
	}

	if (ForwardInputAction.IsNull() && IsForwardKey(PressedKey))
	{
		OpenDropdown();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(MyGeometry, InKeyEvent);
}

FNavigationReply UDropdownBase::NativeOnNavigation(const FGeometry& MyGeometry, const FNavigationEvent& InNavigationEvent, const FNavigationReply& InDefaultReply)
{
	if (DropdownMenuAnchor && DropdownMenuAnchor->IsOpen())
	{
		// CommonUI in Menu mode redirects focus away from the MenuAnchor popup, so
		// navigation and key events never reach the option buttons directly. Handle them here
		const EUINavigation NavType = InNavigationEvent.GetNavigationType();
		if (OpenMenuInstance)
		{
			if (NavType == EUINavigation::Up || NavType == EUINavigation::Down)
				OpenMenuInstance->NavigateSelection(NavType == EUINavigation::Down ? 1 : -1);
		}
		return FNavigationReply::Stop();
	}

	return Super::NativeOnNavigation(MyGeometry, InNavigationEvent, InDefaultReply);
}

bool UDropdownBase::IsForwardKey(const FKey& Key) const
{
	if (!ForwardInputAction.IsNull())
	{
		if (const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
		{
			if (const UCommonInputSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UCommonInputSubsystem>())
			{
				if (const FCommonInputActionDataBase* ActionData = CommonUI::GetInputActionData(ForwardInputAction))
				{
					if (ActionData->IsKeyBoundToInputActionData(Key, InputSubsystem))
					{
						return true;
					}
				}
			}
		}
	}
	
	return Key == EKeys::Enter || Key == EKeys::SpaceBar || Key == EKeys::Gamepad_FaceButton_Bottom;
}

bool UDropdownBase::IsBackKey(const FKey& Key) const
{
	if (!BackInputAction.IsNull())
	{
		if (const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
		{
			if (const UCommonInputSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UCommonInputSubsystem>())
			{
				if (const FCommonInputActionDataBase* ActionData = CommonUI::GetInputActionData(BackInputAction))
				{
					if (ActionData->IsKeyBoundToInputActionData(Key, InputSubsystem))
					{
						return true;
					}
				}
			}
		}
	}

	return Key == EKeys::Escape || Key == EKeys::Gamepad_FaceButton_Right;
}

void UDropdownBase::UpdateVisuals()
{
	if (Options.IsEmpty() || !Options.IsValidIndex(CurrentIndex))
		return;

	FText CurrentText = Options[CurrentIndex];

	if (DropdownButton)
	{
		DropdownButton->SetButtonContent(CurrentText, nullptr);
	}
}

void UDropdownBase::HandleMenuOpenChanged(bool bIsOpen)
{
	if (bIsOpen)
	{
		if (ForwardBindingHandle.IsValid())
			ForwardBindingHandle.Unregister();
	}
	else
	{
		OpenMenuInstance = nullptr;
		if (bIsRowFocused && !ForwardBindingHandle.IsValid() && !ForwardInputAction.IsNull())
		{
			FBindUIActionArgs BindArgs(ForwardInputAction, FSimpleDelegate::CreateUObject(this, &UDropdownBase::OpenDropdown));
			BindArgs.bDisplayInActionBar = false;
			ForwardBindingHandle = RegisterUIActionBinding(BindArgs);
		}
	}
}

void UDropdownBase::RefreshFocusVisuals()
{
	Super::RefreshFocusVisuals();

	bool bIsGamepad = false;
	if (const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		if (const UCommonInputSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UCommonInputSubsystem>())
		{
			bIsGamepad = (InputSubsystem->GetCurrentInputType() == ECommonInputType::Gamepad);
		}
	}

	const bool bShouldBeFocused = (bIsRowFocused && bIsGamepad) || IsHovered();

	if (bShouldBeFocused && !bIsFocusAnimationActive)
	{
		PlayReversibleAnimation(FocusAnimation.Get(), EUMGSequencePlayMode::Forward);
		bIsFocusAnimationActive = true;
	}
	else if (!bShouldBeFocused && bIsFocusAnimationActive)
	{
		PlayReversibleAnimation(FocusAnimation.Get(), EUMGSequencePlayMode::Reverse);
		bIsFocusAnimationActive = false;
	}
}

void UDropdownBase::OnInputMethodChanged(ECommonInputType CurrentInputType)
{
	Super::OnInputMethodChanged(CurrentInputType);
}

void UDropdownBase::PlayReversibleAnimation(UWidgetAnimation* InAnimation, EUMGSequencePlayMode::Type PlayMode)
{
	if (!InAnimation)
		return;

	float StartTime = 0.0f;

	if (IsAnimationPlaying(InAnimation))
	{
		StartTime = GetAnimationCurrentTime(InAnimation);
	}
	else if (PlayMode == EUMGSequencePlayMode::Reverse)
	{
		StartTime = InAnimation->GetEndTime();
	}

	StopAnimation(InAnimation);
	PlayAnimation(InAnimation, StartTime, 1, PlayMode);
}