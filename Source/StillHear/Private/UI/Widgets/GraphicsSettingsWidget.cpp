#include "UI/Widgets/GraphicsSettingsWidget.h"

#include "RHI.h"
#include "DLSSLibrary.h"
#include "HAL/IConsoleManager.h"
#include "UI/Elements/SliderBase.h"
#include "UI/Elements/DropdownBase.h"
#include "SaveSystem/SaveSubsystem.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UI/Elements/PopupButtonBase.h"
#include "SaveSystem/SettingsSaveGame.h"
#include "UI/Widgets/FSRSettingsWidget.h"
#include "UI/Widgets/DLSSSettingsWidget.h"
#include "GameFramework/GameUserSettings.h"
#include "UI/Widgets/CustomQualitySettingsWidget.h"
#include "Internationalization/Internationalization.h"

#pragma region UFUNCTIONS
void UGraphicsSettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UserSettings = GEngine->GetGameUserSettings();
	
	if (const UGameInstance* GI = GetGameInstance())
		SaveManager = GI->GetSubsystem<USaveSubsystem>();
	
	InitializeQualityDropdown();
	InitializeResolutionDropdown();
	InitializeWindowModeDropdown();
	InitializeGammaSlider();
	InitializeUpscalerDropdown();
	InitializeLanguageDropdown();

	if (CustomQualitySettingsContainer)
	{
		CustomQualitySettingsContainer->SetVisibility(ESlateVisibility::Collapsed);
		CustomQualitySettingsContainer->OnSettingsChanged.RemoveAll(this);
		CustomQualitySettingsContainer->OnSettingsChanged.AddUniqueDynamic(this, &UGraphicsSettingsWidget::CheckForChanges);
	}
	
	RefreshVisuals();
	CheckForChanges();
}

