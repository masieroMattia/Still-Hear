#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySystem/Abilities/StillHearGameplayAbility.h"
#include "GA_SoundWave.generated.h"

class UFloatingCompanionComponent;
class USoundWaveData;
class UCameraEffectsComponent;
class UAbilityTask_WaitGameplayEvent;

UCLASS()
class STILLHEAR_API UGA_SoundWave : public UStillHearGameplayAbility
{
	GENERATED_BODY()
	
#pragma region UPROPERTIES
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SoundWave Settings")
	TObjectPtr<USoundWaveData> SoundWaveData;
#pragma endregion
	
#pragma region VARIABLES
protected:
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> WaitSwitchEvent;
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> WaitShootEvent;
	UPROPERTY()
	TArray<TObjectPtr<AActor>> AvailableTargets;
	UPROPERTY()
	TObjectPtr<APlayerController> CachedPlayerController;
	UPROPERTY()
	TObjectPtr<UCameraEffectsComponent> CachedCameraEffectComponent;
	UPROPERTY()
	TObjectPtr<UFloatingCompanionComponent> CachedCompanionComponent;
	
	FActiveGameplayEffectHandle CurrentSpeedEffectHandle;
	FTimerHandle UpdateTargetsTimerHandle;
	
	int32 CurrentTargetIndex = 0;
	float LastSwitchTime = 0.0f;
#pragma endregion 
	
#pragma region CONSTRUCTOR
	UGA_SoundWave();
#pragma endregion
	
#pragma region METHODS
public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) override;
protected:
	void ApplySpeedMultiplierToSelf(float Multiplier);
	virtual void ApplyEffectWithSetByCaller(TSubclassOf<UGameplayEffect> EffectClass, const FGameplayTag& DataTag, float Magnitude) const;
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	
private:
	void SearchForTargets();
	TArray<AActor*> GetValidTargetsOnScreen() const;
	bool GetScreenPosition(FVector2D& OutScreenPosition, const FVector& WorldLocation) const;
	void RemoveChargeAndAimEffects() const;
#pragma endregion
	
#pragma region UFUNCTIONS
protected:
	UFUNCTION()
	void OnSwitchTargetEventReceived(FGameplayEventData Payload);
	UFUNCTION()
	void OnShootEventReceived(FGameplayEventData Payload);
	UFUNCTION()
	void UpdateTargets();
#pragma endregion
};
