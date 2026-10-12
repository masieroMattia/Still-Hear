#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractableShakeComponent.generated.h"

// Vibration modes
UENUM(BlueprintType)
enum class EInteractableShakePattern : uint8
{
	Wild,           // Organic, multi-axis non-repetitive pattern
	Mechanical,     // Rhythmic and synchronized
	Sway,           // Smooth, slower swaying movement
	Jitter,         // High-frequency, tight erratic vibrations
	VerticalOnly,   // Movement restricted primarily to the Z axis
	HorizontalOnly, // Movement restricted primarily to the X and Y axes
	Pendulum        // Simulates a swinging pendulum/bell (smooth arc)
};

// Core settings for a shake effect
USTRUCT(BlueprintType)
struct FInteractableShakeParams
{
	GENERATED_BODY()

#pragma region UPROPERTIES
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EInteractableShakePattern Pattern = EInteractableShakePattern::Wild;

	// Maximum intensity of the shake
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0"))
	float Amplitude = 1.0f;

	// Speed of the shake
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0"))
	float Frequency = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShakeLocation = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShakeRotation = false;

	// Maximum rotation angle (in degrees) the component can tilt
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bShakeRotation", ClampMin = "0.0"))
	float RotationAmplitude = 2.0f;

	// If empty, the component will try to find a MeshComponent to shake
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target")
	TArray<FComponentReference> TargetComponents;
#pragma endregion
};

// Shake settings that include a duration
USTRUCT(BlueprintType)
struct FTimedInteractableShakeParams
{
	GENERATED_BODY()

#pragma region UPROPERTIES
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FInteractableShakeParams Settings;

	// How long this shake phase lasts
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0"))
	float Duration = 0.5f;
#pragma endregion
};

// Internal state tracking for a shaken component to ensure clean resets
USTRUCT()
struct FComponentShakeState
{
	GENERATED_BODY()

	FVector LastOffset = FVector::ZeroVector;
	FQuat LastRotation = FQuat::Identity;
};

