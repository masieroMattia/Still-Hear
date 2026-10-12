#include "Audio/WorldStereoBalanceSubmix.h"
#include "DSP/Dsp.h"

void FWorldStereoBalanceSubmix::Init(const FSoundEffectSubmixInitData& InitData)
{
	CachedSampleRate = (InitData.SampleRate > 0.f) ? InitData.SampleRate : 48000.f;
	SmoothedPan = 0.0f;
	SmoothedStrength = 1.0f;
}

void FWorldStereoBalanceSubmix::OnPresetChanged()
{
	const UWorldStereoBalanceSubmixPreset* PresetObj =
		Cast<UWorldStereoBalanceSubmixPreset>(GetPreset());

	if (!PresetObj)
		return;

	CurrentSettings = PresetObj->Settings;
	CurrentSettings.Pan = FMath::Clamp(CurrentSettings.Pan, -1.0f, 1.0f);
	CurrentSettings.Strength = FMath::Clamp(CurrentSettings.Strength, 0.0f, 1.0f);
}


void FWorldStereoBalanceSubmix::ComputeEqualPowerGains(float Pan, float Strength, float& OutGainL, float& OutGainR)
{
	Pan = FMath::Clamp(Pan, -1.0f, 1.0f);
	Strength = FMath::Clamp(Strength, 0.0f, 1.0f);

	// equal-power panning
	const float t = (Pan + 1.0f) * 0.5f;
	const float PanL = FMath::Cos(t * HALF_PI);
	const float PanR = FMath::Sin(t * HALF_PI);
	
	OutGainL = FMath::Lerp(1.0f, PanL, Strength);
	OutGainR = FMath::Lerp(1.0f, PanR, Strength);
}

void FWorldStereoBalanceSubmix::OnProcessAudio(
	const FSoundEffectSubmixInputData& InData,
	FSoundEffectSubmixOutputData& OutData)
{
	if (!InData.AudioBuffer || !OutData.AudioBuffer)
	{
		return;
	}
	
	*OutData.AudioBuffer = *InData.AudioBuffer;

	const int32 NumChannels = InData.NumChannels;
	if (NumChannels < 2)
	{
		return; 
	}
	
	const int32 NumFrames = InData.NumFrames;

	const float DeltaTime = (NumFrames > 0 && CachedSampleRate > 0.f)
		? (float)NumFrames / CachedSampleRate
		: 0.0f;

	const float Alpha = (DeltaTime > 0.0f)
		? (1.0f - FMath::Exp(-SmoothingSpeed * DeltaTime))
		: 1.0f;

	SmoothedPan = FMath::Lerp(SmoothedPan, CurrentSettings.Pan, Alpha);
	SmoothedStrength = FMath::Lerp(SmoothedStrength, CurrentSettings.Strength, Alpha);

	float GainL = 1.0f, GainR = 1.0f;
	ComputeEqualPowerGains(SmoothedPan, SmoothedStrength, GainL, GainR);

	float* Buffer = OutData.AudioBuffer->GetData();
	
	for (int32 Frame = 0; Frame < NumFrames; ++Frame)
	{
		const int32 Base = Frame * NumChannels;
		Buffer[Base + 0] *= GainL; 
		Buffer[Base + 1] *= GainR; 
	}
}


// ---------------- Preset ----------------

void UWorldStereoBalanceSubmixPreset::SetSettings(const FWorldStereoBalanceSubmixSettings& InSettings)
{
	Settings = InSettings;
	UpdateSettings(Settings); 
}
