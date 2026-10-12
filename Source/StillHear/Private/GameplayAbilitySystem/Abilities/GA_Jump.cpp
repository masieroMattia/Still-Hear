#include "GameplayAbilitySystem/Abilities/GA_Jump.h"

#include "Abilities/Tasks/AbilityTask_WaitMovementModeChange.h"
#include "Character/StillHearCharacterBase.h"
#include "Character/StillHearMainCharacter.h"
#include "GameFramework/Character.h"
#include "GameplayAbilitySystem/Tags/GameplayTags.h"
#include "GameFramework/ForceFeedbackEffect.h"

UGA_Jump::UGA_Jump()
{
	// "Name of the ability" TAG
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(TAG_GameplayAbility_MainCharacter_Jump);
	SetAssetTags(AssetTags);

	// When the ability is activated, these tags are added to the owner
	ActivationOwnedTags.AddTag(TAG_GameplayAbility_MainCharacter_Jump_Active);
	
	BlockAbilitiesWithTag.AddTag(TAG_GameplayAbility);
}

void UGA_Jump::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}

	AStillHearMainCharacter* MainCharacter = Cast<AStillHearMainCharacter>(Character);
	UAbilitySystemComponent* Asc = MainCharacter->GetAbilitySystemComponent();
	
	if (!MainCharacter->PersonalizedCanJump() || !Asc)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	} 
	
	MainCharacter->Jump();

	// Play jump force feedback
	if (JumpForceFeedback)
	{
		if (APlayerController* PC = ActorInfo->PlayerController.Get())
		{
			FForceFeedbackParameters Params;
			Params.bLooping = false;
			Params.bIgnoreTimeDilation = true;
			PC->ClientPlayForceFeedback(JumpForceFeedback, Params);
		}
	}

	// Apply effect that gives the tag Status.MainCharacter.InAir
	const FGameplayEffectSpecHandle SpecHandle = Asc->MakeOutgoingSpec(
		AirStatusEffectClass,
		0.0f,
		Asc->MakeEffectContext()
	);
	
	if (SpecHandle.IsValid())
		Asc->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);

	UAbilityTask_WaitMovementModeChange* MovementModeChangeTask =
		UAbilityTask_WaitMovementModeChange::CreateWaitMovementModeChange(
			this,
			MOVE_Walking
		);

	MovementModeChangeTask->OnChange.AddDynamic(this, &UGA_Jump::OnMovementModeChange);
	MovementModeChangeTask->ReadyForActivation();
}

void UGA_Jump::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const bool bReplicateEndAbility, const bool bWasCancelled)
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());

	if (!Character)
		return;
	
	UAbilitySystemComponent* Asc = Cast<AStillHearCharacterBase>(Character)->GetAbilitySystemComponent();

	if (!Asc)
		return;

	Asc->RemoveActiveGameplayEffectBySourceEffect(AirStatusEffectClass, Asc);
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Jump::OnMovementModeChange(EMovementMode NewMode)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}
