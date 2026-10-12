#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "GameplayTagContainer.h"
#include "SettingsPageBase.generated.h"

class UButtonBase;
class UConfirmationWidget;
class UScrollBox;
class USettingsRowBase;

UCLASS(Abstract)
class STILLHEAR_API USettingsPageBase : public UCommonUserWidget
{
	GENERATED_BODY()

#pragma region UPROPERTIES
protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButtonBase> ApplyButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButtonBase> ResetButton;

	UPROPERTY(EditAnywhere, Category = "Settings|Input")
	FDataTableRowHandle ApplyInputAction;

	UPROPERTY(EditAnywhere, Category = "Settings|Input")
	FDataTableRowHandle ResetInputAction;

	// The confirmation widget class to spawn
	UPROPERTY(EditAnywhere, Category = "Settings|Confirmation")
	TSubclassOf<UConfirmationWidget> ConfirmationWidgetClass;

	UPROPERTY(EditAnywhere, meta = (Categories = "UI.Layer"), Category = "Settings|Confirmation")
	FGameplayTag ConfirmationLayerTag;

	UPROPERTY(EditAnywhere, Category = "Settings|Confirmation")
	FText ApplyConfirmationMessage;

	UPROPERTY(EditAnywhere, Category = "Settings|Confirmation")
	FText ResetConfirmationMessage;
#pragma endregion

#pragma region VARIABLES
private:
	FUIActionBindingHandle ApplyBindingHandle;
	FUIActionBindingHandle ResetBindingHandle;
	TWeakObjectPtr<UScrollBox> WeakScrollBox;
#pragma endregion

#pragma region UFUNCTIONS
private:
	UFUNCTION()
	void HandleApplyClicked();
	UFUNCTION()
	void HandleResetClicked();
	UFUNCTION()
	void HandleApplyConfirmed();
	UFUNCTION()
	void HandleResetConfirmed();
	UFUNCTION()
	void HandleRowFocusChanged(USettingsRowBase* Row, const FText& Description, bool bIsFocused);
#pragma endregion

#pragma region METHODS
private:
	void HandleApplyAction();
	void HandleResetAction();
	void ShowApplyConfirmation();
	void ShowResetConfirmation();
	void FindSettingsRowsRecursive(UWidget* RootWidget, TArray<USettingsRowBase*>& OutRows);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) override;
	virtual void NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) override;

	// Overridable callbacks for confirmation results
	virtual void OnApplyConfirmed() {}
	virtual void OnResetConfirmed() {}
#pragma endregion
};
