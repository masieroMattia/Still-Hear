#pragma once

#include "UI/Elements/ButtonBase.h"
#include "UI/Widgets/CollectiblesWidget.h"
#include "CollectibleSlotButton.generated.h"

class UCollectibleSlotButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCollectibleSlotButtonHovered, UCollectibleSlotButton*, SlotButton);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCollectibleSlotButtonClicked, UCollectibleSlotButton*, SlotButton);

/**
 * C++ button class representing an individual collectible slot in the grid/list.
 */
UCLASS(Abstract, Blueprintable)
class STILLHEAR_API UCollectibleSlotButton : public UButtonBase
{
	GENERATED_BODY()

#pragma region UPROPERTIES
public:
	UPROPERTY(BlueprintAssignable, Category = "Collectibles")
	FOnCollectibleSlotButtonHovered OnSlotButtonHovered;

	UPROPERTY(BlueprintAssignable, Category = "Collectibles")
	FOnCollectibleSlotButtonClicked OnSlotButtonClicked;
#pragma endregion

#pragma region METHODS
public:
	// Initializes slot content based on collected state
	void InitializeSlot(const FCollectibleItemInfo& InItemInfo, const FText& LockedText, TSoftObjectPtr<UTexture2D> LockedImage, TSoftObjectPtr<UMaterialInterface> LockedMaterial);

	FORCEINLINE const FCollectibleItemInfo& GetItemInfo() const { return ItemInfo; }

	// Sets this button as the "wheel selected" item — controls the focus indicator independently of Slate focus
	void SetWheelSelected(bool bInSelected);

	virtual void UpdateFocusIndicator() override;

protected:
	virtual void NativeOnHovered() override;
	virtual FReply NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent) override;
	virtual void NativeOnClicked() override;
#pragma endregion

private:
	FCollectibleItemInfo ItemInfo;
	bool bIsWheelSelected = false;
};
