// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAbilitySystem/Abilities/EnemiesAI/Worm/GA_WormRoar.h"

#include "AIController.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Animation/AnimInstances/EnemyAI/WormAnimInstance.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnemiesAI/Controllers/Base/StillHearAIControllerBase.h"
#include "EnemiesAI/Pawns/Worm/AIWormCharacter.h"
#include "EnemiesAI/Utility/BlackboardKeyNames.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayAbilitySystem/Abilities/GA_Resonance.h"
#include "GameplayAbilitySystem/Tags/GameplayTags.h"

class UAbilityTask_WaitGameplayEvent;

UGA_WormRoar::UGA_WormRoar()
{
	// "Name of the ability" TAG
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(TAG_GameplayAbility_EnemyAI_WormRoar);
	SetAssetTags(AssetTags);
	
	// When the ability is activated, these tags are added to the owner
	ActivationOwnedTags.AddTag(TAG_GameplayAbility_EnemyAI_WormRoar_Active);
}

void UGA_WormRoar::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AAIWormCharacter* Worm = Cast<AAIWormCharacter>(GetAvatarActorFromActorInfo());
	AStillHearAIControllerBase* WormController = Worm->GetAICRef();

	UBlackboardComponent* Blackboard = WormController->GetBlackboardComponent();
	Blackboard->SetValueAsBool(BlackboardKeyNames::KeyNameHasRoared, true);
	
	UWormAnimInstance* WormAnimInstance = Cast<UWormAnimInstance>(Worm->GetMesh()->GetAnimInstance());

	Worm->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	WormAnimInstance->SetIsRoaring(true);
	
	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			RoarMontage,
			1.0f,
			NAME_None,
			true
		);

	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);

	MontageTask->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* RoarStarted = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		TAG_Event_EnemyAI_WormRoar, 
		nullptr,
		false,
		true
	);
	
	RoarStarted->EventReceived.AddDynamic(this, &ThisClass::ApplyRoarEffect);
	RoarStarted->ReadyForActivation();
}

void UGA_WormRoar::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	AAIWormCharacter* Worm = Cast<AAIWormCharacter>(GetAvatarActorFromActorInfo());
	UWormAnimInstance* WormAnimInstance = Cast<UWormAnimInstance>(Worm->GetMesh()->GetAnimInstance());

	Worm->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	WormAnimInstance->SetIsRoaring(false);
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_WormRoar::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UGA_WormRoar::OnMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, true);
}

void UGA_WormRoar::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, true);
}

void UGA_WormRoar::ApplyRoarEffect(FGameplayEventData Payload)
{
	UAbilitySystemComponent* Asc = GetAbilitySystemComponentFromActorInfo();

	if (!Asc)
		return;
	
	// Apply effect that gives the tag Status.MainCharacter.InAir
	const FGameplayEffectSpecHandle SpecHandle = Asc->MakeOutgoingSpec(
		RoarEffect,
		0.0f,
		Asc->MakeEffectContext()
	);
	
	if (SpecHandle.IsValid())
		Asc->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
}
