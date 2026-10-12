#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraEffects/CameraEffectTypes.h"
#include "Engine/DataAsset.h"
#include "Interactions/Actors/Projectile.h"
#include "SoundWaveData.generated.h"

class UGameplayEffect;
class UForceFeedbackEffect;

UCLASS(NotBlueprintable)
class STILLHEAR_API USoundWaveData : public UDataAsset
{
	GENERATED_BODY()
	
#pragma region UPROPERTIES
public:
	UPROPERTY(EditAnywhere, Category = "SoundWave Settings|Camera Effects")
	FCameraEffectPreset ChargeEffectPreset;
	UPROPERTY(EditAnywhere, Category = "SoundWave Settings|Camera Effects")
	FCameraEffectPreset AimEffectPreset;
	UPROPERTY(EditAnywhere, Category = "SoundWave Settings|Camera Effects")
	FCameraEffectPreset ShootEffectPreset;
	
	UPROPERTY(EditDefaultsOnly, Category = "SoundWave Settings|Projectile")
	TSubclassOf<AProjectile> ProjectileClass;
	UPROPERTY(EditDefaultsOnly, Category = "SoundWave Settings|Projectile")
	float OffsetProjectileSpawn = 100.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "SoundWave Settings|Homing")
	bool bEnableHoming = true;
	// Distance below which the projectile will shoot perfectly straight
	UPROPERTY(EditDefaultsOnly, meta = (EditCondition = "bEnableHoming", EditConditionHides, ClampMin = 0.0f), Category = "SoundWave Settings|Homing")
	float MinHomingDistance = 300.0f;
	// Distance at which the projectile will use the maximum pitch offset for a full arc trajectory
	UPROPERTY(EditDefaultsOnly, meta = (EditCondition = "bEnableHoming", EditConditionHides, ClampMin = 1.0f), Category = "SoundWave Settings|Homing")
	float MaxHomingDistance = 1500.0f;
	// Maximum pitch angle applied when the target is further than the maximum homing distance
	UPROPERTY(EditDefaultsOnly, meta = (EditCondition = "bEnableHoming", EditConditionHides), Category = "SoundWave Settings|Homing")
	float MaxHomingPitchOffset = 45.0f;
	UPROPERTY(EditDefaultsOnly, meta = (EditCondition = "bEnableHoming", EditConditionHides, ClampMin = 0.0f), Category = "SoundWave Settings|Homing")
	float HomingAcceleration = 5000.0f;
	
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = 100.0f, UIMin = 100.0f), Category = "SoundWave Settings|Targeting")
	float TargetingRadius = 500.0f;
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = 0.0f, UIMin = 0.0f, ClampMax = 1.0f, UIMax = 1.0f), Category = "SoundWave Settings|Targeting")
	float TargetSwitchThreshold = 0.4f;
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = 0.0f, UIMin = 0.0f), Category = "SoundWave Settings|Targeting") 
	float TargetSwitchCooldown = 0.3f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SoundWave Settings|Effects")
	TSubclassOf<UGameplayEffect> ChargeEffectClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SoundWave Settings|Effects")
	TSubclassOf<UGameplayEffect> AimEffectClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SoundWave Settings|Effects")
	TSubclassOf<UGameplayEffect> SpeedEffectClass;
	
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = 0.0f, UIMin = 0.0f), Category = "SoundWave Settings")
	float AbilityCooldown = 5.0f;
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = 0.0f, UIMin = 0.0f), Category = "SoundWave Settings")
	float SpeedMultiplier = 0.5f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SoundWave Settings|Sounds")
	TObjectPtr<USoundBase> ChargeSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SoundWave Settings|Sounds")
	TObjectPtr<USoundBase> ShootSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SoundWave Settings|Sounds")
	TObjectPtr<USoundBase> AimSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SoundWave Settings|Sounds")
	TObjectPtr<USoundBase> FinishCooldownSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SoundWave Settings|Feedback")
	TObjectPtr<UForceFeedbackEffect> ActiveAimForceFeedback;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SoundWave Settings|Feedback")
	TObjectPtr<UForceFeedbackEffect> ShootForceFeedback;
	// ===== AI NOISE =====
	// Loudness of the noise reported to the AI hearing system when the sound wave is fired
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"), Category = "SoundWave Settings|AI Noise")
	float ShootNoiseLoudness = 1.0f;
	// Maximum range at which the AI can hear the shot (0 = uses the AI perception component range)
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0"), Category = "SoundWave Settings|AI Noise")
	float ShootNoiseRange = 0.0f;
#pragma endregion
};
