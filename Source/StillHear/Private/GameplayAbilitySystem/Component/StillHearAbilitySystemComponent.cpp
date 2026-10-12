// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAbilitySystem/Component/StillHearAbilitySystemComponent.h"

#include "Character/StillHearCharacterBase.h"


UStillHearAbilitySystemComponent::UStillHearAbilitySystemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
}

void UStillHearAbilitySystemComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void UStillHearAbilitySystemComponent::OnRep_ActivateAbilities()
{
	Super::OnRep_ActivateAbilities();

	AStillHearCharacterBase * Character = Cast<AStillHearCharacterBase>(GetOwner());

	if (!Character)
		return;

	bool bAbilitiesChanged = false;

	// If the number between the abilities I had last time and the abilities I have now changed, then I surely have
	// a new ability, or one of the abilities I had before got removed
	if (LastActivatableAbility.Num() != ActivatableAbilities.Items.Num())
	{
		Character->SendAbilitiesChangedEvent();
		LastActivatableAbility = ActivatableAbilities.Items;
	}
	// If the number is the same, I have to check one by one if the abilities I have are actually still the same ones
	else
	{
		for (int i = 0; i < LastActivatableAbility.Num(); i++)
		{
			if (LastActivatableAbility[i].Ability == ActivatableAbilities.Items[i].Ability)
			{
				bAbilitiesChanged = true;
				break;
			}
		}
	}

	// If this is true, then even if the abilities number was the same, they were indeed different
	if (bAbilitiesChanged)
	{
		Character->SendAbilitiesChangedEvent();
		LastActivatableAbility = ActivatableAbilities.Items;
	}
}

void UStillHearAbilitySystemComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                     FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

