#pragma once
 
#include "CoreMinimal.h"
#include "Camera/CameraShakeBase.h"
#include "Curves/CurveFloat.h"
#include "CameraEffectTypes.generated.h"

class UForceFeedbackEffect;

// Wraps a UCameraShakeBase subclass + a runtime scale override
// If null the shake effect is skipped entirely
USTRUCT(BlueprintType)
struct FCameraShakeConfig
{
	GENERATED_BODY()

	// The shake class to play
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake")
	TSubclassOf<UCameraShakeBase> ShakeClass;

	// Multiplier applied on top of the shake class's own scale
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0"), Category = "Shake")
	float Scale = 1.0f;

	// The force feedback to play along with the camera shake
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake")
	TObjectPtr<UForceFeedbackEffect> ShakeForceFeedback;

	// When true, the force feedback will loop until the shake is stopped
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "ShakeForceFeedback != nullptr", EditConditionHides), Category = "Shake")
	bool bLoopForceFeedback = false;

	// When true the shake plays at the player location rather than as a global world shake
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake")
	bool bIsLocalShake = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "!bIsLocalShake", EditConditionHides), Category = "Shake")
	FVector Epicenter = FVector::ZeroVector;

	// Full intensity within this radius
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "!bIsLocalShake", EditConditionHides), Category = "Shake")
	float InnerRadius = 500.0f;

	// Zero intensity outside this radius
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "!bIsLocalShake", EditConditionHides), Category = "Shake")
	float OuterRadius = 2000.0f;

	// Falloff exponent (1.0 = linear)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "!bIsLocalShake", EditConditionHides), Category = "Shake")
	float Falloff = 1.0f;
};

// Drives a temporary FOV change using an optional curve
USTRUCT(BlueprintType)
struct FCameraFOVConfig
{
	GENERATED_BODY()

	// Delta added to (or subtracted from) the camera's current FOV
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FOV")
	float FOVDelta = 15.0f;

	// Optional curve. If null, a simple linear snap-and-return is used based on Duration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FOV")
	TObjectPtr<UCurveFloat> PulseCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FOV")
	bool bKeepFOV = false;
};

// Applies a temporary world-space positional offset to the camera
USTRUCT(BlueprintType)
struct FCameraOffsetConfig
{
	GENERATED_BODY()

	// Direction and magnitude of the initial kick
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Offset")
	FVector KickDirection = FVector(0.f, 0.f, 10.f);

	// How quickly the offset springs back to zero
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1"), Category = "Offset")
	float SpringSpeed = 8.0f;

	// Maximum lifetime of the offset effect
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Offset", meta = (ClampMin = "0.01", EditCondition = "!bKeepOffset", EditConditionHides))
	float MaxDuration = 0.8f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Offset")
	bool bKeepOffset = false;
};

// A self-contained bundle of camera effects
// Each sub-config is independently optional
USTRUCT(BlueprintType)
struct FCameraEffectPreset
{
	GENERATED_BODY()

	// ── Shake ─────────────────────────────────────────────────────────────
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects|Shake")
	bool bPlayShake = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bPlayShake", EditConditionHides), Category = "Effects|Shake")
	FCameraShakeConfig ShakeConfig;

	// ── FOV Pulse ─────────────────────────────────────────────────────────
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects|FOV")
	bool bPlayFOV = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite,	meta = (EditCondition = "bPlayFOV", EditConditionHides), Category = "Effects|FOV")
	FCameraFOVConfig FOVConfig;

	// ── Offset Pulse ──────────────────────────────────────────────────────
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects|Offset")
	bool bPlayOffset = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bPlayOffset", EditConditionHides), Category = "Effects|Offset")
	FCameraOffsetConfig OffsetConfig;
};
