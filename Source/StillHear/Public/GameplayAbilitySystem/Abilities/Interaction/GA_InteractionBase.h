#pragma once

#include "CoreMinimal.h"
#include "Data/DataAssets/InteractionBaseData.h"
#include "GameplayAbilitySystem/Tasks/AT_NavigateTo.h"
#include "GameplayAbilitySystem/Abilities/StillHearGameplayAbility.h"
#include "GA_InteractionBase.generated.h"

class UAbilityTask_PlayMontageAndWait;

UCLASS(Abstract)
class STILLHEAR_API UGA_InteractionBase : public UStillHearGameplayAbility
{
	GENERATED_BODY()
	
#pragma region UPROPERTIES
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction Settings")
	TObjectPtr<UInteractionBaseData> InteractionData; // Data asset containing the settings for the interaction
#pragma endregion
	
#pragma region VARIABLES
protected:
	UPROPERTY()
	TObjectPtr<AActor> CurrentInteractableObj; // The interactable object that the character is currently interacting with
	UPROPERTY()
	TObjectPtr<UAT_NavigateTo> NavigateTask;

	FGameplayEffectSpecHandle MoveToEffectSpecHandle; // Handle for the movement effect applied while navigating to the interactable
#pragma endregion
	
#pragma region METHODS
public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void OnInteractionStart() PURE_VIRTUAL(UGA_InteractionBase::OnInteractionStart);
#pragma endregion
	
#pragma region UFUNCTIONS
protected:
	UFUNCTION()
	void OnTargetLocationReached();
	UFUNCTION()
	virtual void OnStopEventReceived(FGameplayEventData Payload) PURE_VIRTUAL(UGA_InteractionBase::OnStopEventReceived);
#pragma endregion

};
