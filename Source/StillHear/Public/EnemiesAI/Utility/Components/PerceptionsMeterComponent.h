#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemiesAI/Controllers/Mantis/AIMantisController.h"
#include "PerceptionsMeterComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPerceptionsUpdated, float, Awareness, float, Alert);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class STILLHEAR_API UPerceptionsMeterComponent : public UActorComponent
{
	GENERATED_BODY()

#pragma region EVENTS
public:
	UPROPERTY(BlueprintAssignable, Category = "AI Perception|Events")
	FOnPerceptionsUpdated OnPerceptionsUpdated;
#pragma endregion
	
#pragma region VARIABLES
	UPROPERTY()
	TObjectPtr<AStillHearAIControllerBase> AICRef;

	// Timer Handles
	FTimerHandle AwarenessPauseTimerHandle;
	FTimerHandle AwarenessDecreaseTimerHandle;
	FTimerHandle AlertPauseTimerHandle;
	FTimerHandle AlertDecreaseTimerHandle;

	// Awareness Variables
	float CurrentAwarenessValue;
	float MaxAwarenessValue;
	float AwarenessDecreaseValue;
	float AwarenessDecreaseTime;
	float AwarenessPauseTime;
	float AwarenessIncreaseOnHearing_Walk;
	float AwarenessIncreaseOnHearing_Run;
	float AwarenessIncreaseOnHearing_Crouch;
	float AwarenessIncreaseOnHearing_Repeater;
	float AwarenessIncreaseOnSight_Narrow;
	float AwarenessIncreaseOnSight_Wide;
	float AwarenessIncreaseOnSight_Peripheral;
	float AwarenessIncreaseOnSight_Backward;
    
	bool bIsDecreasingAwareness;
	bool bCanUpdateAwareness;

	// Alert Variables
	float CurrentAlertValue;
	float MaxAlertValue;
	float AlertDecreaseValue;
	float AlertDecreaseTime;
	float AlertPauseTime;
	float AlertIncreaseOnSight_Narrow;
	float AlertIncreaseOnSight_Wide;
	float AlertIncreaseOnSight_Peripheral;
	float AlertIncreaseOnSight_Backward;
    
	bool bIsDecreasingAlert;
	bool bCanUpdateAlert;
#pragma endregion
	
#pragma region CONSTRUCTOR
public:
	UPerceptionsMeterComponent();
#pragma endregion
	
#pragma region METHODS
public:
	virtual void InitializeComponent() override;
	
	// Initializes all values from the Data Asset
    void SetupPerceptionsMeterValues();
	void BroadcastPerceptionValues() const;
	
    // Centralized function to handle incoming sensory data
    void ProcessSensoryInput(E_AISense InputSense, E_AISightCone CurrentTypeOfCone = E_AISightCone::NONE, E_AIHearingType HearingType = E_AIHearingType::NONE);
	void ProcessContinuousSight(const E_AISightCone CurrentTypeOfCone, const float DeltaTime);
	
    // Awareness public controls
    void ResetAwareness();
    void StopAwareness();
    float GetCurrentAwareness() const { return CurrentAwarenessValue; }
    float GetMaxAwareness() const { return MaxAwarenessValue; }
    bool GetCanUpdateAwareness() const { return bCanUpdateAwareness; }
    void SetCanUpdateAwareness(const bool bNewStatus) { bCanUpdateAwareness = bNewStatus; }

    // Forces awareness to max and transitions to SUSPICIOUS. Used by group propagation
    void ForceAwarenessToMax();

    // Alert public controls
    void ResetAlert(bool bResetAwareness = true);
    void StopAlert();
    float GetCurrentAlert() const { return CurrentAlertValue; }
    float GetMaxAlert() const { return MaxAlertValue; }
    bool GetCanUpdateAlert() const { return bCanUpdateAlert; }
    void SetCanUpdateAlert(const bool bNewStatus) { bCanUpdateAlert = bNewStatus; }

    // Forces alert to max and transitions to ALERTED. Used by group propagation
    void ForceAlertToMax();
	
	void ResetAll();
protected:
    virtual void BeginPlay() override;

    // Internal Awareness handlers
    void UpdateAwareness(float DeltaValue);
    void OnAwarenessPauseFinished();
    void OnDecreaseAwareness();
    float GetCorrectSightAwareness(E_AISightCone SightConeType) const;
    float GetCorrectHearingAwareness(E_AIHearingType HearingType) const;

    // Internal Alert handlers
    void UpdateAlert(float DeltaValue, bool bIsFromTouching = false);
    void OnAlertPauseFinished();
    void OnDecreaseAlert();
    float GetCorrectSightAlert(E_AISightCone SightConeType) const;
#pragma endregion
};
