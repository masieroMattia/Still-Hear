#include "UI/Widgets/SettingsRowBase.h"

#include "Components/TextBlock.h"
#include "CommonInputSubsystem.h"

#pragma region METHODS
void USettingsRowBase::NativeConstruct()
{
	Super::NativeConstruct();

	if (DescriptionRowHandle.DataTable && !DescriptionRowHandle.RowName.IsNone())
	{
		if (const FSettingDescriptionRow* RowData = DescriptionRowHandle.DataTable->FindRow<FSettingDescriptionRow>(DescriptionRowHandle.RowName, TEXT("")))
		{
			CachedDescription = RowData->Description;
			if (RowTitle.IsEmpty())
			{
				SetRowLabel(RowData->Title);
			}
		}
	}

	if (RowDescriptionLabel)
	{
		RowDescriptionLabel->SetText(CachedDescription);
		RowDescriptionLabel->SetVisibility(ESlateVisibility::Hidden);
	}

	if (RowLabel)
	{
		RowLabel->SetVisibility(ESlateVisibility::Visible);
		if (!RowTitle.IsEmpty())
		{
			SetRowLabel(RowTitle);
		}
	}
	
	if (const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		if (UCommonInputSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UCommonInputSubsystem>())
		{
			InputSubsystem->OnInputMethodChangedNative.AddUObject(this, &USettingsRowBase::OnInputMethodChanged);
		}
	}
	
	RefreshFocusVisuals();
}

void USettingsRowBase::SetRowLabel(const FText& InLabel)
{
	RowTitle = InLabel;
    
	if (RowLabel)
		RowLabel->SetText(RowTitle);
}

void USettingsRowBase::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnAddedToFocusPath(InFocusEvent);

	bIsRowFocused = true;
	RefreshFocusVisuals();
}

void USettingsRowBase::NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnRemovedFromFocusPath(InFocusEvent);

	bIsRowFocused = false;
	RefreshFocusVisuals();
}

FNavigationReply USettingsRowBase::NativeOnNavigation(const FGeometry& MyGeometry, const FNavigationEvent& InNavigationEvent, const FNavigationReply& InDefaultReply)
{
	const EUINavigation NavType = InNavigationEvent.GetNavigationType();

	if (NavType == EUINavigation::Left)
	{
		OnStepLeft();
		return FNavigationReply::Stop();
	}

	if (NavType == EUINavigation::Right)
	{
		OnStepRight();
		return FNavigationReply::Stop();
	}

	return Super::NativeOnNavigation(MyGeometry, InNavigationEvent, InDefaultReply);
}

void USettingsRowBase::OnInputMethodChanged(ECommonInputType CurrentInputType)
{
	RefreshFocusVisuals();
}

void USettingsRowBase::NativeOnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	Super::NativeOnMouseEnter(MyGeometry, MouseEvent);
	RefreshFocusVisuals();
}

void USettingsRowBase::NativeOnMouseLeave(const FPointerEvent& MouseEvent)
{
	Super::NativeOnMouseLeave(MouseEvent);
	RefreshFocusVisuals();
}

void USettingsRowBase::RefreshFocusVisuals()
{
	bool bIsGamepad = false;
        
	// Ask the subsystem if we are currently using a Gamepad
	if (const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		if (const UCommonInputSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UCommonInputSubsystem>())
		{
			bIsGamepad = (InputSubsystem->GetCurrentInputType() == ECommonInputType::Gamepad);
		}
	}

	// The indicator is shown if the widget has focus (via gamepad) or if the mouse is hovering over it
	const bool bShouldShow = (bIsRowFocused && bIsGamepad) || IsHovered();
    
	if (FocusIndicator)
	{
		FocusIndicator->SetVisibility(bShouldShow ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Hidden);
	}

	if (RowDescriptionLabel)
	{
		const bool bShowDescription = bShouldShow && !CachedDescription.IsEmpty();
		RowDescriptionLabel->SetVisibility(bShowDescription ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Hidden);
	}

	if (bShouldShow != bIsCurrentlyFocusedOrHovered)
	{
		bIsCurrentlyFocusedOrHovered = bShouldShow;
		OnRowFocusChanged.Broadcast(this, CachedDescription, bIsCurrentlyFocusedOrHovered);
	}
}
#pragma endregion
