#include "GameplayAbilitySystem/Abilities/GA_Climb.h"

#include "MotionWarpingComponent.h"
#include "Character/StillHearMainCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/ForceFeedbackEffect.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameplayAbilitySystem/Tags/GameplayTags.h"

UGA_Climb::UGA_Climb()
{
	// "Name of the ability" TAG
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(TAG_GameplayAbility_MainCharacter_Climb);
	SetAssetTags(AssetTags);

	BlockAbilitiesWithTag.AddTag(TAG_GameplayAbility_MainCharacter);

	CancelAbilitiesWithTag.AddTag(TAG_GameplayAbility_MainCharacter_Jump);
	
	// When the ability is activated, these tags are added to the owner
	ActivationOwnedTags.AddTag(TAG_GameplayAbility_MainCharacter_Climb_Active);
}

void UGA_Climb::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// Play climb force feedback
	if (ClimbForceFeedback)
	{
		if (APlayerController* PC = ActorInfo->PlayerController.Get())
		{
			FForceFeedbackParameters Params;
			Params.bLooping = false;
			Params.bIgnoreTimeDilation = true;
			PC->ClientPlayForceFeedback(ClimbForceFeedback, Params);
		}
	}
	
	// Deactivating Capsule collisions and setting Movement Mode
	AStillHearMainCharacter* Character = Cast<AStillHearMainCharacter>(GetAvatarActorFromActorInfo());
	Character->GetCharacterMovement()->StopMovementImmediately();
	Character->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	Character->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Ignore);

	// Deactivating IK Trace For Feet
	Character->SetAnimationClimbing(true);
	
	// Setting the Motion Warp Target
	Character->MotionWarping->AddOrUpdateWarpTargetFromLocation(
		FName("Climb"),
		Character->GetMotionWarpTarget()
	);
	
	// I play the climbing montage
	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			ClimbMontage,
			1.0f,
			NAME_None,
			true
		);

	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);

	MontageTask->ReadyForActivation();
}

void UGA_Climb::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// Reactivating Capsule collisions and setting Movement Mode
	const AStillHearMainCharacter* Character = Cast<AStillHearMainCharacter>(GetAvatarActorFromActorInfo());
	Character->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	Character->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	
	// Reactivating IK Trace
	Character->SetAnimationClimbing(false);
	
	// Removing Motion Warp Target
	Character->MotionWarping->RemoveWarpTarget(FName("Climb"));
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Climb::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UGA_Climb::OnMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, true);
}

void UGA_Climb::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, true);
}
