#include "GameplayAbilitySystem/Abilities/Interaction/GA_TapInteraction.h"

#include "GameFramework/Character.h"
#include "Interfaces/Interactable.h"
#include "GameplayAbilitySystem/Tags/GameplayTags.h"

#pragma region CONSTRUCTOR
UGA_TapInteraction::UGA_TapInteraction()
{
	// "Name of the ability" TAG
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(TAG_GameplayAbility_MainCharacter_TapInteraction);
	SetAssetTags(AssetTags);

	// When the ability is activated, these tags are added to the owner
	ActivationOwnedTags.AddTag(TAG_GameplayAbility_MainCharacter_TapInteraction_Active);

	BlockAbilitiesWithTag.AddTag(TAG_GameplayAbility_MainCharacter_TapInteraction);
}
#pragma endregion
	
#pragma region OVERRIDE METHODS
void UGA_TapInteraction::OnInteractionStart()
{
	if (CurrentInteractableObj)
	{
		IInteractable* InteractableInterface = Cast<IInteractable>(CurrentInteractableObj);
		if (InteractableInterface)
			InteractableInterface->StartInteraction(Cast<ACharacter>(GetAvatarActorFromActorInfo()));
	}
}

void UGA_TapInteraction::OnStopEventReceived(FGameplayEventData Payload)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_TapInteraction::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (CurrentInteractableObj)
	{
		IInteractable* InteractableInterface = Cast<IInteractable>(CurrentInteractableObj);
		if (InteractableInterface)
			InteractableInterface->EndInteraction(Cast<ACharacter>(GetAvatarActorFromActorInfo()));
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
#pragma endregion

