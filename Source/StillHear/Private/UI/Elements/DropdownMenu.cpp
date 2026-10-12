#include "UI/Elements/DropdownMenu.h"

#include "TimerManager.h"
#include "Layout/Visibility.h"
#include "Widgets/Layout/SBorder.h"
#include "Components/VerticalBox.h"
#include "UI/Elements/ButtonBase.h"
#include "UI/Elements/DropdownBase.h"
#include "Input/CommonUIInputTypes.h"
#include "Components/VerticalBoxSlot.h"

void UDropdownMenu::NativeConstruct()
{
    Super::NativeConstruct();

    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimerForNextTick(
            FTimerDelegate::CreateWeakLambda(this, [this]()
            {
                if (!OptionsList) 
                	return;
            	
                UWidget* Target = (OptionsList->GetChildrenCount() > SelectedIndex && SelectedIndex >= 0) ? OptionsList->GetChildAt(SelectedIndex) : (OptionsList->GetChildrenCount() > 0 ? OptionsList->GetChildAt(0) : nullptr);
            	
                if (Target) 
                	Target->SetFocus();
            }));
    }
}

void UDropdownMenu::InitializeMenu(UDropdownBase* InParentDropdown, const TArray<FText>& InOptions, int32 InCurrentIndex)
{
	ParentDropdown = InParentDropdown;
	MenuOptions = InOptions;
	SelectedIndex = InCurrentIndex;

	if (!OptionsList || !OptionButtonClass)
		return;

	OptionsList->ClearChildren();

	APlayerController* PC = GetOwningPlayer();
	if (!PC && GetWorld())
	{
		PC = GetWorld()->GetFirstPlayerController();
	}

	TArray<UButtonBase*> CreatedButtons;
	for (int32 i = 0; i < MenuOptions.Num(); ++i)
	{
		UButtonBase* OptionButton = CreateWidget<UButtonBase>(PC, OptionButtonClass);
		if (OptionButton)
		{
			OptionButton->SetIsFocusable(true);
			OptionButton->SetButtonContent(MenuOptions[i], nullptr);
			
			if (UVerticalBoxSlot* BoxSlot = OptionsList->AddChildToVerticalBox(OptionButton))
			{
				BoxSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
			}

			OptionButton->OnClicked().AddUObject(this, &UDropdownMenu::HandleOptionClicked, i);
			CreatedButtons.Add(OptionButton);
		}
	}

	const int32 Count = CreatedButtons.Num();
    if (Count > 1)
    {
        for (int32 i = 0; i < Count; ++i)
        {
            UButtonBase* CurrentButton = CreatedButtons[i];
            UButtonBase* PrevButton = CreatedButtons[(i - 1 + Count) % Count];
            UButtonBase* NextButton = CreatedButtons[(i + 1) % Count];

            CurrentButton->SetNavigationRuleExplicit(EUINavigation::Up, PrevButton);
            CurrentButton->SetNavigationRuleExplicit(EUINavigation::Down, NextButton);
            CurrentButton->SetNavigationRuleBase(EUINavigation::Left, EUINavigationRule::Stop);
            CurrentButton->SetNavigationRuleBase(EUINavigation::Right, EUINavigationRule::Stop);
        }
    }

	if (ParentDropdown && !ParentDropdown->GetBackInputAction().IsNull())
	{
		FBindUIActionArgs BindArgs(ParentDropdown->GetBackInputAction(), FSimpleDelegate::CreateUObject(this, &UDropdownMenu::HandleBackAction));
		BindArgs.bDisplayInActionBar = false;
		BindArgs.bConsumeInput = true;
		BindArgs.InputMode = ECommonInputMode::All;
		BackBindingHandle = RegisterUIActionBinding(BindArgs);
	}

	FocusedOptionIndex = InCurrentIndex;

	if (GetWorld())
		InitializationTime = GetWorld()->GetRealTimeSeconds();
}

void UDropdownMenu::NavigateSelection(int32 Delta)
{
	if (!OptionsList || OptionsList->GetChildrenCount() == 0)
		return;

	const int32 Count = OptionsList->GetChildrenCount();
	FocusedOptionIndex = FMath::Clamp(FocusedOptionIndex + Delta, 0, Count - 1);

	if (UWidget* Target = OptionsList->GetChildAt(FocusedOptionIndex))
		Target->SetFocus();
}

