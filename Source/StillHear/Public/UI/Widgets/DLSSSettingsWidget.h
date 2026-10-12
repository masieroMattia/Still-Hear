#pragma once

#include "DLSSLibrary.h"
#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "StreamlineLibraryDLSSG.h"
#include "StreamlineLibraryReflex.h"
#include "DLSSSettingsWidget.generated.h"

class UDropdownBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDLSSSettingsChanged);

UCLASS()
class STILLHEAR_API UDLSSSettingsWidget : public UCommonUserWidget
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

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UDropdownBase> ReflexDropdown;
#pragma endregion

#pragma region EVENTS
public:
	UPROPERTY(BlueprintAssignable, Category = "Settings|DLSS")
	FOnDLSSSettingsChanged OnSettingsChanged;
#pragma endregion
	
#pragma region UFUNCTIONS
public:
	// Call this to apply settings once the user presses the 'Apply' button in the main Graphics Settings
	UFUNCTION(BlueprintCallable, Category = "Settings|DLSS")
	void ApplyDLSSSettings();
	
	UFUNCTION(BlueprintPure, Category = "Settings|DLSS")
	UWidget* GetFirstFocusableChild() const;

	UFUNCTION(BlueprintPure, Category = "Settings|DLSS")
	UWidget* GetLastFocusableChild() const;
	
protected:
	UFUNCTION()
	void HandleSubSettingChanged(const int32 SelectedIndex, FText SelectedText);
#pragma endregion

#pragma region METHODS
public:
	void SetSavedIndices(int32 SRIndex, int32 FGIndex, int32 RflxIndex) const;
	int32 GetSuperResolutionIndex() const;
	int32 GetFrameGenerationIndex() const;
	int32 GetReflexIndex() const;

protected:
	void InitializeDropdowns();

private:
	void ApplySuperResolution() const;
	void ApplyFrameGeneration() const;
	void ApplyReflex();
#pragma endregion

#pragma region HELPERS
public:
	static UDLSSMode GetDLSSModeFromIndex(int32 Index);
	static EStreamlineDLSSGMode GetDLSSGModeFromIndex(int32 Index);
	static EStreamlineReflexMode GetReflexModeFromIndex(int32 Index);
#pragma endregion
};
