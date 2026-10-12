#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySystem/Abilities/StillHearGameplayAbility.h"
#include "GA_Death.generated.h"

class UForceFeedbackEffect;

UCLASS()
class STILLHEAR_API UGA_Death : public UStillHearGameplayAbility
{
	GENERATED_BODY()

#pragma region CONSTRUCTOR
	UGA_Death();
#pragma endregion
	
#pragma region UPROPERTY
private:
	UPROPERTY(EditDefaultsOnly, Category = "GameplayEffect")
	TSubclassOf<UGameplayEffect> DeathEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "Feedback")
	TObjectPtr<UForceFeedbackEffect> DeathForceFeedback;
#pragma endregion
	
#pragma region METHODS
public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
#pragma endregion
};
