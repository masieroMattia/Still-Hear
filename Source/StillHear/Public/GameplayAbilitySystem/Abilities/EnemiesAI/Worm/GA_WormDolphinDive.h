// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySystem/Abilities/StillHearGameplayAbility.h"
#include "GA_WormDolphinDive.generated.h"

/**
 * 
 */
UCLASS()
class STILLHEAR_API UGA_WormDolphinDive : public UStillHearGameplayAbility
{
	GENERATED_BODY()

#pragma region VARIABLES
private:
	FDelegateHandle StunTagDelegateHandle;
#pragma endregion 
	
#pragma region CONSTRUCTOR
public:
	UGA_WormDolphinDive();
#pragma endregion

#pragma region UFUNCTIONS
private:
	UFUNCTION()
	void OnMovementModeChange(EMovementMode NewMode);
	UFUNCTION()
	void OnStunTagAdded(const FGameplayTag Tag, int32 NewCount);
#pragma endregion
	
#pragma region METHODS
public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
#pragma endregion 
};
