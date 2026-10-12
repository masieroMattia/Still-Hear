#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/ActivatableWidgetBase.h"
#include "CollectiblesWidget.generated.h"

class UCollectibleSlotButton;
class UMaterialInterface;
class UCommonTextBlock;
class UButtonBase;
class UTexture2D;
class UDataTable;
class UScrollBox;
class USpacer;
class UImage;

USTRUCT(BlueprintType)
struct FCollectibleItemInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Collectibles")
	FName RowName;

	UPROPERTY(BlueprintReadOnly, Category = "Collectibles")
	FText DescriptionText;

	UPROPERTY(BlueprintReadOnly, Category = "Collectibles")
	TSoftObjectPtr<UTexture2D> Image;

	UPROPERTY(BlueprintReadOnly, Category = "Collectibles")
	TSoftObjectPtr<UMaterialInterface> Material;

	UPROPERTY(BlueprintReadOnly, Category = "Collectibles")
	bool bIsCollected = false;
};

/**
 * C++ base class for the Collectibles menu
 * Retrieves all items from a collectibles DataTable, maps them to their
 * collected status from the SaveSubsystem, instantiates their buttons,
 * and handles detail updates on selection with a dynamic 3D wheel scrolling effect
 */
UCLASS(Abstract, Blueprintable)
class STILLHEAR_API UCollectiblesWidget : public UActivatableWidgetBase
{
	GENERATED_BODY()

public:
	UCollectibleSlotButton* GetSelectedSlotButton() const { return SelectedSlotButton; }

#pragma region UPROPERTIES
protected:
	// Master DataTable containing all collectible data rows
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectibles")
	TObjectPtr<UDataTable> CollectiblesDataTable;

	// The slot button class used to instantiate the grid buttons
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectibles")
	TSubclassOf<UCollectibleSlotButton> CollectibleSlotButtonClass;

	// Placeholder/Locked Text (e.g., "???")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectibles")
	FText LockedItemText = FText::FromString("???");

	// Placeholder image for locked items
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectibles")
	TSoftObjectPtr<UTexture2D> LockedItemImage;

	// Placeholder material for locked items
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectibles")
	TSoftObjectPtr<UMaterialInterface> LockedItemMaterial;

	// Horizontal offset to push items in the center to create a wheel/curved path
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectibles|WheelSettings")
	float MaxCurveOffset = 50.f;

	// Scale factor for the selected/centered item
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectibles|WheelSettings")
	float CenterItemScale = 1.25f;

	// Scale factor for off-center items at the edges
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectibles|WheelSettings")
	float EdgeItemScale = 0.75f;

	// Minimum opacity for items at the edges of the list
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectibles|WheelSettings")
	float EdgeItemOpacity = 0.4f;

	// Controls the vertical area of influence of the wheel effect around the center (1.0 = half scrollbox height)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectibles|WheelSettings")
	float EffectRadiusScale = 1.0f;

	// Exponent for curve interpolation (higher values make the curve sharper)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectibles|WheelSettings")
	float CurveExponent = 1.0f;

	// Exponent for scale interpolation (higher values make the scaling drop-off sharper)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectibles|WheelSettings")
	float ScaleExponent = 1.0f;

	// Exponent for opacity interpolation (higher values make the fade drop-off sharper)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectibles|WheelSettings")
	float OpacityExponent = 1.0f;

	// Container scroll box where collectible slots will be placed dynamically
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Collectibles|UI")
	TObjectPtr<UScrollBox> CollectiblesGrid;

	// Text block showing the hovered/focused collectible's name in details
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Collectibles|UI")
	TObjectPtr<UCommonTextBlock> DetailNameText;

	// Text block showing the hovered/focused collectible's description in details
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Collectibles|UI")
	TObjectPtr<UCommonTextBlock> DetailDescriptionText;

	// Image showing the hovered/focused collectible's picture/material in details
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Collectibles|UI")
	TObjectPtr<UImage> DetailImage;

	// Button to reset all collected collectibles globally
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Collectibles|UI")
	TObjectPtr<UButtonBase> ResetButton;

	// Input Action for the Reset Button
	UPROPERTY(EditDefaultsOnly, Category = "Collectibles|Input")
	FDataTableRowHandle ResetInputAction;
#pragma endregion

#pragma region METHODS
protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual void NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) override;
	virtual void NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;
	virtual FReply NativeOnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
#pragma endregion

#pragma region UFUNCTIONS
protected:
	// Event triggered in Blueprint once the collectibles list is fully evaluated
	UFUNCTION(BlueprintImplementableEvent, Category = "Collectibles")
	void BP_OnCollectiblesInitialized(const TArray<FCollectibleItemInfo>& Items);

	// Call this from Blueprint when this tab becomes visible to restore gamepad focus
	UFUNCTION(BlueprintCallable, Category = "Collectibles")
	void RestoreFocus();

	virtual UWidget* GetPreferredFocusTarget_Implementation() const override;

private:
	void InitializeCollectiblesGrid();

	UFUNCTION()
	void HandleSlotButtonHovered(UCollectibleSlotButton* SlotButton);

	UFUNCTION()
	void HandleSlotButtonClicked(UCollectibleSlotButton* SlotButton);

	UFUNCTION()
	void HandleUserScrolled(float CurrentOffset);

	UFUNCTION()
	void HandleResetClicked();

	UCollectibleSlotButton* FindClosestButtonToCenter() const;

	// Updates the focus indicator on all buttons based on SelectedSlotButton
	void RefreshAllFocusIndicators();

	UPROPERTY()
	TObjectPtr<USpacer> TopSpacer;

	UPROPERTY()
	TObjectPtr<USpacer> BottomSpacer;

	UPROPERTY()
	TObjectPtr<UCollectibleSlotButton> SelectedSlotButton;

	bool bDisableAutoCentering = false;
	bool bNeedsFocusRestore = false;

	FUIActionBindingHandle ResetBindingHandle;
#pragma endregion
};
