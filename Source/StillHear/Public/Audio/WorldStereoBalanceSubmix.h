#pragma once

#include "CoreMinimal.h"
#include "Sound/SoundEffectSubmix.h"
#include "WorldStereoBalanceSubmix.generated.h"

// ========================================================================
// Settings
// ========================================================================

USTRUCT(BlueprintType)
struct STILLHEAR_API FWorldStereoBalanceSubmixSettings
{
	GENERATED_BODY()

	// -1 = left, 0 = center, +1 = right
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="StereoBalance", meta=(ClampMin="-1.0", ClampMax="1.0"))
	float Pan = 0.0f;

	// 0 = no effect, 1 = full effect
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="StereoBalance", meta=(ClampMin="0.0", ClampMax="1.0"))
	float Strength = 1.0f;
};

// ========================================================================
// Effect runtime (audio thread)
// ========================================================================

class STILLHEAR_API FWorldStereoBalanceSubmix : public FSoundEffectSubmix
{
public:
	FWorldStereoBalanceSubmix() = default;
	virtual ~FWorldStereoBalanceSubmix() override = default;

	virtual void Init(const FSoundEffectSubmixInitData& InitData) override;
	virtual void OnPresetChanged() override;

	virtual void OnProcessAudio(
		const FSoundEffectSubmixInputData& InData,
		FSoundEffectSubmixOutputData& OutData) override;

private:
	// Copy of the current settings to use during audio processing
	FWorldStereoBalanceSubmixSettings CurrentSettings;
	
	float CachedSampleRate = 48000.0f;

	// smoothing
	float SmoothedPan = 0.0f;
	float SmoothedStrength = 1.0f;
	float SmoothingSpeed = 12.0f;

	// utility
	static void ComputeEqualPowerGains(float Pan, float Strength, float& OutGainL, float& OutGainR);
};

// ========================================================================
// Preset (main thread / editor)
// ========================================================================

UCLASS()
class STILLHEAR_API UWorldStereoBalanceSubmixPreset : public USoundEffectSubmixPreset
{
	GENERATED_BODY()

public:
	// Macro del template UE: crea instance + binding settings
	EFFECT_PRESET_METHODS(WorldStereoBalanceSubmix)

	UFUNCTION(BlueprintCallable, Category="Audio Effects")
	void SetSettings(const FWorldStereoBalanceSubmixSettings& InSettings);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SubmixEffectPreset")
	FWorldStereoBalanceSubmixSettings Settings;
};
