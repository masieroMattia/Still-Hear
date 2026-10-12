#include "GameplayAbilitySystem/Abilities/Interaction/GA_HoldInteraction.h"

#include "GameFramework/Character.h"
#include "Interfaces/Interactable.h"
#include "GameplayAbilitySystem/Tags/GameplayTags.h"

#pragma region CONSTRUCTOR
UGA_HoldInteraction::UGA_HoldInteraction()
{
	// "Name of the ability" TAG
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(TAG_GameplayAbility_HoldInteraction);
	SetAssetTags(AssetTags);

	// When the ability is activated, these tags are added to the owner
	ActivationOwnedTags.AddTag(TAG_GameplayAbility_HoldInteraction_Active);
	
	BlockAbilitiesWithTag.AddTag(TAG_GameplayAbility_MainCharacter);
}
#pragma endregion

#pragma region OVERRIDE METHODS
void UGA_HoldInteraction::OnInteractionStart()
{
	if (CurrentInteractableObj)
	{
		IInteractable* InteractableInterface = Cast<IInteractable>(CurrentInteractableObj);
		if (InteractableInterface)
			InteractableInterface->StartInteraction(Cast<ACharacter>(GetAvatarActorFromActorInfo()));
	}
}

void UGA_HoldInteraction::OnStopEventReceived(FGameplayEventData Payload)
{
	if (CurrentInteractableObj)
	{
		IInteractable* InteractableInterface = Cast<IInteractable>(CurrentInteractableObj);
		if (InteractableInterface)
			InteractableInterface->EndInteraction(Cast<ACharacter>(GetAvatarActorFromActorInfo()));
	}
	
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
#pragma endregion