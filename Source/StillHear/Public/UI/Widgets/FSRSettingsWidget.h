#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "FSRSettingsWidget.generated.h"

class UDropdownBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFSRSettingsChanged);

UCLASS()
class STILLHEAR_API UFSRSettingsWidget : public UCommonUserWidget
{
	GENERATED_BODY()

#pragma region CONSTRUCTOR
public:
	virtual void NativeConstruct() override;
#pragma endregion

#pragma region UPROPERTIES
protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UDropdownBase> SuperResolutionDropdown;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UDropdownBase> FrameGenerationDropdown;
#pragma endregion

#pragma region EVENTS
public:
	UPROPERTY(BlueprintAssignable, Category = "Settings|FSR")
	FOnFSRSettingsChanged OnSettingsChanged;
#pragma endregion
	
#pragma region UFUNCTIONS
public:
	// Call this to apply settings once the user presses the 'Apply' button in the main Graphics Settings
	UFUNCTION(BlueprintCallable, Category = "Settings|FSR")
	void ApplyFSRSettings();

	UFUNCTION(BlueprintPure, Category = "Settings|FSR")
	UWidget* GetFirstFocusableChild() const;

	UFUNCTION(BlueprintPure, Category = "Settings|FSR")
	UWidget* GetLastFocusableChild() const;

protected:
	UFUNCTION()
	void HandleSubSettingChanged(const int32 SelectedIndex, FText SelectedText);
#pragma endregion

#pragma region METHODS
public:
	void SetSavedIndices(int32 SRIndex, int32 FGIndex) const;
	int32 GetSuperResolutionIndex() const;
	int32 GetFrameGenerationIndex() const;

protected:
	void InitializeDropdowns();

private:
	void ApplySuperResolution();
	void ApplyFrameGeneration() const;
#pragma endregion
};
