#include "GameplayAbilitySystem/Abilities/EnemiesAI/GA_Stun.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "GameplayAbilitySystem/Tags/GameplayTags.h"
#include "EnemiesAI/Pawns/Base/StillHearAICharacterBase.h"
#include "EnemiesAI/Utility/DataAssets/AIInfo_DataAssetBase.h"
#include "EnemiesAI/Utility/DataAssets/Abilities/StunData.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "EnemiesAI/Controllers/Base/StillHearAIControllerBase.h"

#pragma region CONSTRUCTOR
UGA_Stun::UGA_Stun()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// Name of the ability TAG
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(TAG_GameplayAbility_EnemyAI_Stun);
	SetAssetTags(AssetTags);

	// Activation tags
	ActivationOwnedTags.AddTag(TAG_GameplayAbility_EnemyAI_Stun_Active);

	// This ability is triggered when the stun tag is added
	AbilityTriggers.Empty();
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = TAG_Status_EnemyAI_Stunned;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::OwnedTagPresent;
	AbilityTriggers.Add(TriggerData);
}
#pragma endregion

#pragma region METHODS
void UGA_Stun::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AStillHearAICharacterBase* AICharacter = Cast<AStillHearAICharacterBase>(GetAvatarActorFromActorInfo());
	if (!AICharacter || !StunData)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// Change Collision: Ignore specified channels
	AICharacter->SetCollision(StunData->ChannelsToIgnore, true);

	// Add GameplayCue for visuals
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		const float StunDuration = AICharacter->GetAIInfo_DataAsset()->StunDuration;
		
		FGameplayCueParameters CueParams;
		CueParams.Instigator = GetAvatarActorFromActorInfo();
		CueParams.RawMagnitude = StunDuration;
		ASC->AddGameplayCue(TAG_GameplayCue_EnemyAI_Stun, CueParams);
	}

	// Wait for the duration
	UAbilityTask_WaitDelay* WaitDelayTask = UAbilityTask_WaitDelay::WaitDelay(
		this,
		AICharacter->GetAIInfo_DataAsset()->StunDuration
	);
	WaitDelayTask->OnFinish.AddDynamic(this, &UGA_Stun::OnStunDurationFinished);
	WaitDelayTask->ReadyForActivation();
}

void UGA_Stun::OnStunDurationFinished()
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		FGameplayTagContainer StunTags;
		StunTags.AddTag(TAG_Status_EnemyAI_Stunned);
		
		// Remove any active effects that grant the stun tag
		ASC->RemoveActiveEffectsWithGrantedTags(StunTags);

		// If the tag is still present (e.g. added as a loose tag), remove it manually
		if (ASC->HasMatchingGameplayTag(TAG_Status_EnemyAI_Stunned))
		{
			ASC->RemoveLooseGameplayTag(TAG_Status_EnemyAI_Stunned);
		}
	}
}

void UGA_Stun::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (AStillHearAICharacterBase* AICharacter = Cast<AStillHearAICharacterBase>(GetAvatarActorFromActorInfo()))
	{
		// Restore Collision
		if (StunData)
			AICharacter->SetCollision(StunData->ChannelsToIgnore, false);

		// Remove GameplayCue
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
			ASC->RemoveGameplayCue(TAG_GameplayCue_EnemyAI_Stun);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
#pragma endregion
