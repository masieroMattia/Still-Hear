#pragma once

#include "CoreMinimal.h"
#include "ResonanceManagerComponent.generated.h"

class UNiagaraSystem;
class UResonanceData;
class UNiagaraComponent;

// Static constants for Niagara Parameter Names
struct FResonanceParamNames
{
	static const FName HeightTop;
	static const FName HeightBottom;
	static const FName TriggerShockwave;
};

UENUM(BlueprintType)
enum class EResonancePhase : uint8
{
	Inactive,
	Phase1,  // Ping-pong movement
	Phase2,  // Fast one-shot
	Success,
	Reset
};

// Declare a dynamic multicast delegate for success
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnResonanceSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnResonanceInterrupted);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class STILLHEAR_API UResonanceManagerComponent : public UActorComponent
{
	GENERATED_BODY()

#pragma region UPROPERTY
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance Settings")
	TObjectPtr<UResonanceData> ResonanceData;
#pragma endregion
	
#pragma region VARIABLES
	UPROPERTY()
	TObjectPtr<UNiagaraComponent> Phase1VFX;
	UPROPERTY()
	TObjectPtr<UNiagaraComponent> Phase2VFX;
	UPROPERTY()
	TObjectPtr<UNiagaraComponent> CurrentActiveVFX;
	UPROPERTY()
	TObjectPtr<UAudioComponent> SpawnedSound;
	
	EResonancePhase CurrentPhase = EResonancePhase::Inactive;

	float CurrentHeight;
	float CurrentTargetHeight = 0.0f; // Used for Phase 1 ping-pong movement, toggled between MaxHeight and -MaxHeight
	float PhaseElapsedTime = 0.0f;
	float CurrentPhaseDuration = 1.0f;
	bool bInThreshold = false;
	bool bPhase2Unlocked = false;
	bool bSuccessful = false;
	bool bFailure = false;
#pragma endregion
	
	
#pragma region EVENTS
	UPROPERTY()
	FOnResonanceSuccess OnResonanceSuccess;
	UPROPERTY()
	FOnResonanceInterrupted OnResonanceInterrupted;
#pragma endregion
	
#pragma region CONSTRUCTOR
	UResonanceManagerComponent();
#pragma endregion
	
#pragma region METHODS
	void StartResonance();
	void AttemptMatch();
	void StopResonance();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void HandlePhaseTransition(EResonancePhase NewPhase);
	void HandleInterruption();
	void UpdateLogic(float DeltaTime);
	void UpdateVFXParameters() const;
	void UpdateSoundState();
#pragma endregion
	
#pragma region GETTERS
public:
	float GetCurrentCirclesDistance() const;
	EResonancePhase GetCurrentPhase() const;
	
private:
	UNiagaraComponent* GetOrCreateVFX(TObjectPtr<UNiagaraSystem> System, TObjectPtr<UNiagaraComponent>& Field);
#pragma endregion
};