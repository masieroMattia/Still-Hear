#pragma once

#include "ButtonBase.h"
#include "CoreMinimal.h"
#include "SaveSlotButtonBase.generated.h"

class UImage;
class UTexture2D;

UCLASS(Abstract)
class STILLHEAR_API USaveSlotButtonBase : public UButtonBase
{
	GENERATED_BODY()
	
#pragma region UPROPERTIES
protected:
	// The slot index this button represents
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Slot")
	int32 SlotIndex = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Slot")
	TSoftObjectPtr<UWorld> LevelToOpen;
	
	// Text to display when the slot is empty
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Save Slot")
	FText EmptySlotText = FText::FromString("Empty Slot");

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> DateText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> LevelNameText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> LevelImage;


#pragma endregion

#pragma region METHODS
protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnCurrentTextStyleChanged() override;
	virtual FReply NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent) override;
	virtual void NativeDestruct() override;
	virtual void UpdateFocusIndicator() override;
	virtual void NativeOnHovered() override;
	
private:
	void HandleSlotClicked();
#pragma endregion
	
#pragma region UFUNCTIONS
public:
	UFUNCTION(BlueprintCallable, Category = "Save Slot")
	void RefreshSlotInfo();

	// Populates the button's visuals directly (name / image / label) without reading a disk slot.
	// Used by the dev level-select to drive these buttons from arbitrary data (e.g. a chapter template).
	UFUNCTION(BlueprintCallable, Category = "Save Slot")
	void ApplyDisplay(const FText& Name, const TSoftObjectPtr<UTexture2D>& Image, const FText& InButtonText);

	UFUNCTION(BlueprintPure, Category = "Save Slot")
	int32 GetSlotIndex() const { return SlotIndex; }

	UFUNCTION(BlueprintPure, Category = "Save Slot")
	TSoftObjectPtr<UWorld> GetLevelToOpen() const { return LevelToOpen; }

	UFUNCTION(BlueprintPure, Category = "Save Slot")
	static USaveSlotButtonBase* GetFocusedSlotButton();

	UFUNCTION(BlueprintPure, Category = "Save Slot")
	static int32 GetSelectedSlotIndex();

private:
	static TWeakObjectPtr<USaveSlotButtonBase> FocusedSlotButton;
	static int32 SelectedSlotIndex;
#pragma endregion
};
