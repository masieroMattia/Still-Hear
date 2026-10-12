# pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySystem/Abilities/StillHearGameplayAbility.h"
#include "GA_MantisShift.generated.h"

class UMantisShiftData;
class AAIMantisCharacter;

/**
 * Handles Mantis shifting (teleportation), used both for navigational links and gap-closing attacks
 */
UCLASS()
class STILLHEAR_API UGA_MantisShift : public UStillHearGameplayAbility
{
	GENERATED_BODY()

#pragma region UPROPERTIES
protected:
	// Data defining how this specific shift behaves (timings, attack to trigger, cues). Setup from child blueprint
	UPROPERTY(EditDefaultsOnly, Category = "Shift Data")
	TObjectPtr<UMantisShiftData> ShiftData;
#pragma endregion
	
#pragma region VARIABLES
	TWeakObjectPtr<AAIMantisCharacter> CachedMantis;
	TWeakObjectPtr<AActor> CurrentTarget;

	FTimerHandle PhaseOutTimerHandle;
	FTimerHandle TeleportTimerHandle;
	FTimerHandle TranslationTimerHandle;
	FTimerHandle FadeTimerHandle;

	FVector ShiftStartingLocation;
	FVector LandingTarget = FVector::ZeroVector;
	FRotator ShiftStartingRotation;
	// Final facing for Nav Traversal landings, taken from the destination transform's rotation
	FRotator LandingRotation = FRotator::ZeroRotator;

	float TranslationElapsedTime;
	float FadeElapsedTime = 0.0f;
	bool bIsNavTraversal = false;

	// If set, the AI fully resets its state and is forced into Hunting the moment it lands (used for Chaos-Shift teleports)
	bool bForceHuntOnLanding = false;
	TWeakObjectPtr<AActor> ForceHuntTargetActor;
# pragma endregion

#pragma region CONSTRUCTOR
public:
	UGA_MantisShift();
#pragma endregion

#pragma region METHODS
public:
	// Returns the parsed shift configuration data for the current execution
	UMantisShiftData* GetShiftData() const { return ShiftData; }

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
    // Called when the initial 'fade' completes. Physically teleports the AI and schedules the fade in
    void OnFadeOutComplete();
    // Ticks during the quick translation move
    void TranslateToTargetTick();
    // Called when the final 'fade' completes and restores regular behaviors or executes a follow-up attack
    void OnFadeInComplete();
    
    void FadeOutTick();
    void OnTranslationComplete();
    void FadeInTick();

	// Validates and determines the exact landing impact point on the navmesh
	bool ComputeLandingTarget(const FVector& BaseTarget);
	// If configured, predicts where the target will be based on their velocity
	FVector PredictTargetLocation(const FVector& TargetLoc) const;

	// Modifies collision profiles while shifting to avoid getting stuck during teleport
	void DisableAICollision() const;
	// Restores default collision profiles
	void RestoreAICollision() const;
#pragma endregion
};

