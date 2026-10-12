#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/SettingsPageBase.h"
#include "AudioSettingsWidget.generated.h"

class UGameAudioSubsystem;
class UPopupButtonBase;
class USaveSubsystem;
class USliderBase;

UCLASS()
class STILLHEAR_API UAudioSettingsWidget : public USettingsPageBase
{
	GENERATED_BODY()
	
#pragma region UPROPERTIES
protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USliderBase> MasterVolumeSlider;
    
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USliderBase> MusicVolumeSlider;
    
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USliderBase> SfxVolumeSlider;
    
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USliderBase> AmbienceVolumeSlider;
	
#pragma endregion
    
#pragma region VARIABLES
private:
	UPROPERTY()
	TObjectPtr<UGameAudioSubsystem> AudioManager;
    
	UPROPERTY()
	TObjectPtr<USaveSubsystem> SaveManager;
#pragma endregion
    
#pragma region CONSTRUCTOR
public:
	virtual void NativeConstruct() override;
#pragma endregion
    
#pragma region UFUNCTIONS
public:
	// Asks the Save Subsystem to write the current audio levels to disk
	UFUNCTION(BlueprintCallable, Category = "Settings|Audio")
	void ApplyAudioSettings() const;

	// Resets all audio levels to 1.0 (100%) and updates the visuals
	UFUNCTION(BlueprintCallable, Category = "Settings|Audio")
	void ResetToDefaultAudioSettings() const;
#pragma endregion
	
#pragma region METHODS
protected:
	UFUNCTION()
	void SetMasterVolume(const float Value);
	UFUNCTION()
	void SetMusicVolume(const float Value);
	UFUNCTION()
	void SetSfxVolume(const float Value);
	UFUNCTION()
	void SetAmbienceVolume(const float Value);
	
	void InitializeElements();
	void CheckForChanges() const;
	bool AreSettingsDefault() const;
	void OnSettingsApplied() const;
	void RefreshVisuals() const;

	virtual void OnApplyConfirmed() override;
	virtual void OnResetConfirmed() override;
#pragma endregion
};
