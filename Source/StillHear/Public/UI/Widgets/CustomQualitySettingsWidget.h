#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "CustomQualitySettingsWidget.generated.h"

class UDropdownBase;
class UGameUserSettings;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCustomQualitySettingsChanged);

UCLASS()
class STILLHEAR_API UCustomQualitySettingsWidget : public UCommonUserWidget
{
	GENERATED_BODY()

#pragma region CONSTRUCTOR
public:
	virtual void NativeConstruct() override;
#pragma endregion

#pragma region UPROPERTIES
protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UDropdownBase> ViewDistanceDropdown;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UDropdownBase> AntiAliasingDropdown;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UDropdownBase> ShadowQualityDropdown;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UDropdownBase> GlobalIlluminationDropdown;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UDropdownBase> ReflectionQualityDropdown;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UDropdownBase> TextureQualityDropdown;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UDropdownBase> EffectsQualityDropdown;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UDropdownBase> PostProcessingDropdown;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UDropdownBase> FoliageQualityDropdown;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UDropdownBase> ShadingQualityDropdown;
#pragma endregion

#pragma region EVENTS
public:
	UPROPERTY(BlueprintAssignable, Category = "Settings|Quality")
	FOnCustomQualitySettingsChanged OnSettingsChanged;
#pragma endregion

#pragma region UFUNCTIONS
public:
	UFUNCTION(BlueprintPure, Category = "Settings|Quality")
	UWidget* GetFirstFocusableChild() const;

	UFUNCTION(BlueprintPure, Category = "Settings|Quality")
	UWidget* GetLastFocusableChild() const;

protected:
	UFUNCTION()
	void HandleSubSettingChanged(const int32 SelectedIndex, FText SelectedText);
#pragma endregion

#pragma region METHODS
public:
	void ApplyCustomQualitySettings(UGameUserSettings* UserSettings);
	void RefreshVisuals(const UGameUserSettings* UserSettings);
	bool GetIsDirty(const UGameUserSettings* UserSettings) const;

protected:
	void InitializeDropdowns();
#pragma endregion
};
