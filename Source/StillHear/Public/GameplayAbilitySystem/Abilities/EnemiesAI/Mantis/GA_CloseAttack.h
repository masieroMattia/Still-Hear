#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySystem/Abilities/EnemiesAI/Mantis/GA_AttackBase.h"
#include "GA_CloseAttack.generated.h"

UCLASS()
class STILLHEAR_API UGA_CloseAttack : public UGA_AttackBase
{
	GENERATED_BODY()

#pragma region CONSTRUCTORS
public:
	UGA_CloseAttack();
#pragma endregion

#pragma region UFUNCTIONS
	UFUNCTION()
	void OnMontageCompleted();
	UFUNCTION()
	void OnMontageCancelled();
	UFUNCTION()
	void TrackTargetTick();
	UFUNCTION()
	void OnSafetyTimeout();
#pragma endregion

#pragma region VARIABLES
private:
	FTimerHandle TrackingTimerHandle;
	FTimerHandle SafetyTimerHandle;
#pragma endregion
	
#pragma region METHODS
public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
#pragma endregion
};