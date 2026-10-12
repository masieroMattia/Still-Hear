#pragma once

#include "CoreMinimal.h"
#include "StillHearGameplayAbility.h"
#include "GA_Resonance.generated.h"

class UResonanceManagerComponent;
class UCameraEffectsComponent;
class UResonanceData;

UCLASS()
class STILLHEAR_API UGA_Resonance : public UStillHearGameplayAbility
{
	GENERATED_BODY()
	
#pragma region UPROPERTIES
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance Settings")
	TObjectPtr<UResonanceData> ResonanceData;
#pragma endregion

#pragma region VARIABLES
	UPROPERTY()
	TObjectPtr<UResonanceManagerComponent> ResonanceManager;
	UPROPERTY()
	TArray<TObjectPtr<AActor>> CurrentResonanceObjects;
	UPROPERTY()
	TObjectPtr<APlayerController> CachedPlayerController;
	UPROPERTY()
	TObjectPtr<UCameraEffectsComponent> CachedCameraEffectComponent;
#pragma endregion
	
#pragma region CONSTRUCTOR
	UGA_Resonance();
#pragma endregion

#pragma region METHODS
public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	// Scans the area for resonance objects
	void FindValidResonanceObjects();
#pragma endregion

#pragma region UFUNCTIONS
protected:
	UFUNCTION()
	virtual void OnMatchInputReceived(FGameplayEventData Payload);
	UFUNCTION()
	virtual void OnResonanceSuccess();
	UFUNCTION()
	virtual void OnResonanceInterrupted();
	UFUNCTION()
	virtual void OnStopEventReceived(FGameplayEventData Payload);
#pragma endregion
};
