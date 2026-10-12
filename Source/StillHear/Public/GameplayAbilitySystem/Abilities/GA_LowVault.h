#pragma once

#include "CoreMinimal.h"
#include "StillHearGameplayAbility.h"
#include "GA_LowVault.generated.h"

UCLASS()
class STILLHEAR_API UGA_LowVault : public UStillHearGameplayAbility
{
	GENERATED_BODY()

#pragma region UPROPERTY
private:
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> VaultMontage;
#pragma endregion
	
#pragma region CONSTRUCTOR
public:
	UGA_LowVault();
#pragma endregion

#pragma region UFUNCTIONS
private:
	UFUNCTION()
	void OnMontageCompleted();
	UFUNCTION()
	void OnMontageInterrupted();
	UFUNCTION()
	void OnMontageCancelled();

	UFUNCTION()
	void OnCollisionActivateEventReceived(FGameplayEventData Payload);
#pragma endregion
	
#pragma region METHODS
public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
#pragma endregion
};