void UGraphicsSettingsWidget::ApplyGraphicsSettings()
{
	if (!UserSettings)
		return;

	// Retrieve current values from the UI and write them to GameUserSettings
	if (QualityDropdown) 
	{
		const int32 SelectedQualityIdx = QualityDropdown->GetSelectedIndex();
		if (SelectedQualityIdx == 5 && bAllowCustomQuality) // CUSTOM
		{
			if (CustomQualitySettingsContainer)
			{
				CustomQualitySettingsContainer->ApplyCustomQualitySettings(UserSettings);
			}
		}
		else
		{
			UserSettings->SetOverallScalabilityLevel(SelectedQualityIdx);
		}
	}
            
	if (WindowModeDropdown) 
		UserSettings->SetFullscreenMode(static_cast<EWindowMode::Type>(WindowModeDropdown->GetSelectedIndex()));
            
	if (ResolutionDropdown && SupportedResolutions.IsValidIndex(ResolutionDropdown->GetSelectedIndex())) 
		UserSettings->SetScreenResolution(SupportedResolutions[ResolutionDropdown->GetSelectedIndex()]);

	// Force the engine to apply changes to the screen and save them to the .ini file
	UserSettings->ApplySettings(true);

	// Apply Upscaler logic
	if (UpscalerDropdown && AvailableUpscalers.IsValidIndex(UpscalerDropdown->GetSelectedIndex()))
	{
		const FString SelectedUpscaler = AvailableUpscalers[UpscalerDropdown->GetSelectedIndex()];

		// Helper: disable FSR (correct CVar is r.FidelityFX.FSR.Enabled, NOT FSR3.Enabled)
		auto DisableFSR = []()
		{
			if (IConsoleVariable* CV = IConsoleManager::Get().FindConsoleVariable(TEXT("r.FidelityFX.FSR.Enabled")))
				CV->Set(0, ECVF_SetByGameSetting);
			if (IConsoleVariable* CV = IConsoleManager::Get().FindConsoleVariable(TEXT("r.FidelityFX.FI.Enabled")))
				CV->Set(0, ECVF_SetByGameSetting);
		};

		// Helper: disable DLSS completely (SR + screen percentage reset)
		auto DisableDLSS = []()
		{
			UDLSSLibrary::EnableDLSS(false);
			if (IConsoleVariable* CVarSP = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ScreenPercentage")))
			{
				const EConsoleVariableFlags Priority = static_cast<EConsoleVariableFlags>(CVarSP->GetFlags() & ECVF_SetByMask);
				CVarSP->Set(100.0f, Priority != 0 ? Priority : ECVF_SetByGameSetting);
			}
		};

		if (SelectedUpscaler == "DLSS" && DLSSSettingsContainer)
		{
			DisableFSR();
			// Re-set AA to TAA/TSR so DLSS can take over the TAA slot
			if (IConsoleVariable* CVarAA = IConsoleManager::Get().FindConsoleVariable(TEXT("r.AntiAliasingMethod")))
				CVarAA->Set(2, ECVF_SetByGameSetting); // 2 = TAA (DLSS overrides this)
			DLSSSettingsContainer->ApplyDLSSSettings();
		}
		else if (SelectedUpscaler == "FSR" && FSRSettingsContainer)
		{
			DisableDLSS();
			FSRSettingsContainer->ApplyFSRSettings();
		}
		else if (SelectedUpscaler == "TSR")
		{
			DisableFSR();
			DisableDLSS();

			// Enable TSR
			if (IConsoleVariable* CVarAA = IConsoleManager::Get().FindConsoleVariable(TEXT("r.AntiAliasingMethod")))
				CVarAA->Set(4, ECVF_SetByGameSetting); // 4 = TSR
		}
	}
	
	// Only commit the gamma to the engine once the user confirms the change
	ApplyGammaToEngine(CurrentGamma);

	if (SaveManager && SaveManager->GetSaveSettings())
	{
		SaveManager->GetSaveSettings()->DisplayGamma = CurrentGamma;

		if (UpscalerDropdown && AvailableUpscalers.IsValidIndex(UpscalerDropdown->GetSelectedIndex()))
		{
			SaveManager->GetSaveSettings()->ActiveUpscaler = AvailableUpscalers[UpscalerDropdown->GetSelectedIndex()];
		}

		if (DLSSSettingsContainer)
		{
			SaveManager->GetSaveSettings()->DLSS_SuperResolutionIndex = DLSSSettingsContainer->GetSuperResolutionIndex();
			SaveManager->GetSaveSettings()->DLSS_FrameGenerationIndex = DLSSSettingsContainer->GetFrameGenerationIndex();
			SaveManager->GetSaveSettings()->DLSS_ReflexIndex = DLSSSettingsContainer->GetReflexIndex();
		}

		if (FSRSettingsContainer)
		{
			SaveManager->GetSaveSettings()->FSR_SuperResolutionIndex = FSRSettingsContainer->GetSuperResolutionIndex();
			SaveManager->GetSaveSettings()->FSR_FrameGenerationIndex = FSRSettingsContainer->GetFrameGenerationIndex();
		}

		if (LanguageDropdown && AvailableLanguages.IsValidIndex(LanguageDropdown->GetSelectedIndex()))
		{
			const FString LanguageCode = AvailableLanguages[LanguageDropdown->GetSelectedIndex()];
			FInternationalization::Get().SetCurrentCulture(LanguageCode);
			SaveManager->GetSaveSettings()->Language = LanguageCode;
		}

		SaveManager->SaveSettingsAsync();
	}
        
	CheckForChanges();
    
	if (ApplyButton)
        ApplyButton->SetIsEnabled(false);
}

void UGraphicsSettingsWidget::SetGamma(const float Value)
{
	CurrentGamma = Value;
	CheckForChanges();
}

void UGraphicsSettingsWidget::ApplyGammaToEngine(const float Value) const
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		const FString Command = FString::Printf(TEXT("gamma %f"), Value);
		PC->ConsoleCommand(Command);
	}
}

void UGraphicsSettingsWidget::ResetToDefaultGraphicsSettings()
{
	if (!UserSettings)
		return;

	// Reset the engine to factory/hardware default values
	UserSettings->SetToDefaults();
	UserSettings->SetOverallScalabilityLevel(2); // High
	UserSettings->ApplySettings(true);
    
	SetGamma(2.2f);
	ApplyGammaToEngine(CurrentGamma);

	if (SaveManager && SaveManager->GetSaveSettings())
	{
		SaveManager->GetSaveSettings()->DisplayGamma = CurrentGamma;

		FInternationalization::Get().SetCurrentCulture(TEXT("en"));
		SaveManager->GetSaveSettings()->Language = TEXT("en");

		SaveManager->SaveSettingsAsync();
	}

	// Update the UI to reflect new values and disable the Apply button
	RefreshVisuals();
	CheckForChanges();
}
#pragma endregion
	
