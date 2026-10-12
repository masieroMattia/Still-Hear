#pragma once

#include "CoreMinimal.h"
#include "CameraEffectTypes.h"
#include "Components/ActorComponent.h"
#include "CameraEffectsComponent.generated.h"

class UCameraOffsetModifier;
class UForceFeedbackEffect;

struct FCameraOffsetPulseConfig;
struct FCameraFOVPulseConfig;
struct FCameraEffectPreset;
struct FCameraShakeConfig;

struct FActiveFOVPulse
{
	FCameraFOVConfig Config;
	float Alpha = 0.0f;
};

struct FActiveOffsetPulse
{
	FCameraOffsetConfig Config;
	FVector CurrentOffset = FVector::ZeroVector;
	float ElapsedTime     = 0.0f;
};

UCLASS(ClassGroup=(Camera), meta=(BlueprintSpawnableComponent))
class STILLHEAR_API UCameraEffectsComponent : public UActorComponent
{
	GENERATED_BODY()

#pragma region CONSTRUCTOR
public:
	UCameraEffectsComponent();
#pragma endregion

#pragma region VARIABLES
private:
	// ── Shake state ──────────────────────────────────────────────
	UPROPERTY()
	TObjectPtr<UCameraShakeBase> ActiveLoopingShake;

	UPROPERTY()
	TObjectPtr<UForceFeedbackEffect> ActiveShakeForceFeedback;

	FTimerHandle ForceFeedbackTimerHandle;
	
	// ── FOV state ────────────────────────────────────────────────
	// Pulse — one-shot pulse
	bool bFOVPulseActive = false; // True while a FOV pulse is in progress
	TArray<FActiveFOVPulse> ActiveFOVPulses;

	// Sustained — bKeepFOV, persists until StopAllEffects
	bool bSustainedFOVActive = false;
	float SustainedFOVDelta = 0.0f;
	float SustainedFOVAlpha = 0.0f;
	FCameraFOVConfig ActiveSustainedFOVConfig;

	// ── Offset state ─────────────────────────────────────────────
	bool bOffsetPulseActive = false; // True while an offset pulse is running
	TArray<FActiveOffsetPulse> ActiveOffsetPulses;

	// Sustained — bKeepOffset, persists until StopAllEffects
	bool bSustainedOffsetActive = false;
	FVector SustainedOffset = FVector::ZeroVector;
	FVector CurrentSustainedOffset = FVector::ZeroVector;
	FCameraOffsetConfig ActiveSustainedOffsetConfig;
	
	// Modifier used to safely alter the FOV without overwriting the base camera value
	TWeakObjectPtr<class UCameraFOVModifier> FOVModifier;
	// Persistent camera modifier used to apply the offset pulse effect
	TWeakObjectPtr<UCameraOffsetModifier> OffsetModifier;
#pragma endregion
	
#pragma region METHODS
protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
private:
	APlayerCameraManager* GetCameraManager() const;
	APlayerController* GetOwnerController() const;

	void TickFOV(float DeltaTime);
	void TickOffset(float DeltaTime);
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
#pragma endregion
	
#pragma region UFUNCTIONS
public:
	// Play all enabled layers in the preset
	UFUNCTION(BlueprintCallable, Category = "CameraEffects")
	void PlayEffectPreset(const FCameraEffectPreset& Preset, FVector CustomEpicenter = FVector::ZeroVector);
	// Trigger only the camera shake
	UFUNCTION(BlueprintCallable, Category = "CameraEffects")
	void PlayCameraShake(const FCameraShakeConfig& Config, FVector CustomEpicenter = FVector::ZeroVector);
	// Trigger only the FOV pulse. Interrupts any running FOV pulse
	UFUNCTION(BlueprintCallable, Category = "CameraEffects")
	void PlayFOV(const FCameraFOVConfig& Config);
	// Trigger only the positional offset pulse. Interrupts any running offset
	UFUNCTION(BlueprintCallable, Category = "CameraEffects")
	void PlayOffset(const FCameraOffsetConfig& Config);
	// Immediately stop all running effects and restore the camera
	UFUNCTION(BlueprintCallable, Category = "CameraEffects")
	void StopLoopingShake();
	UFUNCTION(BlueprintCallable, Category = "CameraEffects")
	void StopAllEffects();
#pragma endregion
};