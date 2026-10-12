#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Camera/CameraEffects/CameraEffectTypes.h"
#include "ResonanceData.generated.h"

class UGameplayEffect;
class UForceFeedbackEffect;

UCLASS()
class STILLHEAR_API UResonanceData : public UDataAsset
{
	GENERATED_BODY()
	
#pragma region UPROPERTIES
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance Settings")
	FGameplayTagContainer ResonanceTags = FGameplayTagContainer();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance Settings")
	TSubclassOf<UGameplayEffect> ResonanceEffectClass;
	
	UPROPERTY(EditAnywhere, Category = "Resonance Settings|Camera Effects")
	FCameraEffectPreset EntryEffectPreset;
	UPROPERTY(EditAnywhere, Category = "Resonance Settings|Camera Effects")
	FCameraEffectPreset Phase2EffectPreset;
	UPROPERTY(EditAnywhere, Category = "Resonance Settings|Camera Effects")
	FCameraEffectPreset SuccessEffectPreset;
	UPROPERTY(EditAnywhere, Category = "Resonance Settings|Camera Effects")
	FCameraEffectPreset InterruptEffectPreset;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance Settings|Config")
	float ResonanceRadius = 200.0f;
	UPROPERTY(EditAnywhere, Category = "Resonance Settings|Config")
	float MaxHeight = 50.0f;
	UPROPERTY(EditAnywhere, Category = "Resonance Settings|Config")
	float MatchThreshold = 15.0f;
	UPROPERTY(EditAnywhere, Category = "Resonance Settings|Config")
	float SpeedMultiplier = 2.0f;
	UPROPERTY(EditAnywhere, Category = "Resonance Settings|Config")
	float ResetSpeed = 15.0f;
	UPROPERTY(EditAnywhere, Category = "Resonance Settings|Config")
	TObjectPtr<UCurveFloat> ResonanceCurve;
	
	UPROPERTY(EditAnywhere, Category = "Resonance Settings|VFX")
	TObjectPtr<class UNiagaraSystem> Phase1VFX;
	UPROPERTY(EditAnywhere, Category = "Resonance Settings|VFX")
	TObjectPtr<class UNiagaraSystem> Phase2VFX;
	UPROPERTY(EditAnywhere, Category = "Resonance Settings|VFX")
	TObjectPtr<class UNiagaraSystem> InterruptVFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance Settings|Sounds")
	TObjectPtr<USoundBase> EntryResonanceSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance Settings|Sounds")
	TObjectPtr<USoundBase> ActiveResonanceSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance Settings|Sounds")
	TObjectPtr<USoundBase> CollideResonanceSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance Settings|Sounds")
	TObjectPtr<USoundBase> Phase2ResonanceSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance Settings|Sounds")
	TObjectPtr<USoundBase> SuccessResonanceSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance Settings|Sounds")
	TObjectPtr<USoundBase> FailResonanceSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance Settings|Sounds")
	TObjectPtr<USoundBase> InterruptResonanceSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance Settings|Feedback")
	TObjectPtr<UForceFeedbackEffect> ActiveResonanceForceFeedback;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance Settings|Feedback")
	TObjectPtr<UForceFeedbackEffect> InteractResonanceForceFeedback;
#pragma endregion
};
