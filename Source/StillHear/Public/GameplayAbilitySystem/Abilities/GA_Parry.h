#pragma once

#include "CoreMinimal.h"
#include "StillHearGameplayAbility.h"
#include "GA_Parry.generated.h"

class UParryData;

UCLASS()
class STILLHEAR_API UGA_Parry : public UStillHearGameplayAbility
{
	GENERATED_BODY()
	
#pragma region UPROPERTIES
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parry Settings")
	TObjectPtr<UParryData> ParryData;
#pragma endregion
	
#pragma region VARIABLES
private:
	UPROPERTY()
	TObjectPtr<UShapeComponent> ParrySphereComponent;

	bool bHasTriggeredSuccessfulParry = false;

	FTimerHandle ParryWindowTimer;
#pragma endregion
	
#pragma region CONSTRUCTOR
public:
	UGA_Parry();
#pragma endregion
	
#pragma region METHODS
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;

	UFUNCTION()
	void OnParrySphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnCollisionEventReceived(FGameplayEventData Payload);

	void OnParryWindowFinished();
	
	UFUNCTION()
	void OnInvulnerabilityEffectRemoved(const FGameplayEffectRemovalInfo& RemovalInfo);

private:
	void ResetParrySphere();
	void ApplyGameplayEffectToTarget(AActor* Target, const FVector& ImpactLocation);
	void TriggerParrySensoryEffects();
#pragma endregion
};
