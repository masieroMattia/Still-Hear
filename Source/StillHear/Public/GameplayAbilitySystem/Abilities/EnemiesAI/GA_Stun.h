#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySystem/Abilities/StillHearGameplayAbility.h"
#include "GA_Stun.generated.h"

class UStunData;

UCLASS()
class STILLHEAR_API UGA_Stun : public UStillHearGameplayAbility
{
	GENERATED_BODY()

#pragma region CONSTRUCTOR
public:
	UGA_Stun();
#pragma endregion

#pragma region UPROPERTIES
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Stun")
	TObjectPtr<UStunData> StunData;
#pragma endregion

#pragma region METHODS
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UFUNCTION()
	void OnStunDurationFinished();
#pragma endregion
};
