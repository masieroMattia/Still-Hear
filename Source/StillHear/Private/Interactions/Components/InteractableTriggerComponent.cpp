#include "Interactions/Components/InteractableTriggerComponent.h"

#include "Interactions/Actors/InteractableObj.h"

#pragma region METHODS
void UInteractableTriggerComponent::OnTriggerEnter(AActor* OtherActor, UPrimitiveComponent* OtherComp)
{
	if (TriggerActivation == ETriggerActivationEvent::OnExit)
		return;

	ActivateInteractables();
}

void UInteractableTriggerComponent::OnTriggerExit(AActor* OtherActor, UPrimitiveComponent* OtherComp)
{
	if (TriggerActivation == ETriggerActivationEvent::OnEnter)
		return;

	ActivateInteractables();
}

void UInteractableTriggerComponent::ActivateInteractables()
{
	for (const TSoftObjectPtr<AInteractableObj>& SoftRef : InteractableObjects)
	{
		// Resolve the soft reference; returns nullptr if the object is not loaded / invalid
		AInteractableObj* Interactable = SoftRef.Get();
		if (!Interactable)
			continue;

		Interactable->TriggerInteraction(GetOwner());
	}

	// Notify the base class that an activation occurred
	IncrementTriggerCount();
}
#pragma endregion