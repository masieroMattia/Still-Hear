#include "GameplayAbilitySystem/Abilities/GA_Sprint.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/StillHearMainCharacter.h"
#include "GameplayAbilitySystem/Tags/GameplayTags.h"

UGA_Sprint::UGA_Sprint()
{
	// "Name of the ability" TAG
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(TAG_GameplayAbility_MainCharacter_Sprint);
	SetAssetTags(AssetTags);

	BlockAbilitiesWithTag.AddTag(TAG_GameplayAbility_MainCharacter_Crouch);
	CancelAbilitiesWithTag.AddTag(TAG_GameplayAbility_MainCharacter_Crouch);
	
	// When the ability is activated, these tags are added to the owner
	ActivationOwnedTags.AddTag(TAG_GameplayAbility_MainCharacter_Sprint_Active);

}

void UGA_Sprint::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	const AStillHearMainCharacter* Character = Cast<AStillHearMainCharacter>(GetAvatarActorFromActorInfo());

	if (!Character)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}
	
	// I calculate the multiplier to apply depending on the values set by designers
	const float Multiplier = Character->SprintSpeed / Character->BaseSpeed;

	ApplySpeedMultiplierToOwner(Character, Multiplier);

	//Character->SetLocomotionBlendSpace(SprintBlendSpace);

	// I wait for the input release of the sprint button through a gameplay event
	const FGameplayTag TagToWaitFor = TAG_Event_InputReleased_Sprint;
	
	UAbilityTask_WaitGameplayEvent* WaitForInputRelease =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this,
			TagToWaitFor,
			nullptr,
			true,
			true
		);

	WaitForInputRelease->EventReceived.AddDynamic(this, &UGA_Sprint::OnInputRelease);

	WaitForInputRelease->ReadyForActivation();
}

void UGA_Sprint::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const bool bReplicateEndAbility, const bool bWasCancelled)
{
	const AStillHearMainCharacter* Character = Cast<AStillHearMainCharacter>(GetAvatarActorFromActorInfo());

	if (!Character)
		return;

	UAbilitySystemComponent* Asc = Character->GetAbilitySystemComponent();
	
	if (!Asc)
		return;

	Asc->RemoveActiveGameplayEffectBySourceEffect(SprintSpeedMultiplierEffectClass, Asc);

	//Character->SetDefaultLocomotionBlendSpace();
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Sprint::ApplySpeedMultiplierToOwner(const AStillHearMainCharacter* Character, const float Multiplier) const
{
	
	UAbilitySystemComponent* Asc = Character->GetAbilitySystemComponent();
	
	if (!Asc)
		return;

	const FGameplayEffectSpecHandle SpecHandle = Asc->MakeOutgoingSpec(
		SprintSpeedMultiplierEffectClass,
		1.0f,
		Asc->MakeEffectContext()
	);

	if (!SpecHandle.IsValid())
		return;

	SpecHandle.Data->SetSetByCallerMagnitude(
		FGameplayTag::RequestGameplayTag(FName("Data.SpeedMultiplier")),
		Multiplier
	);

	// Apply the effect to change the values in the character class
	Asc->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
}

void UGA_Sprint::OnInputRelease(FGameplayEventData Payload)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}
