#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "DropdownMenu.generated.h"

class UVerticalBox;
class UButtonBase;
class UDropdownBase;

UCLASS(Abstract, Blueprintable)
class STILLHEAR_API UDropdownMenu : public UCommonUserWidget
{
	GENERATED_BODY()

protected:
#pragma region UPROPERTIES
	// The Vertical Box that holds option buttons
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UVerticalBox> OptionsList;

	// The button template class to instantiate for each option
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dropdown Menu")
	TSubclassOf<UButtonBase> OptionButtonClass;

	// Reference to the parent setting row
	UPROPERTY(BlueprintReadOnly, Category = "Dropdown Menu")
	TObjectPtr<UDropdownBase> ParentDropdown;
#pragma endregion

#pragma region VARIABLES
private:
	TArray<FText> MenuOptions;
	int32 SelectedIndex = 0;
	bool bBackgroundHidden = false;
	float InitializationTime = 0.0f;
	int32 FocusedOptionIndex = 0;
	FUIActionBindingHandle BackBindingHandle;
#pragma endregion

#pragma region METHODS
public:
	// Populates options and sets initial active focus
	void InitializeMenu(UDropdownBase* InParentDropdown, const TArray<FText>& InOptions, int32 InCurrentIndex);

	// Moves selection by Delta (-1 = up, +1 = down), called by DropdownBase when the popup can't receive navigation directly
	void NavigateSelection(int32 Delta);

	// Confirms the currently navigated option — call from DropdownBase when Accept is pressed
	void ConfirmCurrentSelection() const;
	
protected:
	virtual void NativeConstruct() override;

	// Handle option selection click
	UFUNCTION()
	void HandleOptionClicked(int32 OptionIndex) const;

	UFUNCTION()
	void HandleBackAction() const;

	// KeyDown handling to support Escape / Back to cancel/close the dropdown
	virtual FReply NativeOnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnPreviewKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual void NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
#pragma endregion
};