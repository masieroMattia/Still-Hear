#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AudioDistanceTuningData.generated.h"
/**
 * Data Asset to tune audio distance-based effects
 */
UCLASS()
class STILLHEAR_API UAudioDistanceTuningData : public UDataAsset
{
	GENERATED_BODY()
	
public:

	// ---- Distance thresholds ----
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Distance")
	float A = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Distance")
	float B = 2000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Distance")
	float C = 4000.f;

	// ---- World volume mapping ----
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="WorldVolume", meta=(ClampMin="0.01", ClampMax="1.0"))
	float WorldMax = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="WorldVolume", meta=(ClampMin="0.01", ClampMax="1.0"))
	float WorldMin = 0.15f;
	
	// ---- Noise (B->C) ----
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Noise")
	USoundBase* NoiseSound = nullptr; 

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Noise", meta=(ClampMin="0.0", ClampMax="1.0"))
	float NoiseAtB = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Noise", meta=(ClampMin="0.0", ClampMax="1.0"))
	float NoiseAtC = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Noise")
	float NoiseFadeIn = 0.10f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Noise")
	float NoiseFadeOut = 0.10f;
	
	// ---- Runtime routing ----
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Routing")
	USoundMix* SMX_WorldDynamics = nullptr; 

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Routing")
	USoundClass* SC_WorldDynamics = nullptr; //SC_World (child di SC_SFX)

	// ---- Muffle via Submix override ----
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Muffle")
	USoundSubmix* SM_World = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Muffle")
	USoundEffectSubmixPreset* LPF_Preset_On = nullptr; // lowpass ON

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Muffle")
	float MuffleFadeTime = 0.10f;

	// ---- Update/smoothing ----
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Update")
	float UpdateInterval = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Update")
	float WorldInterpSpeed = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Update")
	float NoiseInterpSpeed = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Update")
	float VolumeEpsilon = 0.01f;
	
};
