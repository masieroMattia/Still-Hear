#pragma once

#include "CoreMinimal.h"
#include "StillHearGameplayAbility.h"
#include "GA_Sprint.generated.h"

class AStillHearMainCharacter;

UCLASS()
class STILLHEAR_API UGA_Sprint : public UStillHearGameplayAbility
{
	GENERATED_BODY()
	
#pragma region UPROPERTY
	/*
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BlendSpace")
	TObjectPtr<class UBlendSpace> SprintBlendSpace;
	*/

private:
	UPROPERTY(EditDefaultsOnly, Category = "GameplayEffect")
	TSubclassOf<UGameplayEffect> SprintSpeedMultiplierEffectClass;
#pragma endregion
	
#pragma region CONSTRUCTOR
public:
	UGA_Sprint();
#pragma endregion

#pragma region UFUNCTIONS
private:
	UFUNCTION()
	void OnInputRelease(FGameplayEventData Payload);
#pragma endregion
	
#pragma region METHODS
public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	void ApplySpeedMultiplierToOwner(const AStillHearMainCharacter* Character, float Multiplier) const;
#pragma endregion
};