void UDropdownMenu::ConfirmCurrentSelection() const
{
	HandleOptionClicked(FocusedOptionIndex);
}

void UDropdownMenu::HandleOptionClicked(const int32 OptionIndex) const
{
	if (GetWorld() && (GetWorld()->GetRealTimeSeconds() - InitializationTime) < 0.2f)
		return;

	if (ParentDropdown)
	{
		ParentDropdown->SetSelectedIndex(OptionIndex);
		ParentDropdown->CloseDropdown();
	}
}

void UDropdownMenu::HandleBackAction() const
{
	if (ParentDropdown)
	{
		ParentDropdown->CloseDropdown();
	}
}

FReply UDropdownMenu::NativeOnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();

	if (ParentDropdown && ParentDropdown->IsBackKey(Key))
	{
		ParentDropdown->CloseDropdown();
		return FReply::Handled();
	}

	if (ParentDropdown && ParentDropdown->IsForwardKey(Key))
	{
		if (OptionsList)
		{
			for (int32 i = 0; i < OptionsList->GetChildrenCount(); ++i)
			{
				if (const UWidget* Child = OptionsList->GetChildAt(i))
				{
					if (Child->HasAnyUserFocus())
					{
						HandleOptionClicked(i);
						return FReply::Handled();
					}
				}
			}
		}
	}

	return Super::NativeOnKeyDown(MyGeometry, InKeyEvent);
}

FReply UDropdownMenu::NativeOnPreviewKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();

	if (ParentDropdown && ParentDropdown->IsBackKey(Key))
	{
		ParentDropdown->CloseDropdown();
		return FReply::Handled();
	}

	if (ParentDropdown && ParentDropdown->IsForwardKey(Key))
	{
		if (OptionsList)
		{
			for (int32 i = 0; i < OptionsList->GetChildrenCount(); ++i)
			{
				if (const UWidget* Child = OptionsList->GetChildAt(i))
				{
					if (Child->HasAnyUserFocus())
					{
						HandleOptionClicked(i);
						return FReply::Handled();
					}
				}
			}
		}
	}

	return Super::NativeOnPreviewKeyDown(MyGeometry, InKeyEvent);
}

void UDropdownMenu::NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnRemovedFromFocusPath(InFocusEvent);

	if (ParentDropdown)
	{
		ParentDropdown->CloseDropdown();
	}
}

void UDropdownMenu::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// UMenuAnchor wraps the popup in an SOverlay that includes a default background widget
	// Walk up the Slate tree to find that SOverlay and collapse every sibling of our content
	// Runs once — bBackgroundHidden prevents re-entry after the first successful pass
	if (!bBackgroundHidden)
	{
		const TSharedPtr<SWidget> SlateWidget = GetCachedWidget();
		if (SlateWidget.IsValid())
		{
			const TSharedPtr<SWidget> ContentBorder = SlateWidget->GetParentWidget();
			if (ContentBorder.IsValid())
			{
				TSharedPtr<SWidget> OverlayParent = ContentBorder->GetParentWidget();
				if (OverlayParent.IsValid() && OverlayParent->GetType() == FName(TEXT("SOverlay")))
				{
					FChildren* OverlayChildren = OverlayParent->GetChildren();
					if (OverlayChildren)
					{
						int32 ContentIndex = INDEX_NONE;
						for (int32 i = 0; i < OverlayChildren->Num(); ++i)
						{
							if (OverlayChildren->GetChildAt(i) == ContentBorder.ToSharedRef())
							{
								ContentIndex = i;
								break;
							}
						}

						if (ContentIndex != INDEX_NONE)
						{
							for (int32 i = 0; i < OverlayChildren->Num(); ++i)
							{
								if (i != ContentIndex)
								{
									OverlayChildren->GetChildAt(i)->SetVisibility(EVisibility::Collapsed);
								}
							}
							bBackgroundHidden = true;
						}
					}
				}
			}
		}
	}
}