#pragma region METHODS
void UGraphicsSettingsWidget::InitializeQualityDropdown()
{
	if (!QualityDropdown || !UserSettings)
		return;

	TArray<FText> QualityOptions = 
	{
		FText::FromString("LOW"),
		FText::FromString("MEDIUM"),
		FText::FromString("HIGH"),
		FText::FromString("EPIC"),
		FText::FromString("CINEMATIC")
	 };

	if (bAllowCustomQuality)
	{
		QualityOptions.Add(FText::FromString("CUSTOM"));
	}
    
	QualityDropdown->SetOptions(QualityOptions);
	
	const int32 OverallLevel = UserSettings->GetOverallScalabilityLevel();
	if (OverallLevel >= 0 && OverallLevel < 5)
	{
		QualityDropdown->SetSelectedIndex(OverallLevel);
	}
	else
	{
		if (bAllowCustomQuality)
		{
			QualityDropdown->SetSelectedIndex(5); // CUSTOM
		}
		else
		{
			QualityDropdown->SetSelectedIndex(4);
		}
	}
    
	// When the UI changes, simply trigger the dirty check
	QualityDropdown->OnSelectionChanged.RemoveAll(this);
	QualityDropdown->OnSelectionChanged.AddUniqueDynamic(this, &UGraphicsSettingsWidget::HandleQualityChanged);
}

void UGraphicsSettingsWidget::InitializeResolutionDropdown()
{
	if (!ResolutionDropdown || !UserSettings)
		return;

	// Request all supported fullscreen resolutions from the engine for the current monitor
	SupportedResolutions.Empty();
	UKismetSystemLibrary::GetSupportedFullscreenResolutions(SupportedResolutions);

	// Create the array of text options for the Rotator UI
	TArray<FText> ResolutionTexts;
	int32 CurrentResIndex = 0;
	const FIntPoint CurrentResolutions = UserSettings->GetScreenResolution();

	for (int32 i = 0; i < SupportedResolutions.Num(); ++i)
	{
		const FIntPoint Res = SupportedResolutions[i];
       
		FString WidthStr = FString::FromInt(Res.X);
		FString HeightStr = FString::FromInt(Res.Y);

		// Pad width on the left to 4 characters using Figure Space (\u2007)
		while (WidthStr.Len() < 4)
		{
			WidthStr = FString(TEXT("\u2007")) + WidthStr;
		}

		// Pad height on the right to 4 characters using Figure Space (\u2007)
		while (HeightStr.Len() < 4)
		{
			HeightStr = HeightStr + FString(TEXT("\u2007"));
		}

		// Format the display string, e.g., "1920 x 1080"
		const FString ResString = FString::Printf(TEXT("%s x %s"), *WidthStr, *HeightStr);
		ResolutionTexts.Add(FText::FromString(ResString));

		// Cache the index if this matches the currently active engine resolution
		if (Res == CurrentResolutions)
			CurrentResIndex = i;
	}

	// Populate the Rotator and set its default visual state
	ResolutionDropdown->SetOptions(ResolutionTexts);
	ResolutionDropdown->SetSelectedIndex(CurrentResIndex);
	ResolutionDropdown->OnSelectionChanged.AddUniqueDynamic(this, &UGraphicsSettingsWidget::HandleResolutionChanged);
}

void UGraphicsSettingsWidget::InitializeWindowModeDropdown()
{
	if (!WindowModeDropdown || !UserSettings)
		return;

	const TArray WindowModeOptions = {
		FText::FromString("FULLSCREEN"),
		FText::FromString("WINDOWED FULLSCREEN"),
		FText::FromString("WINDOWED")
	 };
    
	WindowModeDropdown->SetOptions(WindowModeOptions);
	WindowModeDropdown->SetSelectedIndex(UserSettings->GetFullscreenMode());
	WindowModeDropdown->OnSelectionChanged.AddUniqueDynamic(this, &UGraphicsSettingsWidget::HandleWindowModeChanged);
}

