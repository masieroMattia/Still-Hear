#include "GameplayAbilitySystem/Abilities/StillHearGameplayAbility.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemComponent.h"
#include "GameplayAbilitySystem/Tags/GameplayTags.h"

class UAbilitySystemGlobals;

UStillHearGameplayAbility::UStillHearGameplayAbility()
{
	ActivationOwnedTags.AddTag(TAG_GameplayAbility_Active);

	ActivationBlockedTags.AddTag(TAG_Status_Death);
	
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

bool UStillHearGameplayAbility::DoesAbilitySatisfyTagRequirements(const UAbilitySystemComponent& AbilitySystemComponent,
	const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	// Define a common lambda to check for blocked tags
	bool bBlocked = false;
	auto CheckForBlocked = [&](const FGameplayTagContainer& ContainerA, const FGameplayTagContainer& ContainerB)
	{
		// Do we not have any tags in common?  Then we're not blocked
		// THIS LINE WAS THE PROBLEM, THE LAST CHECK ORIGINALLY WAS !ContainerA.HasAny(ContainerB)
		if (ContainerA.IsEmpty() || ContainerB.IsEmpty() || !ContainerB.HasAny(ContainerA))
		{
			return;
		}

		if (OptionalRelevantTags)
		{
			// Ensure the global blocking tag is only added once
			if (!bBlocked)
			{
				UAbilitySystemGlobals& AbilitySystemGlobals = UAbilitySystemGlobals::Get();
				const FGameplayTag& BlockedTag = AbilitySystemGlobals.ActivateFailTagsBlockedTag;
				OptionalRelevantTags->AddTag(BlockedTag);
			}

			// Now append all the blocking tags
			OptionalRelevantTags->AppendMatchingTags(ContainerA, ContainerB);
		}

		bBlocked = true;
	};

	// Define a common lambda to check for missing required tags
	bool bMissing = false;
	auto CheckForRequired = [&](const FGameplayTagContainer& TagsToCheck, const FGameplayTagContainer& RequiredTags)
	{
		// Do we have no requirements, or have met all requirements?  Then nothing's missing
		if (RequiredTags.IsEmpty() || TagsToCheck.HasAll(RequiredTags))
		{
			return;
		}

		if (OptionalRelevantTags)
		{
			// Ensure the global missing tag is only added once
			if (!bMissing)
			{
				UAbilitySystemGlobals& AbilitySystemGlobals = UAbilitySystemGlobals::Get();
				const FGameplayTag& MissingTag = AbilitySystemGlobals.ActivateFailTagsMissingTag;
				OptionalRelevantTags->AddTag(MissingTag);
			}

			FGameplayTagContainer MissingTags = RequiredTags; 
			MissingTags.RemoveTags(TagsToCheck.GetGameplayTagParents());
			OptionalRelevantTags->AppendTags(MissingTags);
		}

		bMissing = true;
	};

	// Start by checking all of the blocked tags first (so OptionalRelevantTags will contain blocked tags first)
	CheckForBlocked(AbilitySystemComponent.GetBlockedAbilityTags(), GetAssetTags());
	CheckForBlocked(AbilitySystemComponent.GetOwnedGameplayTags(), ActivationBlockedTags);
	if (SourceTags != nullptr)
	{
		CheckForBlocked(*SourceTags, SourceBlockedTags);
	}
	if (TargetTags != nullptr)
	{
		CheckForBlocked(*TargetTags, TargetBlockedTags);
	}

	// Now check all required tags
	CheckForRequired(AbilitySystemComponent.GetOwnedGameplayTags(), ActivationRequiredTags);
	if (SourceTags != nullptr)
	{
		CheckForRequired(*SourceTags, SourceRequiredTags);
	}
	if (TargetTags != nullptr)
	{
		CheckForRequired(*TargetTags, TargetRequiredTags);
	}

	// We succeeded if there were no blocked tags and no missing required tags	
	return !bBlocked && !bMissing;
}


