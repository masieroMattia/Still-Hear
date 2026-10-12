#include "GameplayAbilitySystem/Abilities/State/GA_Death.h"

#include "GameplayAbilitySystem/Tags/GameplayTags.h"
#include "GameFramework/ForceFeedbackEffect.h"

UGA_Death::UGA_Death()
{
	// "Name of the ability" TAG
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(TAG_GameplayAbility_Death);
	SetAssetTags(AssetTags);

	// When the ability is activated, these tags are added to the owner
	ActivationOwnedTags.AddTag(TAG_GameplayAbility_Death_Active);
	
	CancelAbilitiesWithTag.AddTag(TAG_GameplayAbility);
	BlockAbilitiesWithTag.AddTag(TAG_GameplayAbility);
	
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_Death::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// Play heavy, long force feedback vibration on character death
	if (DeathForceFeedback && ActorInfo)
	{
		if (APlayerController* PC = ActorInfo->PlayerController.Get())
		{
			FForceFeedbackParameters Params;
			Params.bLooping = false;
			Params.bIgnoreTimeDilation = true;
			PC->ClientPlayForceFeedback(DeathForceFeedback, Params);
		}
	}

	(void)ApplyGameplayEffectToOwner(Handle, ActorInfo, ActivationInfo, DeathEffectClass.GetDefaultObject(), 1, 1);
	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}
