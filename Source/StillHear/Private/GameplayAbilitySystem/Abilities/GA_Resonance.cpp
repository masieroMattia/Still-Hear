#include "GameplayAbilitySystem/Abilities/GA_Resonance.h"

#include "Interfaces/Interactable.h"
#include "Data/DataAssets/ResonanceData.h"
#include "VFX/ResonanceManagerComponent.h"
#include "Character/StillHearMainCharacter.h"
#include "TraceAndCollision/CustomCollision.h"
#include "GameFramework/ForceFeedbackEffect.h"
#include "Interactions/Actors/ChaosResonanceObj.h"
#include "GameplayAbilitySystem/Tags/GameplayTags.h"
#include "Camera/CameraEffects/CameraEffectsComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

#pragma region CONSTRUCTOR
UGA_Resonance::UGA_Resonance()
{
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(TAG_GameplayAbility_MainCharacter_Resonance);
	SetAssetTags(AssetTags);

	ActivationOwnedTags.AddTag(TAG_GameplayAbility_MainCharacter_Resonance_Active);
	
	BlockAbilitiesWithTag.AddTag(TAG_GameplayAbility_MainCharacter_Resonance);
}
#pragma endregion

#pragma region METHODS
void UGA_Resonance::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	const AStillHearMainCharacter* Character = Cast<AStillHearMainCharacter>(ActorInfo->AvatarActor.Get());
	if (!Character)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	CachedPlayerController = Cast<APlayerController>(Character->GetController());
	if (CachedPlayerController)
		CachedCameraEffectComponent = CachedPlayerController->GetComponentByClass(UCameraEffectsComponent::StaticClass()) ? Cast<UCameraEffectsComponent>(CachedPlayerController->GetComponentByClass(UCameraEffectsComponent::StaticClass())) : nullptr;

	ResonanceManager = Character->GetResonanceManagerComponent();
	if (!ResonanceManager)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	FindValidResonanceObjects();

	ResonanceManager->OnResonanceSuccess.AddDynamic(this, &UGA_Resonance::OnResonanceSuccess);
	ResonanceManager->OnResonanceInterrupted.AddDynamic(this, &UGA_Resonance::OnResonanceInterrupted);
	ResonanceManager->StartResonance();

	// Wait for the "Match Input" event, which will be triggered by the character when they attempt to match the resonance
	UAbilityTask_WaitGameplayEvent* WaitMatchInput = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		TAG_Event_MainCharacter_ActivateResonance, 
		nullptr,
		false,
		true
	);
	
	if (WaitMatchInput)
	{
		WaitMatchInput->EventReceived.AddDynamic(this, &UGA_Resonance::OnMatchInputReceived);
		WaitMatchInput->ReadyForActivation();
	}
	
	// Wait for the "Cancel" or "Stop" event
	UAbilityTask_WaitGameplayEvent* WaitStop = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		TAG_Event_MainCharacter_StopResonance,
		nullptr,
		true,
		true
	);
	
	if (WaitStop)
	{
		WaitStop->EventReceived.AddDynamic(this, &UGA_Resonance::OnStopEventReceived);
		WaitStop->ReadyForActivation();
	}
	
	if (CachedCameraEffectComponent)
		CachedCameraEffectComponent->PlayEffectPreset(ResonanceData->EntryEffectPreset);

	// Play continuous (looping) light force feedback vibration while resonance is active
	if (ResonanceData->ActiveResonanceForceFeedback && CachedPlayerController)
	{
		FForceFeedbackParameters Params;
		Params.bLooping = true;
		Params.bIgnoreTimeDilation = true;
		Params.Tag = FName("ResonanceActive");
		CachedPlayerController->ClientPlayForceFeedback(ResonanceData->ActiveResonanceForceFeedback, Params);
	}
}

void UGA_Resonance::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	CurrentResonanceObjects.Empty();

	if (ResonanceManager)
	{
		ResonanceManager->OnResonanceSuccess.RemoveDynamic(this, &UGA_Resonance::OnResonanceSuccess);
		ResonanceManager->OnResonanceInterrupted.RemoveDynamic(this, &UGA_Resonance::OnResonanceInterrupted);
		ResonanceManager->StopResonance();
	}

	if (CachedCameraEffectComponent && bWasCancelled)
		CachedCameraEffectComponent->StopAllEffects();

	// Stop the continuous looping force feedback vibration
	if (ResonanceData->ActiveResonanceForceFeedback && CachedPlayerController)
	{
		CachedPlayerController->ClientStopForceFeedback(ResonanceData->ActiveResonanceForceFeedback, FName("ResonanceActive"));
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Resonance::FindValidResonanceObjects()
{
	CurrentResonanceObjects.Empty();
	if (!ResonanceData) 
		return;

	const FVector Center = GetAvatarActorFromActorInfo()->GetActorLocation();
	const float Radius = ResonanceData->ResonanceRadius;

	TArray<FOverlapResult> OverlapResults;
	const FCollisionObjectQueryParams Params(ECustomCollision::Resonance);

	if (GetWorld()->OverlapMultiByObjectType(OverlapResults, Center, FQuat::Identity, Params, FCollisionShape::MakeSphere(Radius)))
	{
		for (const FOverlapResult& Result : OverlapResults)
		{
			AActor* Obj = Result.GetActor();
			if (Obj && Obj->Implements<UInteractable>() && Obj->Implements<UGameplayTagAssetInterface>())
			{
				// Skip objects already broken/breaking so a second activation can't re-target them.
				if (const AChaosResonanceObj* ResonanceObj = Cast<AChaosResonanceObj>(Obj))
				{
					if (ResonanceObj->GetResonanceState() != EChaosResonanceState::Intact)
						continue;
				}

				const IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(Obj);
				if (TagInterface && TagInterface->HasAnyMatchingGameplayTags(ResonanceData->ResonanceTags))
					CurrentResonanceObjects.Add(Obj);
			}
		}
	}
}
#pragma endregion

#pragma region UFUNCTIONS
void UGA_Resonance::OnMatchInputReceived(FGameplayEventData Payload)
{
	// Play strong, sharp force feedback vibration when the interaction button is pressed
	if (ResonanceData->InteractResonanceForceFeedback && CachedPlayerController)
	{
		FForceFeedbackParameters Params;
		Params.bLooping = false;
		Params.bIgnoreTimeDilation = true;
		CachedPlayerController->ClientPlayForceFeedback(ResonanceData->InteractResonanceForceFeedback, Params);
	}

	if (ResonanceManager)
		ResonanceManager->AttemptMatch();
}

void UGA_Resonance::OnResonanceSuccess()
{
	if (CachedCameraEffectComponent)
	{
		CachedCameraEffectComponent->StopAllEffects();
		CachedCameraEffectComponent->PlayEffectPreset(ResonanceData->SuccessEffectPreset);
	}

	// Objects were captured at activation time; skip any that got destroyed/replaced before this fired.
	for (const TObjectPtr<AActor>& Obj : CurrentResonanceObjects)
	{
		if (!IsValid(Obj))
			continue;

		if (IInteractable* Interactable = Cast<IInteractable>(Obj))
			Interactable->StartInteraction(Cast<ACharacter>(GetAvatarActorFromActorInfo()));
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Resonance::OnResonanceInterrupted()
{
	if (CachedCameraEffectComponent)
	{
		CachedCameraEffectComponent->StopAllEffects();
		CachedCameraEffectComponent->PlayEffectPreset(ResonanceData->InterruptEffectPreset);
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_Resonance::OnStopEventReceived(FGameplayEventData Payload)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
#pragma endregion