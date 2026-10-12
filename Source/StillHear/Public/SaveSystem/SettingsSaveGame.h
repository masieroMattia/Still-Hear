#pragma once

#include "Input/InputDeviceType.h"

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SettingsSaveGame.generated.h"

/**
 * This SaveGame class is used to store user settings such as audio levels
 * and other configurable options.
 */
UCLASS()
class STILLHEAR_API USettingsSaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:
	// Audio
	UPROPERTY(BlueprintReadWrite)
	float MasterVolume = 1.0f;

	UPROPERTY(BlueprintReadWrite)
	float MusicVolume = 1.0f;

	UPROPERTY(BlueprintReadWrite)
	float SfxVolume = 1.0f;

	UPROPERTY(BlueprintReadWrite)
	float VoiceVolume = 1.0f;

	UPROPERTY(BlueprintReadWrite)
	float AmbienceVolume = 1.0f;

	// Input rebinding
	UPROPERTY(BlueprintReadWrite)
	TArray<struct FBindingData> Bindings;

	// Controls
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<class UInputMappingContext> GamepadContext = nullptr;

	UPROPERTY(BlueprintReadWrite)
	int32 GamepadPresetIndex = 0;

	UPROPERTY(BlueprintReadWrite)
	bool bActivateVibration = true;

	UPROPERTY(BlueprintReadWrite)
	float VibrationIntensity = 100.0f;
	
	// Language
	UPROPERTY(BlueprintReadWrite)
	FString Language = TEXT("en");

	// Graphic
	UPROPERTY(BlueprintReadWrite)
	float DisplayGamma = 2.2f;

	// Upscaling
	UPROPERTY(BlueprintReadWrite)
	FString ActiveUpscaler = TEXT("TSR");

	UPROPERTY(BlueprintReadWrite)
	int32 FSR_SuperResolutionIndex = 0;

	UPROPERTY(BlueprintReadWrite)
	int32 FSR_FrameGenerationIndex = 0;

	UPROPERTY(BlueprintReadWrite)
	int32 DLSS_SuperResolutionIndex = 0;

	UPROPERTY(BlueprintReadWrite)
	int32 DLSS_FrameGenerationIndex = 0;

	UPROPERTY(BlueprintReadWrite)
	int32 DLSS_ReflexIndex = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Collectibles")
	TSet<FName> CollectedCollectibles;
};
