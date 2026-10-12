#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/SettingsPageBase.h"
#include "GraphicsSettingsWidget.generated.h"

class UCustomQualitySettingsWidget;
class UDLSSSettingsWidget;
class UFSRSettingsWidget;
class UPopupButtonBase;
class USaveSubsystem;
class UDropdownBase;
class USliderBase;

UCLASS()
class STILLHEAR_API UGraphicsSettingsWidget : public USettingsPageBase
{
	GENERATED_BODY()

#pragma region UPROPERTIES
protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UDropdownBase> QualityDropdown;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UDropdownBase> ResolutionDropdown;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UDropdownBase> WindowModeDropdown;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USliderBase> GammaSlider;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UDropdownBase> UpscalerDropdown;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UDropdownBase> LanguageDropdown;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UDLSSSettingsWidget> DLSSSettingsContainer;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UFSRSettingsWidget> FSRSettingsContainer;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UCustomQualitySettingsWidget> CustomQualitySettingsContainer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bAllowCustomQuality = true;

#pragma endregion

#pragma region VARIABLES
private:
	UPROPERTY()
	TObjectPtr<UGameUserSettings> UserSettings;
	UPROPERTY()
	TObjectPtr<USaveSubsystem> SaveManager;
	UPROPERTY()
	TArray<FIntPoint> SupportedResolutions;

	float CurrentGamma = 2.2f;

	// Upscaler cache to map index to meaning
	TArray<FString> AvailableUpscalers;

	TArray<FString> AvailableLanguages;
#pragma endregion

#pragma region CONSTRUCTOR

protected:
	virtual void NativeConstruct() override;
	virtual void OnApplyConfirmed() override;
	virtual void OnResetConfirmed() override;
#pragma endregion

#pragma region UFUNCTIONS

public:
	// Apply and save all current settings
	UFUNCTION(BlueprintCallable, Category = "Settings|Graphics")
	void ApplyGraphicsSettings();

	// Gamma correction
	UFUNCTION(BlueprintCallable, Category = "Settings|Graphics")
	void SetGamma(const float Value);

	// Reset all settings to default/hardware recommended values
	UFUNCTION(BlueprintCallable, Category = "Settings|Graphics")
	void ResetToDefaultGraphicsSettings();
#pragma endregion

#pragma region METHODS

protected:
	UFUNCTION()
	void HandleQualityChanged(const int32 SelectedIndex, FText SelectedText);
	UFUNCTION()
	void HandleResolutionChanged(const int32 SelectedIndex, FText SelectedText);
	UFUNCTION()
	void HandleWindowModeChanged(const int32 SelectedIndex, FText SelectedText);
	UFUNCTION()
	void HandleUpscalerChanged(const int32 SelectedIndex, FText SelectedText);
	UFUNCTION()
	void HandleLanguageChanged(const int32 SelectedIndex, FText SelectedText);
	UFUNCTION()
	void OnSubSettingsChanged();

	void InitializeQualityDropdown();
	void InitializeResolutionDropdown();
	void InitializeWindowModeDropdown();
	void InitializeGammaSlider();
	void InitializeUpscalerDropdown();
	void InitializeLanguageDropdown();

	// Sends the "gamma" console command, actually changing the displayed gamma
	void ApplyGammaToEngine(float Value) const;

	UFUNCTION()
	void CheckForChanges();
	void RefreshVisuals();
#pragma endregion
};