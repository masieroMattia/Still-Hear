// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySystem/Abilities/StillHearGameplayAbility.h"
#include "GA_WormRoar.generated.h"

/**
 * 
 */
UCLASS()
class STILLHEAR_API UGA_WormRoar : public UStillHearGameplayAbility
{
	GENERATED_BODY()

#pragma region UPROPERTY
public:
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> RoarMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	TSubclassOf<UGameplayEffect> RoarEffect;
#pragma endregion 
	
#pragma region CONSTRUCTOR
public:
	UGA_WormRoar();
#pragma endregion

#pragma region UFUNCTION
private:
	UFUNCTION()
	void OnMontageCompleted();
	UFUNCTION()
	void OnMontageInterrupted();
	UFUNCTION()
	void OnMontageCancelled();

	UFUNCTION()
	void ApplyRoarEffect(FGameplayEventData Payload);
#pragma endregion
	
#pragma region METHODS
public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;	
#pragma endregion 
};
