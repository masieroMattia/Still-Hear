// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAbilitySystem/Abilities/EnemiesAI/Worm/GA_WormDolphinDive.h"

#include "Abilities/Tasks/AbilityTask_WaitMovementModeChange.h"
#include "Animation/AnimInstances/EnemyAI/WormAnimInstance.h"
#include "EnemiesAI/Controllers/Worm/AIWormController.h"
#include "EnemiesAI/Pawns/Worm/AIWormCharacter.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameplayAbilitySystem/Tags/GameplayTags.h"
#include "Input/InputDeviceType.h"
#include "Kismet/GameplayStatics.h"

UGA_WormDolphinDive::UGA_WormDolphinDive()
{
	// "Name of the ability" TAG
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(TAG_GameplayAbility_EnemyAI_WormDolphinDive);
	AssetTags.AddTag(TAG_GameplayAbility_EnemyAI_Attack);
	SetAssetTags(AssetTags);
	
	// When the ability is activated, these tags are added to the owner
	ActivationOwnedTags.AddTag(TAG_GameplayAbility_EnemyAI_WormDolphinDive_Active);
	ActivationOwnedTags.AddTag(TAG_GameplayAbility_EnemyAI_Attack_Active);

	ActivationBlockedTags.AddTag(TAG_GameplayAbility_EnemyAI_WormDolphinDive_Active);
}

void UGA_WormDolphinDive::OnMovementModeChange(EMovementMode NewMode)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UGA_WormDolphinDive::OnStunTagAdded(const FGameplayTag Tag, int32 NewCount)
{
	// When the Stunned tag is applied (NewCount > 0), cancel the attack ability
	if (NewCount > 0)
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false);
}

void UGA_WormDolphinDive::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AAIWormCharacter* Worm = Cast<AAIWormCharacter>(GetAvatarActorFromActorInfo());
	AAIWormController* Controller = Cast<AAIWormController>(Worm->GetAICRef());
	UWormAnimInstance* WormAnimInstance = Cast<UWormAnimInstance>(Worm->GetMesh()->GetAnimInstance());

	if (Worm->GetMovementComponent()->IsFalling())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}

	const FVector Target = Controller->GetCurrentTargetLocation();
	FVector LaunchVelocity = FVector::ZeroVector;
	
	bool bArcFound = UGameplayStatics::SuggestProjectileVelocity_CustomArc(
		GetWorld(),
		LaunchVelocity,
		Worm->GetActorLocation(),
		Target + FVector(0, 0, 100)
	);

	if (!bArcFound)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}

	GetAbilitySystemComponentFromActorInfo()->ExecuteGameplayCue(TAG_GameplayCue_EnemyAI_AttackFeedback);
	
	WormAnimInstance->SetIsDiving(true);
	Worm->LaunchCharacter(LaunchVelocity, true, true);

	UAbilityTask_WaitMovementModeChange* MovementModeChangeTask =
		UAbilityTask_WaitMovementModeChange::CreateWaitMovementModeChange(
			this,
			MOVE_Walking
		);

	MovementModeChangeTask->OnChange.AddDynamic(this, &ThisClass::OnMovementModeChange);
	MovementModeChangeTask->ReadyForActivation();

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC)
		StunTagDelegateHandle = ASC->RegisterGameplayTagEvent(TAG_Status_EnemyAI_Stunned, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ThisClass::OnStunTagAdded);
	
	CommitAbilityCooldown(Handle, ActorInfo, ActivationInfo, true);

	Controller->SetIsDiving(true);
}

void UGA_WormDolphinDive::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	const AAIWormCharacter* Worm = Cast<AAIWormCharacter>(GetAvatarActorFromActorInfo());
	
	UWormAnimInstance* WormAnimInstance = Cast<UWormAnimInstance>(Worm->GetMesh()->GetAnimInstance());
	WormAnimInstance->SetIsDiving(false);

	AAIWormController* Controller = Cast<AAIWormController>(Worm->GetAICRef());
	Controller->SetIsDiving(false);

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC)
		ASC->UnregisterGameplayTagEvent(StunTagDelegateHandle, TAG_Status_EnemyAI_Stunned, EGameplayTagEventType::NewOrRemoved);
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
