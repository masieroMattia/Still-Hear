#pragma once

#include "CoreMinimal.h"
#include "StillHearGameplayAbility.h"
#include "GA_Jump.generated.h"

class UForceFeedbackEffect;

UCLASS()
class STILLHEAR_API UGA_Jump : public UStillHearGameplayAbility
{
	GENERATED_BODY()

#pragma region UPROPERTY
private:
	UPROPERTY(EditDefaultsOnly, Category = "GameplayEffect")
	TSubclassOf<UGameplayEffect> AirStatusEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "Feedback")
	TObjectPtr<UForceFeedbackEffect> JumpForceFeedback;
#pragma endregion
	
#pragma region CONSTRUCTOR
public:
	UGA_Jump();
#pragma endregion

#pragma region UFUNCTIONS
private:
	UFUNCTION()
	void OnMovementModeChange(EMovementMode NewMode);
#pragma endregion
	
#pragma region METHODS
public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
#pragma endregion
};
