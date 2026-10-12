#include "Interactions/Components/BlueprintTriggerComponent.h"

#pragma region METHODS
void UBlueprintTriggerComponent::OnTriggerEnter(AActor* OtherActor, UPrimitiveComponent* OtherComp)
{
	OnTriggerEnterEvent.Broadcast(OtherActor, OtherComp);
}

void UBlueprintTriggerComponent::OnTriggerExit(AActor* OtherActor, UPrimitiveComponent* OtherComp)
{
	OnTriggerExitEvent.Broadcast(OtherActor, OtherComp);
}

void UBlueprintTriggerComponent::ConsumeTrigger()
{
	IncrementTriggerCount();
}
#pragma endregion
