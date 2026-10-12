#include "GameplayAbilitySystem/Abilities/GA_LowVault.h"
#include "MotionWarpingComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/StillHearCharacterBase.h"
#include "Character/StillHearMainCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayAbilitySystem/Tags/GameplayTags.h"

UGA_LowVault::UGA_LowVault()
{
	// "Name of the ability" TAG
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(TAG_GameplayAbility_MainCharacter_LowVault);
	SetAssetTags(AssetTags);

	BlockAbilitiesWithTag.AddTag(TAG_GameplayAbility_MainCharacter);
	
	// When the ability is activated, these tags are added to the owner
	ActivationOwnedTags.AddTag(TAG_GameplayAbility_MainCharacter_LowVault_Active);
}

void UGA_LowVault::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// I deactivate the character's collisions and set the movement mode to flying
	AStillHearMainCharacter* Character = Cast<AStillHearMainCharacter>(GetAvatarActorFromActorInfo());
	Character->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Ignore);
	Character->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	Character->SetAnimationClimbing(true);

	// I move the character at the correct distance from the wall so the animation is smoother
	FVector DesiredLocation = Character->GetActorLocation();
	DesiredLocation.Z = Character->GetWallHeightReference().Z - 20.0f;

	Character->SetActorLocation(DesiredLocation);

	// Setting the Motion Warp Target
	Character->MotionWarping->AddOrUpdateWarpTargetFromLocation(
		FName("Landing"),
		Character->GetMotionWarpTarget()
	);
	
	// I play the vaulting montage
	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			VaultMontage,
			1.0f,
			NAME_None,
			true
		);

	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);

	MontageTask->ReadyForActivation();

	const FGameplayTag TagToWaitFor = TAG_Event_Collision_Activate;
	
	UAbilityTask_WaitGameplayEvent* CollisionEventTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this,
			TagToWaitFor,
			nullptr,
			true,
			true
		);

	CollisionEventTask->EventReceived.AddDynamic(this, &ThisClass::OnCollisionActivateEventReceived);

	CollisionEventTask->ReadyForActivation();
}

void UGA_LowVault::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// I reactivate the character's collisions and set the movement mode to walking
	const AStillHearMainCharacter* Character = Cast<AStillHearMainCharacter>(GetAvatarActorFromActorInfo());
	
	Character->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	Character->SetAnimationClimbing(false);

	// Removing Motion Warp Target
	Character->MotionWarping->RemoveWarpTarget(FName("Landing"));
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_LowVault::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UGA_LowVault::OnMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, true);
}

void UGA_LowVault::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, true);
}

void UGA_LowVault::OnCollisionActivateEventReceived(FGameplayEventData Payload)
{
	const AStillHearMainCharacter* Character = Cast<AStillHearMainCharacter>(GetAvatarActorFromActorInfo());
	Character->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
}
