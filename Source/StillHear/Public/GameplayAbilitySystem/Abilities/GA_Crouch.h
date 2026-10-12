#pragma once

#include "CoreMinimal.h"
#include "StillHearGameplayAbility.h"
#include "Data/DataAssets/CrouchData.h"
#include "GA_Crouch.generated.h"

UCLASS()
class STILLHEAR_API UGA_Crouch : public UStillHearGameplayAbility
{
	GENERATED_BODY()

#pragma region UPROPERTY
private:
	UPROPERTY(EditDefaultsOnly, Category = "Data")
	TObjectPtr<UCrouchData> AbilityData;
#pragma endregion
	
#pragma region CONSTRUCTOR
public:
	UGA_Crouch();
#pragma endregion

#pragma region UFUNCTIONS
private:
	// Called on crouch input release: requests the character to stand up
	UFUNCTION()
	void OnUncrouchPressed(FGameplayEventData Payload);
#pragma endregion
	
#pragma region METHODS
public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
#pragma endregion
};
