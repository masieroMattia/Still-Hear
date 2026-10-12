// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/Controls/Gamepad/GamepadBindingsWidget.h"

#include "CommonInputSubsystem.h"
#include "InputMappingContext.h"
#include "Character/StillHearPlayerController.h"
#include "Components/Image.h"
#include "Data/DataTables/GamepadImageData.h"
#include "Kismet/GameplayStatics.h"
#include "SaveSystem/SaveSubsystem.h"
#include "SaveSystem/SettingsSaveGame.h"
#include "UI/Elements/ButtonBase.h"
#include "UI/Elements/RotatorBase.h"
#include "UI/Elements/SliderBase.h"

void UGamepadBindingsWidget::UpdateApplyButton()
{
	if (IsValid(ApplyButton))
	{
		if (bDifferentPresetRotator || bDifferentActivatedVibration || bDifferentVibrationIntensity)
		{
			ApplyButton->SetVisibility(ESlateVisibility::Visible);
			if (!ApplyInputAction.IsNull())
			{
				ApplyButton->SetTriggeringInputAction(ApplyInputAction);
			}
		}
		else
		{
			ApplyButton->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void UGamepadBindingsWidget::OnApplyConfirmed()
{
	const int32 RotatorIndex = GamepadPresetRotator->GetSelectedIndex();
	const FGamepadImageData* GamepadImageData = GamepadDataTable->FindRow<FGamepadImageData>(FName(*FString::FromInt(RotatorIndex)), Context);
	UInputMappingContext* SelectedMapping = GamepadImageData->InputMappingContext.LoadSynchronous();
	
	USettingsSaveGame* SaveGame = SaveSubsystem->GetSaveSettings();
	SaveGame->GamepadContext = SelectedMapping;
	SaveGame->GamepadPresetIndex = RotatorIndex;
	SaveGame->bActivateVibration = VibrationRotator->GetSelectedIndex() == 0;
	SaveGame->VibrationIntensity = VibrationIntensitySlider->GetValue();

	SaveSubsystem->SaveSettingsAsync();

	CurrentPresetRotatorIndex = RotatorIndex;
	CurrentVibrationActivatedIndex = VibrationRotator->GetSelectedIndex();
	CurrentVibrationIntensity = VibrationIntensitySlider->GetValue();

	bDifferentPresetRotator = false;
	bDifferentActivatedVibration = false;
	bDifferentVibrationIntensity = false;

	AStillHearPlayerController* PC = Cast<AStillHearPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	PC->ApplyControlsSettings();
	
	UpdateApplyButton();

	if (CurrentPresetRotatorIndex != 0 || CurrentVibrationActivatedIndex != 0 || CurrentVibrationIntensity != 100.f)
	{
		DisplayResetToDefaultButton();
	}
	else
	{
		ResetButton->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UGamepadBindingsWidget::OnResetConfirmed()
{
	GamepadPresetRotator->SetSelectedIndex(0);
	CurrentPresetRotatorIndex = 0;
	
	VibrationRotator->SetSelectedIndex(0);
	CurrentVibrationActivatedIndex = 0;
	
	VibrationIntensitySlider->SetValue(100.f);
	CurrentVibrationIntensity = 100.f;

	bDifferentPresetRotator = false;
	bDifferentActivatedVibration = false;
	bDifferentVibrationIntensity = false;

	USettingsSaveGame* SaveGame = SaveSubsystem->GetSaveSettings();
	SaveGame->GamepadContext = DefaultGamepadMappingContext;
	SaveGame->GamepadPresetIndex = 0;
	SaveGame->bActivateVibration = true;
	SaveGame->VibrationIntensity = 100.f;

	SaveSubsystem->SaveSettingsAsync();

	AStillHearPlayerController* PC = Cast<AStillHearPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	PC->ApplyControlsSettings();

	RefreshGamepadImage();
	UpdateApplyButton();
	ResetButton->SetVisibility(ESlateVisibility::Hidden);
}

void UGamepadBindingsWidget::HandleGamepadRotatorSelectionChanged(int32 SelectedIndex, FText SelectedText)
{
	if (SelectedIndex != CurrentPresetRotatorIndex)
	{
		bDifferentPresetRotator = true;
	}
	else
	{
		bDifferentPresetRotator = false;
	}

	UpdateApplyButton();
	RefreshGamepadImage();
}

void UGamepadBindingsWidget::HandleVibrationRotatorSelectionChanged(int32 SelectedIndex, FText SelectedText)
{
	if (SelectedIndex != CurrentVibrationActivatedIndex)
	{
		bDifferentActivatedVibration = true;
	}
	else
	{
		bDifferentActivatedVibration = false;	
	}

	UpdateApplyButton();
}

void UGamepadBindingsWidget::HandleVibrationIntensityChanged(float Value)
{
	if (!FMath::IsNearlyEqual(Value, CurrentVibrationIntensity))
	{
		bDifferentVibrationIntensity = true;
	}
	else
	{
		bDifferentVibrationIntensity = false;	
	}

	UpdateApplyButton();
}

void UGamepadBindingsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (IsValid(ApplyButton))
	{
		ApplyButton->SetVisibility(ESlateVisibility::Hidden);
	}

	const USettingsSaveGame* Settings = SaveSubsystem->GetSaveSettings();
	
	CurrentPresetRotatorIndex = Settings->GamepadPresetIndex;
	CurrentVibrationActivatedIndex = Settings->bActivateVibration ? 0 : 1;
	CurrentVibrationIntensity = Settings->VibrationIntensity;
	
	GamepadPresetRotator->SetSelectedIndex(CurrentPresetRotatorIndex);
	VibrationRotator->SetSelectedIndex(CurrentVibrationActivatedIndex);
	VibrationIntensitySlider->SetValue(CurrentVibrationIntensity);
	
	RefreshGamepadImage();

	if (ResetButton)
	{
		if (CurrentPresetRotatorIndex != 0 || CurrentVibrationActivatedIndex != 0 || CurrentVibrationIntensity != 100.f)
		{
			DisplayResetToDefaultButton();
		}
		else
		{
			ResetButton->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void UGamepadBindingsWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	InputSubsystem = UCommonInputSubsystem::Get(GetOwningLocalPlayer());
	InputSubsystem->OnInputMethodChangedNative.AddUObject(this, &UGamepadBindingsWidget::HandleInputChanged);

	GamepadPresetRotator->OnSelectionChanged.AddUniqueDynamic(this, &ThisClass::HandleGamepadRotatorSelectionChanged);
	VibrationRotator->OnSelectionChanged.AddUniqueDynamic(this, &ThisClass::HandleVibrationRotatorSelectionChanged);
	VibrationIntensitySlider->OnValueChanged.AddUniqueDynamic(this, &ThisClass::HandleVibrationIntensityChanged);

	SaveSubsystem = GetGameInstance()->GetSubsystem<USaveSubsystem>();
}

void UGamepadBindingsWidget::HandleInputChanged(ECommonInputType InputType)
{
	if (InputType == ECommonInputType::Gamepad)
	{
		RefreshGamepadImage();
	}
}

void UGamepadBindingsWidget::RefreshGamepadImage()
{
	const int32 RotatorIndex = GamepadPresetRotator->GetSelectedIndex();

	const FGamepadImageData* GamepadImageData = GamepadDataTable->FindRow<FGamepadImageData>(FName(*FString::FromInt(RotatorIndex)), Context);
	
	const FName CurrentGamepadName = InputSubsystem->GetCurrentGamepadName();

	const USettingsSaveGame* Settings = SaveSubsystem->GetSaveSettings();
	
	if (CurrentGamepadName == FName("XB1"))
	{
		if (Settings->Language == "en")
		{
			GamepadImage->SetBrushFromTexture(GamepadImageData->XboxImageEng.LoadSynchronous());
		}
		else if (Settings->Language == "it")
		{
			GamepadImage->SetBrushFromTexture(GamepadImageData->XboxImageIta.LoadSynchronous());
		}
	}
	else if (CurrentGamepadName == FName("PS5"))
	{
		if (Settings->Language == "en")
		{
			GamepadImage->SetBrushFromTexture(GamepadImageData->PlaystationImageEng.LoadSynchronous());
		}
		else if (Settings->Language == "it")
		{
			GamepadImage->SetBrushFromTexture(GamepadImageData->PlaystationImageIta.LoadSynchronous());
		}
	}
}

void UGamepadBindingsWidget::DisplayResetToDefaultButton()
{
	if (IsValid(ResetButton))
	{
		ResetButton->SetVisibility(ESlateVisibility::Visible);
		if (!ResetInputAction.IsNull())
		{
			ResetButton->SetTriggeringInputAction(ResetInputAction);
		}
	}
}