void UGraphicsSettingsWidget::InitializeGammaSlider()
{
	// Load gamma from our custom SaveSubsystem
	if (SaveManager && SaveManager->GetSaveSettings())
		CurrentGamma = SaveManager->GetSaveSettings()->DisplayGamma;

	// Sync the engine's gamma to the saved value (the gamma cvar doesn't persist on its own)
	ApplyGammaToEngine(CurrentGamma);

	if (GammaSlider)
	{
		GammaSlider->SetValue(CurrentGamma);
		GammaSlider->OnValueChanged.AddUniqueDynamic(this, &UGraphicsSettingsWidget::SetGamma);
	}
}

void UGraphicsSettingsWidget::InitializeUpscalerDropdown()
{
	if (!UpscalerDropdown)
		return;

	AvailableUpscalers.Empty();
	AvailableUpscalers.Add("TSR");
	AvailableUpscalers.Add("FSR");

	if (UDLSSLibrary::IsDLSSSupported())
	{
		AvailableUpscalers.Add("DLSS");
	}

	TArray<FText> UpscalerOptions;
	for (const FString& UpscalerMethod : AvailableUpscalers)
	{
		UpscalerOptions.Add(FText::FromString(UpscalerMethod));
	}
    
	UpscalerDropdown->SetOptions(UpscalerOptions);
	UpscalerDropdown->SetSelectedIndex(0); // Default to TSR
    
	UpscalerDropdown->OnSelectionChanged.AddUniqueDynamic(this, &UGraphicsSettingsWidget::HandleUpscalerChanged);

	// Hide both sub-menus initially, but bind to their changes
	if (DLSSSettingsContainer)
	{
		DLSSSettingsContainer->SetVisibility(ESlateVisibility::Collapsed);
		DLSSSettingsContainer->OnSettingsChanged.AddUniqueDynamic(this, &UGraphicsSettingsWidget::CheckForChanges);
	}
		
	if (FSRSettingsContainer)
	{
		FSRSettingsContainer->SetVisibility(ESlateVisibility::Collapsed);
		FSRSettingsContainer->OnSettingsChanged.AddUniqueDynamic(this, &UGraphicsSettingsWidget::CheckForChanges);
	}
}

void UGraphicsSettingsWidget::HandleUpscalerChanged(const int32 SelectedIndex, FText SelectedText)
{
	// First hide everything
	if (DLSSSettingsContainer)
		DLSSSettingsContainer->SetVisibility(ESlateVisibility::Collapsed);
	if (FSRSettingsContainer)
		FSRSettingsContainer->SetVisibility(ESlateVisibility::Collapsed);

	if (AvailableUpscalers.IsValidIndex(SelectedIndex))
	{
		const FString SelectedUpscaler = AvailableUpscalers[SelectedIndex];

		// Show the corresponding settings container
		if (SelectedUpscaler == "DLSS" && DLSSSettingsContainer)
		{
			DLSSSettingsContainer->SetVisibility(ESlateVisibility::Visible);
		}
		else if (SelectedUpscaler == "FSR" && FSRSettingsContainer)
		{
			FSRSettingsContainer->SetVisibility(ESlateVisibility::Visible);
		}
	}

	CheckForChanges();
}

