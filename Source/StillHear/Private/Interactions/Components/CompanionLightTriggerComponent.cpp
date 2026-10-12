#include "Interactions/Components/CompanionLightTriggerComponent.h"

#include "Character/StillHearMainCharacter.h"
#include "TraceAndCollision/CustomCollision.h"
#include "Character/FloatingCompanionComponent.h"

UCompanionLightTriggerComponent::UCompanionLightTriggerComponent()
{
	UPrimitiveComponent::SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	UPrimitiveComponent::SetCollisionResponseToAllChannels(ECR_Ignore);
	UPrimitiveComponent::SetCollisionResponseToChannel(ECustomCollision::Player, ECR_Overlap);

	SetGenerateOverlapEvents(true);
	AllowedCollisionChannels.Add(ECustomCollision::Player);
}

void UCompanionLightTriggerComponent::OnTriggerEnter(AActor* OtherActor, UPrimitiveComponent* OtherComp)
{
	UpdateCompanionLight(OtherActor);
	IncrementTriggerCount();
}

void UCompanionLightTriggerComponent::OnTriggerExit(AActor* OtherActor, UPrimitiveComponent* OtherComp)
{
	UpdateCompanionLight(OtherActor);
}

void UCompanionLightTriggerComponent::UpdateCompanionLight(AActor* OtherActor) const
{
	if (const AStillHearMainCharacter* Player = Cast<AStillHearMainCharacter>(OtherActor))
	{
		// Calculate direction of movement relative to component's forward
		FVector PlayerVelocity = Player->GetVelocity();
		if (PlayerVelocity.IsNearlyZero())
			PlayerVelocity = Player->GetActorForwardVector();

		const float Dot = FVector::DotProduct(GetForwardVector(), PlayerVelocity.GetSafeNormal());

		// Dot > 0 means player is moving in the Forward direction of the component
		const bool bTurnOn = (Dot > 0.0f) ? bForwardTurnsOn : !bForwardTurnsOn;

		if (UFloatingCompanionComponent* Companion = Player->FindComponentByClass<UFloatingCompanionComponent>())
		{
			Companion->SetLightEnabled(bTurnOn, LightColor, bUpdateColor, bTurnOn ? LightIntensity : -1.0f);
		}
	}
}
