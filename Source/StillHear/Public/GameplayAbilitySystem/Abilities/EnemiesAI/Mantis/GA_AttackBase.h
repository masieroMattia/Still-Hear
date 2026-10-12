#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySystem/Abilities/StillHearGameplayAbility.h"
#include "GA_AttackBase.generated.h"

class AAIMantisCharacter;
class UAttackBaseData;
class UShapeComponent;

UCLASS(Abstract)
class STILLHEAR_API UGA_AttackBase : public UStillHearGameplayAbility
{
	GENERATED_BODY()
	
#pragma region UPROPERTIES
protected:
	// Data Asset containing shared attack parameters (hit effect, hit box tag, cooldown, distances)
	UPROPERTY(EditDefaultsOnly, Category = "Attack Data")
	TObjectPtr<UAttackBaseData> AttackData;
#pragma endregion
	
#pragma region VARIABLES
protected:
	TWeakObjectPtr<AAIMantisCharacter> CachedMantis;
	TWeakObjectPtr<AActor> CurrentTarget;
	bool bHasHitThisSwing = false;
#pragma endregion
	
#pragma region CONSTRUCTOR
public:
	UGA_AttackBase();
#pragma endregion
	
#pragma region UFUNCTIONS
protected:
	UFUNCTION()
	void OnHitBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnStunTagAdded(const FGameplayTag Tag, int32 NewCount);
#pragma endregion

#pragma region METHODS
public:
	// Returns the attack data asset
	const UAttackBaseData* GetAttackData() const { return AttackData; }
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	void BindHitBox();
	void UnbindHitBox();
	void DisableHitBox() const;
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	
	UShapeComponent* FindHitBox() const;

private:
	void RegisterStunListener();
	void UnregisterStunListener() const;
#pragma endregion
};


