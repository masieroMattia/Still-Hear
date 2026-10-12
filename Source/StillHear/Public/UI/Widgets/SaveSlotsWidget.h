#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/ActivatableWidgetBase.h"
#include "SaveSlotsWidget.generated.h"

class UButtonBase;
class UConfirmationWidget;
class USaveSlotButtonBase;

UCLASS(Abstract, Blueprintable)
class STILLHEAR_API USaveSlotsWidget : public UActivatableWidgetBase
{
	GENERATED_BODY()

#pragma region UPROPERTIES
protected:
	// Widget elements
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButtonBase> DeleteButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButtonBase> OverwriteButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButtonBase> LoadButton;

	// UI and Configuration
	UPROPERTY(EditDefaultsOnly, Category = "Save Slots|UI")
	TSubclassOf<UConfirmationWidget> ConfirmationWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Save Slots|UI", meta = (Categories = "UI.Layer"))
	FGameplayTag ConfirmationLayerTag;

	UPROPERTY(EditDefaultsOnly, Category = "Save Slots|UI")
	FText DeleteConfirmMessage;

	UPROPERTY(EditDefaultsOnly, Category = "Save Slots|UI")
	FText OverwriteConfirmMessage;

	// Input Actions
	UPROPERTY(EditDefaultsOnly, Category = "Save Slots|Input")
	FDataTableRowHandle DeleteInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "Save Slots|Input")
	FDataTableRowHandle OverwriteInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "Save Slots|Input")
	FDataTableRowHandle LoadInputAction;
#pragma endregion

#pragma region VARIABLES
private:
	FUIActionBindingHandle DeleteBindingHandle;
	FUIActionBindingHandle OverwriteBindingHandle;
	FUIActionBindingHandle LoadBindingHandle;
	bool bIsShowingConfirmation = false;
	int32 PendingDeleteIndex = -1;
	int32 PendingOverwriteIndex = -1;
#pragma endregion

#pragma region UFUNCTIONS
private:
	UFUNCTION()
	void HandleDeleteClicked();

	UFUNCTION()
	void HandleOverwriteClicked();

	UFUNCTION()
	void HandleLoadClicked();

	UFUNCTION()
	void HandleSlotDoubleClicked();

	UFUNCTION()
	void OnDeleteConfirmed();

	UFUNCTION()
	void OnOverwriteConfirmed();
#pragma endregion

#pragma region METHODS
protected:
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

private:
	UConfirmationWidget* ShowConfirmation(const FText& Message);
	USaveSlotButtonBase* FindSlotButtonByIndex(int32 SlotIndex) const;
	void RegisterInputBindings();
	void UnregisterInputBindings();
#pragma endregion
};
