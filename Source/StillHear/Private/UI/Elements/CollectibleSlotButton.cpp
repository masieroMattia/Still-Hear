#include "UI/Elements/CollectibleSlotButton.h"

#include "Components/Image.h"
#include "Data/DataTables/CollectibleData.h"

void UCollectibleSlotButton::InitializeSlot(const FCollectibleItemInfo& InItemInfo, const FText& LockedText, const TSoftObjectPtr<UTexture2D> LockedImage, const TSoftObjectPtr<UMaterialInterface> LockedMaterial)
{
	ItemInfo = InItemInfo;

	if (ItemInfo.bIsCollected)
	{
		SetButtonContent(CollectibleRowNameToDisplayText(ItemInfo.RowName), ItemInfo.Image.LoadSynchronous());
		
		if (Icon && !ItemInfo.Material.IsNull())
		{
			Icon->SetBrushFromMaterial(ItemInfo.Material.LoadSynchronous());
		}
	}
	else
	{
		SetButtonContent(LockedText, LockedImage.LoadSynchronous());
		if (Icon && !LockedMaterial.IsNull())
		{
			Icon->SetBrushFromMaterial(LockedMaterial.LoadSynchronous());
		}
	}
}

void UCollectibleSlotButton::SetWheelSelected(bool bInSelected)
{
	bIsWheelSelected = bInSelected;
	UpdateFocusIndicator();
}

void UCollectibleSlotButton::UpdateFocusIndicator()
{
	if (FocusIndicator)
		FocusIndicator->SetVisibility(bIsWheelSelected ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
}

void UCollectibleSlotButton::NativeOnHovered()
{
	// Bypass UButtonBase::NativeOnHovered() to avoid calling SetFocus() on mouse hover
	// This prevents the mouse cursor from hijacking focus during scroll wheel or gamepad navigation
	UCommonButtonBase::NativeOnHovered();

	PlayReversibleAnimation(HoveredAnimation, EUMGSequencePlayMode::Forward);
	UpdateFocusIndicator();
}

FReply UCollectibleSlotButton::NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent)
{
	FReply Reply = Super::NativeOnFocusReceived(InGeometry, InFocusEvent);
	OnSlotButtonHovered.Broadcast(this);
	return Reply;
}

void UCollectibleSlotButton::NativeOnClicked()
{
	Super::NativeOnClicked();
	OnSlotButtonClicked.Broadcast(this);
}