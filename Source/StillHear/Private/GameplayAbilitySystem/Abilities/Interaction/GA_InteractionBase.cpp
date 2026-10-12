#include "GameplayAbilitySystem/Abilities/Interaction/GA_InteractionBase.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "Interfaces/Interactable.h"
#include "GameplayAbilitySystem/Tags/GameplayTags.h"
#include "GameplayAbilitySystem/Tasks/AT_NavigateTo.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "TraceAndCollision/CustomCollision.h"
#include "GameFramework/ForceFeedbackEffect.h"

#pragma region METHODS
void UGA_InteractionBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	const ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (!Character)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	if (!InteractionData)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	
	const FGameplayTag TagToWaitFor = TAG_Event_StopInteraction;
	UAbilityTask_WaitGameplayEvent* WaitStopInteraction = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		TagToWaitFor,
		nullptr,
		false,
		true
	);

	WaitStopInteraction->EventReceived.AddDynamic(this, &UGA_InteractionBase::OnStopEventReceived);
	WaitStopInteraction->ReadyForActivation();
	
	const FVector InteractSphereCenter = Character->GetActorLocation();
	const float InteractSphereRadius = InteractionData->InteractRadius;
	
	TArray<FOverlapResult> OverlapResults;
	FCollisionObjectQueryParams CollisionObjectParams;
	CollisionObjectParams.AddObjectTypesToQuery(ECustomCollision::Interactable);
	
	const bool bHasOverlap = Character->GetWorld()->OverlapMultiByObjectType(
		OverlapResults,
		InteractSphereCenter,
		FQuat::Identity,
		CollisionObjectParams,
		FCollisionShape::MakeSphere(InteractSphereRadius)
	);
	
	if (InteractionData->bDrawDebug)
	{
		#if WITH_EDITOR
			// Debug sphere to visualize the interaction radius
			DrawDebugSphere(Character->GetWorld(), InteractSphereCenter, InteractSphereRadius, 12, FColor::Green, false, 2.0f);
		#endif
	}
	
	if (bHasOverlap)
	{
		// Sort overlaps by distance so the player always interacts with the closest valid object first
		OverlapResults.Sort([&InteractSphereCenter](const FOverlapResult& A, const FOverlapResult& B) {
		   const float DistA = A.GetActor() ? FVector::DistSquared(InteractSphereCenter, A.GetActor()->GetActorLocation()) : MAX_flt;
		   const float DistB = B.GetActor() ? FVector::DistSquared(InteractSphereCenter, B.GetActor()->GetActorLocation()) : MAX_flt;
		   return DistA < DistB;
		});
		
		for (const FOverlapResult& Result : OverlapResults)
		{
			AActor* OverlappedActor = Result.GetActor();
			if (!IsValid(OverlappedActor) || !OverlappedActor->Implements<UInteractable>())
				continue; // Skip actors that are not valid or do not implement the interactable interface

			if (!OverlappedActor->Implements<UGameplayTagAssetInterface>())
				continue; // Skip actors that do not implement the gameplay tag asset interface

			const IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(OverlappedActor);
			if (!TagInterface || !TagInterface->HasAnyMatchingGameplayTags(InteractionData->InteractionTags))
				continue;
			
			CurrentInteractableObj = OverlappedActor; // Set the current interactable object to the first valid interactable object found in the overlap

			const float ZDiff = CurrentInteractableObj->GetActorLocation().Z - Character->GetActorLocation().Z;
			if (FMath::Abs(ZDiff) > InteractionData->MaxZDifferenceToInteract)
			{
				CurrentInteractableObj = nullptr; // Set the current interactable object to null if it's too high or too low compared to the character
				continue;
			}

			FVector NearestSpotLocation = FVector::ZeroVector;
			FVector NearestSpotDirection = FVector::ZeroVector;
			
			IInteractable* InteractableInterface = Cast<IInteractable>(CurrentInteractableObj);
			
			const bool bIsThereNearSpot = InteractableInterface->GetNearestInteractionSpotLocation(Character->GetActorLocation(), NearestSpotLocation, NearestSpotDirection);
			if (!bIsThereNearSpot) 
			{
				NearestSpotLocation = Character->GetActorLocation();
				NearestSpotDirection = (CurrentInteractableObj->GetActorLocation() - Character->GetActorLocation());
				NearestSpotDirection.Z = 0.0f;
				NearestSpotDirection.Normalize();
			}
			
			const float AcceptanceRadius = bIsThereNearSpot ? InteractionData->SpotAcceptanceRadius : InteractionData->MoveToAcceptanceRadius;

			// Task to move towards the slot if it's too far
			NavigateTask = UAT_NavigateTo::NavigateTo(
				this,
				NearestSpotLocation,
				AcceptanceRadius,
				NearestSpotDirection
			);
			
			NavigateTask->OnTargetLocationReached.AddDynamic(this, &UGA_InteractionBase::OnTargetLocationReached);
			NavigateTask->ReadyForActivation();

			UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
			if (AbilitySystemComponent && InteractionData->MoveToEffectClass) 
			{
				FGameplayTagContainer TagsToRemove;
				TagsToRemove.AddTag(TAG_GameplayAbility_MainCharacter_Sprint);
				AbilitySystemComponent->CancelAbilities(&TagsToRemove, nullptr, nullptr); // Cancel sprint ability when starting to navigate to the interactable
				
				const FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(
					InteractionData->MoveToEffectClass,
					0.0f,
					AbilitySystemComponent->MakeEffectContext()
				);

				if (SpecHandle.IsValid())
					AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
			}

			return; // Only interact with the first valid interactable object found in the overlap
		}
	}
	
	CurrentInteractableObj = nullptr; // Set the current interactable object to null if no valid interactable object was found in the overlap
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false); // End the ability if no valid interactable object was found in the overlap
}

void UGA_InteractionBase::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (!IsEndAbilityValid(Handle, ActorInfo))
		return;
	
	if (IsValid(NavigateTask))
		NavigateTask->EndTask(); // End the navigate task if it's still active when the ability ends

	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (AbilitySystemComponent && InteractionData && InteractionData->MoveToEffectClass)
		AbilitySystemComponent->RemoveActiveGameplayEffectBySourceEffect(InteractionData->MoveToEffectClass, AbilitySystemComponent);

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
#pragma endregion
	
#pragma region UFUNCTIONS
void UGA_InteractionBase::OnTargetLocationReached()
{
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (AbilitySystemComponent && InteractionData->MoveToEffectClass)
		AbilitySystemComponent->RemoveActiveGameplayEffectBySourceEffect(InteractionData->MoveToEffectClass, AbilitySystemComponent);

	NavigateTask = nullptr;

	// Play force feedback vibration when starting interaction
	if (InteractionData && InteractionData->InteractionForceFeedback)
	{
		if (APlayerController* PC = GetActorInfo().PlayerController.Get())
		{
			FForceFeedbackParameters Params;
			Params.bLooping = false;
			Params.bIgnoreTimeDilation = true;
			PC->ClientPlayForceFeedback(InteractionData->InteractionForceFeedback, Params);
		}
	}
	
	OnInteractionStart(); // Call the interaction start function to trigger any additional logic or animations in derived classes
}
#pragma endregion