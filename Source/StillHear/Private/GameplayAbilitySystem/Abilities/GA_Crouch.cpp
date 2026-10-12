#include "GameplayAbilitySystem/Abilities/GA_Crouch.h"

#include "GameFramework/Character.h"
#include "AbilitySystemComponent.h"
#include "GameplayAbilitySystem/Tags/GameplayTags.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

UGA_Crouch::UGA_Crouch()
{
	// "Name of the ability" TAG
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(TAG_GameplayAbility_MainCharacter_Crouch);
	SetAssetTags(AssetTags);

	// When the ability is activated, these tags are added to the owner
	ActivationOwnedTags.AddTag(TAG_GameplayAbility_MainCharacter_Crouch_Active);

	// Block re-activation while ability is running and while crouched status GE is active
	ActivationBlockedTags.AddTag(TAG_GameplayAbility_MainCharacter_Crouch_Active);
	ActivationBlockedTags.AddTag(TAG_Status_MainCharacter_Crouched);
}

void UGA_Crouch::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
		return;
	
	Character->Crouch();

	if (AbilityData && Character->GetCharacterMovement()->CanCrouchInCurrentState())
	{
		UAbilitySystemComponent* Asc = GetAbilitySystemComponentFromActorInfo();
		if (!Asc)
			return;

		const FGameplayEffectSpecHandle SpecHandle = Asc->MakeOutgoingSpec(
			AbilityData->CrouchStatusEffect,
			0.0f,
			Asc->MakeEffectContext()
		);
		if (SpecHandle.IsValid())
		{
			Asc->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
		}
	}

	// Wait for input release to request standing up
	UAbilityTask_WaitGameplayEvent* WaitForUncrouchPressed = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		TAG_Event_InputReleased_Crouch,
		nullptr,
		true,
		true
	);

	WaitForUncrouchPressed->EventReceived.AddDynamic(this, &UGA_Crouch::OnUncrouchPressed);
	WaitForUncrouchPressed->ReadyForActivation();
}

void UGA_Crouch::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const bool bReplicateEndAbility, const bool bWasCancelled)
{
	UAbilitySystemComponent* Asc = GetAbilitySystemComponentFromActorInfo();

	if (Asc && AbilityData)
	{
		// Remove the crouched status GE
		Asc->RemoveActiveGameplayEffectBySourceEffect(AbilityData->CrouchStatusEffect, Asc);
	}
	
	if (ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
	{
		Character->UnCrouch();
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Crouch::OnUncrouchPressed(FGameplayEventData Payload)
{
	CommitAbilityCooldown(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UGA_Crouch::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo) const
{
	const UGameplayEffect* CooldownGE = GetCooldownGameplayEffect();

	if (!CooldownGE || !AbilityData)
	{
		Super::ApplyCooldown(Handle, ActorInfo, ActivationInfo);
		return;
	}

	const FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(CooldownGE->GetClass(), GetAbilityLevel());
	if (SpecHandle.IsValid())
	{
		SpecHandle.Data->SetSetByCallerMagnitude(TAG_Data_MainCharacter_CrouchCooldown, AbilityData->CooldownDuration);
		(void)ApplyGameplayEffectSpecToOwner(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, SpecHandle);
	}
}