void UGraphicsSettingsWidget::HandleQualityChanged(const int32 SelectedIndex, FText SelectedText)
{
	if (CustomQualitySettingsContainer)
	{
		CustomQualitySettingsContainer->SetVisibility((SelectedIndex == 5 && bAllowCustomQuality) ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
	CheckForChanges();
}
void UGraphicsSettingsWidget::HandleResolutionChanged(const int32 SelectedIndex, FText SelectedText)
{
	CheckForChanges();
}
void UGraphicsSettingsWidget::HandleWindowModeChanged(const int32 SelectedIndex, FText SelectedText)
{
	CheckForChanges();
}

void UGraphicsSettingsWidget::OnSubSettingsChanged()
{
	if (ApplyButton)
	{
		ApplyButton->SetIsEnabled(true);
	}
}

void UGraphicsSettingsWidget::CheckForChanges()
{
	if (!UserSettings) 
		return;

	bool bIsDirty = false;

	// Compare the UI state with the saved Engine state
	if (QualityDropdown)
	{
		const int32 SelectedQualityIdx = QualityDropdown->GetSelectedIndex();
		const int32 EngineQualityLevel = UserSettings->GetOverallScalabilityLevel();
		if (SelectedQualityIdx == 5 && bAllowCustomQuality) // CUSTOM
		{
			if (EngineQualityLevel != -1)
			{
				bIsDirty = true;
			}
			else if (CustomQualitySettingsContainer && CustomQualitySettingsContainer->GetIsDirty(UserSettings))
			{
				bIsDirty = true;
			}
		}
		else
		{
			if (SelectedQualityIdx != EngineQualityLevel)
			{
				bIsDirty = true;
			}
		}
	}

	if (WindowModeDropdown && WindowModeDropdown->GetSelectedIndex() != UserSettings->GetFullscreenMode())
		bIsDirty = true;

	if (ResolutionDropdown && SupportedResolutions.IsValidIndex(ResolutionDropdown->GetSelectedIndex()))
	{
		if (SupportedResolutions[ResolutionDropdown->GetSelectedIndex()] != UserSettings->GetScreenResolution())
			bIsDirty = true;
	}

	// For Gamma, check if it differs from the one saved
	if (SaveManager && SaveManager->GetSaveSettings())
	{
		if (!FMath::IsNearlyEqual(CurrentGamma, SaveManager->GetSaveSettings()->DisplayGamma))
			bIsDirty = true;

		// Check if active upscaler changed
		if (UpscalerDropdown && AvailableUpscalers.IsValidIndex(UpscalerDropdown->GetSelectedIndex()))
		{
			if (AvailableUpscalers[UpscalerDropdown->GetSelectedIndex()] != SaveManager->GetSaveSettings()->ActiveUpscaler)
				bIsDirty = true;
		}
		
		// Check sub-settings
		if (DLSSSettingsContainer)
		{
			if (DLSSSettingsContainer->GetSuperResolutionIndex() != SaveManager->GetSaveSettings()->DLSS_SuperResolutionIndex ||
				DLSSSettingsContainer->GetFrameGenerationIndex() != SaveManager->GetSaveSettings()->DLSS_FrameGenerationIndex ||
				DLSSSettingsContainer->GetReflexIndex() != SaveManager->GetSaveSettings()->DLSS_ReflexIndex)
				bIsDirty = true;
		}

		if (FSRSettingsContainer)
		{
			if (FSRSettingsContainer->GetSuperResolutionIndex() != SaveManager->GetSaveSettings()->FSR_SuperResolutionIndex ||
				FSRSettingsContainer->GetFrameGenerationIndex() != SaveManager->GetSaveSettings()->FSR_FrameGenerationIndex)
				bIsDirty = true;
		}

		if (LanguageDropdown && AvailableLanguages.IsValidIndex(LanguageDropdown->GetSelectedIndex()))
		{
			if (AvailableLanguages[LanguageDropdown->GetSelectedIndex()] != SaveManager->GetSaveSettings()->Language)
				bIsDirty = true;
		}
	}

	if (ApplyButton) 
		ApplyButton->SetIsEnabled(bIsDirty);
        
	if (ResetButton)
		ResetButton->SetIsEnabled(bIsDirty);
}

void UGraphicsSettingsWidget::RefreshVisuals()
{
	if (!UserSettings)
		return;

	if (QualityDropdown)
	{
		const int32 OverallLevel = UserSettings->GetOverallScalabilityLevel();
		if (OverallLevel >= 0 && OverallLevel < 5)
		{
			QualityDropdown->SetSelectedIndex(OverallLevel);
			if (CustomQualitySettingsContainer)
			{
				CustomQualitySettingsContainer->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
		else
		{
			if (bAllowCustomQuality)
			{
				QualityDropdown->SetSelectedIndex(5); // CUSTOM
				if (CustomQualitySettingsContainer)
				{
					CustomQualitySettingsContainer->SetVisibility(ESlateVisibility::Visible);
				}
			}
			else
			{
				QualityDropdown->SetSelectedIndex(4);
				if (CustomQualitySettingsContainer)
				{
					CustomQualitySettingsContainer->SetVisibility(ESlateVisibility::Collapsed);
				}
			}
		}
	}

	if (CustomQualitySettingsContainer)
	{
		CustomQualitySettingsContainer->RefreshVisuals(UserSettings);
	}

	if (WindowModeDropdown)
		WindowModeDropdown->SetSelectedIndex(UserSettings->GetFullscreenMode());

	if (ResolutionDropdown)
	{
		const FIntPoint CurrentRes = UserSettings->GetScreenResolution();
		bool bFoundMatch = false;

		for (int32 i = 0; i < SupportedResolutions.Num(); ++i)
		{
			if (SupportedResolutions[i] == CurrentRes)
			{
				ResolutionDropdown->SetSelectedIndex(i);
				bFoundMatch = true;
				break;
			}
		}
       
		if (!bFoundMatch && !SupportedResolutions.IsEmpty())
			ResolutionDropdown->SetSelectedIndex(0);
	}

	if (GammaSlider)
		GammaSlider->SetValue(CurrentGamma);

	// Restore Language
	if (LanguageDropdown && SaveManager && SaveManager->GetSaveSettings())
	{
		const int32 LangIndex = AvailableLanguages.Find(SaveManager->GetSaveSettings()->Language);
		LanguageDropdown->SetSelectedIndex(LangIndex != INDEX_NONE ? LangIndex : 0);
	}

	// Restore Upscaler settings
	if (SaveManager && SaveManager->GetSaveSettings())
	{
		const FString SavedUpscaler = SaveManager->GetSaveSettings()->ActiveUpscaler;
		const int32 FoundIndex = AvailableUpscalers.Find(SavedUpscaler);
		if (FoundIndex != INDEX_NONE && UpscalerDropdown)
		{
			UpscalerDropdown->SetSelectedIndex(FoundIndex);
			HandleUpscalerChanged(FoundIndex, FText());
		}

		// Restore sub-choices to the UI directly
		if (DLSSSettingsContainer)
		{
			DLSSSettingsContainer->SetSavedIndices(
				SaveManager->GetSaveSettings()->DLSS_SuperResolutionIndex,
				SaveManager->GetSaveSettings()->DLSS_FrameGenerationIndex,
				SaveManager->GetSaveSettings()->DLSS_ReflexIndex
			);
		}
		if (FSRSettingsContainer)
		{
			FSRSettingsContainer->SetSavedIndices(
				SaveManager->GetSaveSettings()->FSR_SuperResolutionIndex,
				SaveManager->GetSaveSettings()->FSR_FrameGenerationIndex
			);
		}
	}
}

void UGraphicsSettingsWidget::InitializeLanguageDropdown()
{
	if (!LanguageDropdown)
		return;

	AvailableLanguages = { TEXT("en"), TEXT("it") };

	const TArray LanguageOptions = {
		FText::FromString(TEXT("ENGLISH")),
		FText::FromString(TEXT("ITALIANO"))
	};

	LanguageDropdown->SetOptions(LanguageOptions);

	FString SavedLanguage = TEXT("en");
	if (SaveManager && SaveManager->GetSaveSettings())
		SavedLanguage = SaveManager->GetSaveSettings()->Language;

	const int32 SavedIndex = AvailableLanguages.Find(SavedLanguage);
	LanguageDropdown->SetSelectedIndex(SavedIndex != INDEX_NONE ? SavedIndex : 0);

	LanguageDropdown->OnSelectionChanged.RemoveAll(this);
	LanguageDropdown->OnSelectionChanged.AddUniqueDynamic(this, &UGraphicsSettingsWidget::HandleLanguageChanged);
}

void UGraphicsSettingsWidget::HandleLanguageChanged(const int32 SelectedIndex, FText SelectedText)
{
	CheckForChanges();
}

void UGraphicsSettingsWidget::OnApplyConfirmed()
{
	ApplyGraphicsSettings();
}

void UGraphicsSettingsWidget::OnResetConfirmed()
{
	ResetToDefaultGraphicsSettings();
}
#pragma endregion