/**
 * Component that adds a configurable procedural shake effect
 * Allows defining different shake intensities and target components for different phases of interaction
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class STILLHEAR_API UInteractableShakeComponent : public UActorComponent
{
	GENERATED_BODY()

#pragma region UPROPERTIES
public:
	// Constant shake that happens regardless of interaction state
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Phases")
	FInteractableShakeParams AlwaysShake;

	// Shake that happens when the player is within the interaction range
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Phases")
	FInteractableShakeParams InRangeShake;

	// Shake that happens after the player interacts, but before the logic starts
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Phases")
	FTimedInteractableShakeParams PreInteractionShake;

	// Shake that happens while the interaction is actively ongoing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Phases")
	FInteractableShakeParams InteractingShake;

	// Shake that happens for a duration after the interaction has finished
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Phases")
	FTimedInteractableShakeParams AfterShake;

	// Default component to shake if a phase has no specific targets. If null, tries to find a mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
	FComponentReference DefaultTargetComponent;

	// How fast the shake transitions between different phases or intensities
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings", meta = (ClampMin = "0.1"))
	float BlendSpeed = 8.0f;
#pragma endregion

#pragma region VARIABLES
private:
	bool bIsInRange = false;
	bool bIsPreInteracting = false;
	bool bIsInteracting = false;
	
	float AfterShakeTimer = 0.0f;
	float TimeAccumulator = 0.0f;

	UPROPERTY()
	TObjectPtr<USceneComponent> ResolvedDefaultTarget;

	UPROPERTY()
	TMap<TObjectPtr<USceneComponent>, FComponentShakeState> TrackedComponents;

	// Components that must never be shaken
	UPROPERTY()
	TSet<TObjectPtr<USceneComponent>> BlacklistedComponents;

	// Blended state variables
	float BlendedAmplitude = 0.0f;
	float BlendedFrequency = 0.0f;
	float BlendedRotationAmplitude = 0.0f;
	float BlendedFreqY = 1.0f;
	float BlendedFreqZ = 1.0f;
	float BlendedPhaseY = 0.0f;
	float BlendedPhaseZ = 0.0f;
	FVector BlendedAxisMult = FVector::OneVector;

	// Cached target components for each phase
	UPROPERTY()
	TArray<TObjectPtr<USceneComponent>> AlwaysShakeTargets;

	UPROPERTY()
	TArray<TObjectPtr<USceneComponent>> InRangeShakeTargets;

	UPROPERTY()
	TArray<TObjectPtr<USceneComponent>> PreInteractionShakeTargets;

	UPROPERTY()
	TArray<TObjectPtr<USceneComponent>> InteractingShakeTargets;

	UPROPERTY()
	TArray<TObjectPtr<USceneComponent>> AfterShakeTargets;

	UPROPERTY()
	TArray<TObjectPtr<USceneComponent>> DefaultShakeTargets;
#pragma endregion

#pragma region CONSTRUCTOR
public:
	UInteractableShakeComponent();
#pragma endregion

#pragma region METHODS
private:
	// Internal struct for mapping active shake configurations to target simulation values
	struct FShakeTargetState
	{
		float Amplitude = 0.0f;
		float Frequency = 0.0f;
		float RotationAmplitude = 0.0f;
		float FreqY = 0.0f;
		float FreqZ = 0.0f;
		float PhaseY = 0.0f;
		float PhaseZ = 0.0f;
		FVector AxisMult = FVector::OneVector;
	};

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	// Target resolution & caching
	void ResolveDefaultTarget();
	void CacheAllTargets();
	void CacheTargetsForParams(const FInteractableShakeParams* Params, TArray<TObjectPtr<USceneComponent>>& OutTargets);
	const TArray<TObjectPtr<USceneComponent>>* GetCachedTargets(const FInteractableShakeParams* Params) const;

	// Tick updates
	void UpdateTimers(float DeltaTime);
	FShakeTargetState CalculateTargetState(const FInteractableShakeParams* Params) const;
	void BlendCurrentState(const FShakeTargetState& TargetState, float DeltaTime);
	void UpdateShakeEffect(float DeltaTime, const FInteractableShakeParams* ActiveParams);

	void ApplyShake(float DeltaTime, const FInteractableShakeParams* Params);
	void ResetAllComponentTransforms();
	
	const FInteractableShakeParams* GetCurrentActiveParams() const;

public:
	float GetPreInteractionDuration() const { return PreInteractionShake.Duration; }
	bool ShouldPreInteractionShake() const { return PreInteractionShake.Settings.bEnabled; }
	bool IsInteracting() const { return bIsInteracting; }
	bool IsPreInteracting() const { return bIsPreInteracting; }
#pragma endregion

#pragma region UFUNCTIONS
public:
	UFUNCTION(BlueprintCallable, Category = "Shake")
	void SetInRange(bool bInRange);

	UFUNCTION(BlueprintCallable, Category = "Shake")
	void SetPreInteracting(bool bPreInteracting);

	UFUNCTION(BlueprintCallable, Category = "Shake")
	void SetInteracting(bool bInteracting);

	UFUNCTION(BlueprintCallable, Category = "Shake")
	void PlayAfterShake();

	UFUNCTION(BlueprintCallable, Category = "Shake")
	void StopAllShakes();

	/**
	 * Permanently removes a component from any shake phase:
	 * undoes any current offset on it and blacklists it so it will never be shaken again
	 */
	UFUNCTION(BlueprintCallable, Category = "Shake")
	void StopShakingComponent(USceneComponent* Component);

	UFUNCTION(BlueprintCallable, Category = "Shake")
	FVector GetShakeOffsetForComponent(USceneComponent* Component) const;

	UFUNCTION(BlueprintCallable, Category = "Shake")
	FRotator GetShakeRotationForComponent(USceneComponent* Component) const;
#pragma endregion
};
