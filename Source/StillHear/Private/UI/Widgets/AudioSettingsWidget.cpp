#include "UI/Widgets/AudioSettingsWidget.h"

#include "UI/Elements/SliderBase.h"
#include "Audio/GameAudioSubsystem.h"
#include "SaveSystem/SaveSubsystem.h"
#include "UI/Elements/PopupButtonBase.h"

#pragma region UFUNCTIONS
void UAudioSettingsWidget::NativeConstruct()
{
   Super::NativeConstruct();

   if (const UGameInstance* GI = GetGameInstance())
   {
      AudioManager = GI->GetSubsystem<UGameAudioSubsystem>();
      SaveManager = GI->GetSubsystem<USaveSubsystem>();
   }

   InitializeElements();
}

void UAudioSettingsWidget::ApplyAudioSettings() const
{
   // The SaveManager handles grabbing the current levels from the AudioManager internally
   if (SaveManager)
      SaveManager->SaveSettingsAsync();
}

void UAudioSettingsWidget::ResetToDefaultAudioSettings() const
{
   if (!AudioManager)
      return;

   // Resetting all levels directly in the engine memory
   AudioManager->SetMasterVolume(1.0f);
   AudioManager->SetMusicVolume(1.0f);
   AudioManager->SetSFXVolume(1.0f);
   AudioManager->SetAmbienceVolume(1.0f);

   // Persist the default levels so they're no longer considered unsaved changes
   if (SaveManager)
      SaveManager->SaveSettingsAsync();

   // Update the UI to reflect the default state
   RefreshVisuals();
   CheckForChanges();
}

void UAudioSettingsWidget::SetMasterVolume(const float Value)
{
   if (AudioManager) 
      AudioManager->SetMasterVolume(FMath::Square(Value / 100.0f));
   
   CheckForChanges();
}

void UAudioSettingsWidget::SetMusicVolume(const float Value)
{
   if (AudioManager) 
      AudioManager->SetMusicVolume(FMath::Square(Value / 100.0f));
   
   CheckForChanges();
}

void UAudioSettingsWidget::SetSfxVolume(const float Value)
{
   if (AudioManager) 
      AudioManager->SetSFXVolume(FMath::Square(Value / 100.0f));
   
   CheckForChanges();
}

void UAudioSettingsWidget::SetAmbienceVolume(const float Value)
{
   if (AudioManager) 
      AudioManager->SetAmbienceVolume(FMath::Square(Value / 100.0f));
   
   CheckForChanges();
}
#pragma endregion

#pragma region METHODS
void UAudioSettingsWidget::InitializeElements()
{
   // Position the sliders according to current engine memory
   RefreshVisuals();

   // Bind all slider movements to update the engine in real-time
   if (MasterVolumeSlider)
      MasterVolumeSlider->OnValueChanged.AddUniqueDynamic(this, &UAudioSettingsWidget::SetMasterVolume);
       
   if (MusicVolumeSlider)
      MusicVolumeSlider->OnValueChanged.AddUniqueDynamic(this, &UAudioSettingsWidget::SetMusicVolume);
       
   if (SfxVolumeSlider)
      SfxVolumeSlider->OnValueChanged.AddUniqueDynamic(this, &UAudioSettingsWidget::SetSfxVolume);
       
   if (AmbienceVolumeSlider)
      AmbienceVolumeSlider->OnValueChanged.AddUniqueDynamic(this, &UAudioSettingsWidget::SetAmbienceVolume);
   
    // Initial check
    CheckForChanges();
}

void UAudioSettingsWidget::CheckForChanges() const
{
   if (!AudioManager || !SaveManager)
      return;

   // Compare current engine levels with the saved ones to toggle Apply
   const FSoundLevels& CurrentLevels = AudioManager->GetCurrentLevels();

   bool bIsDirty = false;
   if (!FMath::IsNearlyEqual(CurrentLevels.Master,  SaveManager->GetSaveSettings()->MasterVolume) ||
       !FMath::IsNearlyEqual(CurrentLevels.Music, SaveManager->GetSaveSettings()->MusicVolume) ||
       !FMath::IsNearlyEqual(CurrentLevels.SFX, SaveManager->GetSaveSettings()->SfxVolume) ||
       !FMath::IsNearlyEqual(CurrentLevels.Voice, SaveManager->GetSaveSettings()->VoiceVolume) ||
       !FMath::IsNearlyEqual(CurrentLevels.Ambience, SaveManager->GetSaveSettings()->AmbienceVolume))
   {
      bIsDirty = true;
   }

   if (ApplyButton)
      ApplyButton->SetIsEnabled(bIsDirty);

   // Toggle Reset if we are not already at default values
   if (ResetButton)
      ResetButton->SetIsEnabled(!AreSettingsDefault());
}

bool UAudioSettingsWidget::AreSettingsDefault() const
{
   if (!AudioManager)
      return true;

   const FSoundLevels& CurrentLevels = AudioManager->GetCurrentLevels();
    
   // Return true only if all levels are exactly 1.0f
   return FMath::IsNearlyEqual(CurrentLevels.Master, 1.0f) &&
          FMath::IsNearlyEqual(CurrentLevels.Music, 1.0f) &&
          FMath::IsNearlyEqual(CurrentLevels.SFX, 1.0f) &&
          FMath::IsNearlyEqual(CurrentLevels.Ambience, 1.0f);
}

void UAudioSettingsWidget::OnSettingsApplied() const
{
   ApplyAudioSettings();
   CheckForChanges();
}

void UAudioSettingsWidget::RefreshVisuals() const
{
   if (!AudioManager)
      return;
   
   const FSoundLevels& CurrentLevels = AudioManager->GetCurrentLevels();

   if (MasterVolumeSlider)   
      MasterVolumeSlider->SetValue(FMath::Sqrt(CurrentLevels.Master) * 100.0f);
   
   if (MusicVolumeSlider)    
      MusicVolumeSlider->SetValue(FMath::Sqrt(CurrentLevels.Music) * 100.0f);
   
   if (SfxVolumeSlider)      
      SfxVolumeSlider->SetValue(FMath::Sqrt(CurrentLevels.SFX) * 100.0f);
   
   if (AmbienceVolumeSlider) 
      AmbienceVolumeSlider->SetValue(FMath::Sqrt(CurrentLevels.Ambience) * 100.0f);
}

void UAudioSettingsWidget::OnApplyConfirmed()
{
	OnSettingsApplied();
}

void UAudioSettingsWidget::OnResetConfirmed()
{
	ResetToDefaultAudioSettings();
}
#pragma endregion