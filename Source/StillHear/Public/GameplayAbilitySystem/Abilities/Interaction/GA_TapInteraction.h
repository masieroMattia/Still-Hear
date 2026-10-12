#pragma once

#include "CoreMinimal.h"
#include "GA_InteractionBase.h"
#include "GA_TapInteraction.generated.h"

UCLASS()
class STILLHEAR_API UGA_TapInteraction : public UGA_InteractionBase
{
	GENERATED_BODY()
	
#pragma region CONSTRUCTOR
	UGA_TapInteraction();
#pragma endregion
	
#pragma region OVERRIDE METHODS
protected:
	virtual void OnInteractionStart() override;
	virtual void OnStopEventReceived(FGameplayEventData Payload) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
#pragma endregion
};
