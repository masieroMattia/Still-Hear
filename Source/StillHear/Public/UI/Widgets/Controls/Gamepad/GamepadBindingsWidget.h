// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonInputTypeEnum.h"
#include "UI/Widgets/SettingsPageBase.h"
#include "GamepadBindingsWidget.generated.h"

/**
 * 
 */
UCLASS()
class STILLHEAR_API UGamepadBindingsWidget : public USettingsPageBase
{
	GENERATED_BODY()

#pragma region UPROPERTY
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	TObjectPtr<class UDataTable> GamepadDataTable;
	UPROPERTY(EditDefaultsOnly, Category = "Data")
	TObjectPtr<class UInputMappingContext> DefaultGamepadMappingContext;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> GamepadImage;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class URotatorBase> GamepadPresetRotator;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class URotatorBase> VibrationRotator;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class USliderBase> VibrationIntensitySlider;

	UPROPERTY()
	TObjectPtr<class USaveSubsystem> SaveSubsystem;
	UPROPERTY()
	TObjectPtr<class UCommonInputSubsystem> InputSubsystem;
#pragma endregion

#pragma region VARIABLES
	const FString Context = TEXT("GamepadDataTableLookup");
	int32 CurrentPresetRotatorIndex = 0;
	int32 CurrentVibrationActivatedIndex = 0;
	float CurrentVibrationIntensity = 0.0f;

	// Internal checks to understand if Apply button should be displayed or not
	bool bDifferentPresetRotator = false;
	bool bDifferentActivatedVibration = false;
	bool bDifferentVibrationIntensity = false;
#pragma endregion 

#pragma region UFUNCTIONS
protected:
	UFUNCTION()
	void HandleGamepadRotatorSelectionChanged(int32 SelectedIndex, FText SelectedText);
	UFUNCTION()
	void HandleVibrationRotatorSelectionChanged(int32 SelectedIndex, FText SelectedText);
	UFUNCTION()
	void HandleVibrationIntensityChanged(float Value);
#pragma endregion 
	
#pragma region METHODS
protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnInitialized() override;

	void HandleInputChanged(ECommonInputType InputType);
	virtual void RefreshGamepadImage();
	virtual void DisplayResetToDefaultButton();
	virtual void UpdateApplyButton();

	// Confirmation callbacks from USettingsPageBase
	virtual void OnApplyConfirmed() override;
	virtual void OnResetConfirmed() override;
#pragma endregion 
};
