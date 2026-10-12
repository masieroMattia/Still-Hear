#pragma once

#include "CoreMinimal.h"
#include "BindingRowWidget.h"
#include "InputAction.h"
#include "UI/Widgets/SettingsPageBase.h"
#include "Input/BindingData.h"
#include "BindingsPageWidgetBase.generated.h"

enum class EInputDeviceType : uint8;
class UPressAnyKeyWidget;

UCLASS()
class STILLHEAR_API UBindingsPageWidgetBase : public USettingsPageBase
{
	GENERATED_BODY()

#pragma region UPROPERTY
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Rebinding")
	TSubclassOf<UPressAnyKeyWidget> PressAnyKeyWidgetClass;

	UPROPERTY(EditDefaultsOnly, meta = (Categories = "UI.Layer"), Category = "Rebinding")
	FGameplayTag RebindLayerTag;

	UPROPERTY(Transient)
	TObjectPtr<UBindingRowWidget> ActiveRebindRow;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UBindingsListWidget> BindingsList;
	UPROPERTY()
	TArray<UBindingRowWidget*> RowWidgets;
	UPROPERTY()
	TMap<TObjectPtr<UBindingRowWidget>, FBindingData> PendingChanges;
#pragma endregion
	
#pragma region UFUNCTION	
private:
	UFUNCTION()
	void DisplayResetToDefaultButton();

	UFUNCTION()
	void HandleRebindRequested(UBindingRowWidget* Row, UInputAction* InputAction, const FKey& CurrentKey, const EInputDeviceType DeviceType);

	UFUNCTION()
	void HandleKeySelected(const FKey& NewKey);

	UFUNCTION()
	void HandleRebindCanceled();
#pragma endregion

#pragma region METHODS	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnInitialized() override;
	
	// Confirmation callbacks from USettingsPageBase
	virtual void OnApplyConfirmed() override;
	virtual void OnResetConfirmed() override;
	
private:
	void UpdateApplyButton();
	void SetPendingChange(UBindingRowWidget* Row, UInputAction* InputAction, const FKey& ContextKey, const FKey& NewKey, EInputDeviceType DeviceType);
#pragma endregion 
};
