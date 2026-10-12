#pragma once

#include "CoreMinimal.h"
#include "StillHearGameplayAbility.h"
#include "GA_LowGetOnTop.generated.h"

UCLASS()
class STILLHEAR_API UGA_LowGetOnTop : public UStillHearGameplayAbility
{
	GENERATED_BODY()

#pragma region UPROPERTY
private:
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> LowGetOnTopMontage;
#pragma endregion
	
#pragma region CONSTRUCTOR
public:
	UGA_LowGetOnTop();
#pragma endregion

#pragma region UFUNCTIONS
private:
	UFUNCTION()
	void OnMontageCompleted();
	UFUNCTION()
	void OnMontageInterrupted();
	UFUNCTION()
	void OnMontageCancelled();
#pragma endregion
	
#pragma region METHODS
public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
#pragma endregion
};
