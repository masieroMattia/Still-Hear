#include "GameplayAbilitySystem/Abilities/GA_LowGetOnTop.h"

#include "MotionWarpingComponent.h"
#include "Character/StillHearCharacterBase.h"
#include "Character/StillHearMainCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameplayAbilitySystem/Tags/GameplayTags.h"

UGA_LowGetOnTop::UGA_LowGetOnTop()
{
	// "Name of the ability" TAG
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(TAG_GameplayAbility_MainCharacter_LowGetOnTop);
	SetAssetTags(AssetTags);

	BlockAbilitiesWithTag.AddTag(TAG_GameplayAbility_MainCharacter);
	
	// When the ability is activated, these tags are added to the owner
	ActivationOwnedTags.AddTag(TAG_GameplayAbility_MainCharacter_LowGetOnTop_Active);
}

void UGA_LowGetOnTop::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// Deactivating Capsule collisions and setting Movement Mode
	AStillHearMainCharacter* Character = Cast<AStillHearMainCharacter>(GetAvatarActorFromActorInfo());
	Character->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Ignore);
	Character->GetCharacterMovement()->SetMovementMode(MOVE_Flying);

	// Deactivating IK Trace
	Character->SetAnimationClimbing(true);

	// Setting the Motion Warp Target
	Character->MotionWarping->AddOrUpdateWarpTargetFromLocation(
		FName("GetUp"),
		Character->GetMotionWarpTarget()
	);

	// Play get on top of the wall montage
	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			LowGetOnTopMontage,
			1.0f,
			NAME_None,
			true
		);

	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);

	MontageTask->ReadyForActivation();
}

void UGA_LowGetOnTop::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// Reactivating Capsule collisions and setting Movement Mode
	const AStillHearMainCharacter* Character = Cast<AStillHearMainCharacter>(GetAvatarActorFromActorInfo());
	Character->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	Character->GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	// Reactivating IK Trace
	Character->SetAnimationClimbing(false);

	// Removing Motion Warp Target
	Character->MotionWarping->RemoveWarpTarget(FName("GetUp"));
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_LowGetOnTop::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UGA_LowGetOnTop::OnMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, true);
}

void UGA_LowGetOnTop::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, true);
}
