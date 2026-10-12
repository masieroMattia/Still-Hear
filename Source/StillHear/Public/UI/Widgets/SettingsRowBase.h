#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Engine/DataTable.h"
#include "Data/DataTables/SettingDescriptionRow.h"
#include "SettingsRowBase.generated.h"

enum class ECommonInputType : uint8;
class UTextBlock;
class USettingsRowBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSettingsRowFocusedChanged, USettingsRowBase*, Row, const FText&, Description, bool, bIsFocused);

UCLASS(Abstract, Blueprintable)
class STILLHEAR_API USettingsRowBase : public UCommonUserWidget
{
	GENERATED_BODY()
	
#pragma region EVENTS
public:
	UPROPERTY(BlueprintAssignable, Category = "Settings Row")
	FOnSettingsRowFocusedChanged OnRowFocusChanged;
#pragma endregion
	
#pragma region UPROPERTIES
protected:
	// The default text to display for this row
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings Row")
	FText RowTitle;

	// Optional text block for the row's label
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RowLabel;
	
	// The line/background that highlights the entire row
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UWidget> FocusIndicator;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings Row")
	FDataTableRowHandle DescriptionRowHandle;

	// Optional text block for the row's description
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RowDescriptionLabel;
#pragma endregion

#pragma region VARIABLES
protected:
	bool bIsRowFocused = false;

private:
	bool bIsCurrentlyFocusedOrHovered = false;
	FText CachedDescription;
#pragma endregion
	
#pragma region METHODS
public:
	// Dynamically update the row's label at runtime
	UFUNCTION(BlueprintCallable, Category = "Settings Row")
	void SetRowLabel(const FText& InLabel);
	
protected:
	virtual void NativeConstruct() override;
    
	// Focus visuals
	virtual void NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) override;
	virtual void NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) override;
	virtual void NativeOnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& MouseEvent) override;
	virtual void OnInputMethodChanged(ECommonInputType CurrentInputType);
    
	// Intercepts gamepad
	virtual FNavigationReply NativeOnNavigation(const FGeometry& MyGeometry, const FNavigationEvent& InNavigationEvent, const FNavigationReply& InDefaultReply) override;

	// Virtual functions meant to be overridden by child classes
	virtual void OnStepLeft() {}
	virtual void OnStepRight() {}
	
	virtual void RefreshFocusVisuals();
#pragma endregion
};
