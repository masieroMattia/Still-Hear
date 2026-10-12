#include "UI/Indicator/IndicatorTriggerComponent.h"
#include "UI/Indicator/IndicatorComponent.h"

#pragma region METHODS
void UIndicatorTriggerComponent::OnTriggerEnter(AActor* OtherActor, UPrimitiveComponent* OtherComp)
{
	if (bActivateOnEnter)
	{
		ApplyToAllIndicators(true);
		IncrementTriggerCount();
	}
}

void UIndicatorTriggerComponent::OnTriggerExit(AActor* OtherActor, UPrimitiveComponent* OtherComp)
{
	if (bDeactivateOnExit)
		ApplyToAllIndicators(false);
}

void UIndicatorTriggerComponent::ApplyToAllIndicators(const bool bRegister)
{
	for (const TSoftObjectPtr<AActor>& SoftActor : TargetActors)
	{
		const AActor* Target = SoftActor.Get();
		if (!IsValid(Target))
			continue;

		TArray<UIndicatorComponent*> IndicatorComponents;
		Target->GetComponents<UIndicatorComponent>(IndicatorComponents);

		for (UIndicatorComponent* Comp : IndicatorComponents)
		{
			if (!IsValid(Comp))
				continue;

			if (bRegister)
				Comp->RegisterIndicator();
			else
				Comp->UnregisterIndicator();
		}
	}
}
#pragma endregion

