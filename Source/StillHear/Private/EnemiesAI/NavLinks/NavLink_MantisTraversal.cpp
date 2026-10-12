#include "EnemiesAI/NavLinks/NavLink_MantisTraversal.h"

#include "AbilitySystemGlobals.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnemiesAI/Pawns/Mantis/AIMantisCharacter.h"
#include "EnemiesAI/Controllers/Base/StillHearAIControllerBase.h"

ANavLink_MantisTraversal::ANavLink_MantisTraversal()
{
	PrimaryActorTick.bCanEverTick = false;
	bSmartLinkIsRelevant = true;
	PointLinks.Empty();
}

void ANavLink_MantisTraversal::BeginPlay()
{
	Super::BeginPlay();
	
	OnSmartLinkReached.AddUniqueDynamic(this, &ThisClass::NotifySmartLinkReached);
}

void ANavLink_MantisTraversal::DisableLinkTemporarily(const float Duration)
{
	// Guard: only call SetSmartLinkEnabled when the state actually changes
	if (bSmartLinkIsRelevant)
		SetSmartLinkEnabled(false);

	// Always refresh the re-enable timer (extends it if already running)
	GetWorld()->GetTimerManager().SetTimer(
		LinkReenableTimerHandle,
		[this]() { SetSmartLinkEnabled(true); },
		Duration,
		false
	);
}

void ANavLink_MantisTraversal::NotifySmartLinkReached(AActor* MovingActor, const FVector& DestinationPoint)
{
	AAIMantisCharacter* AICharacter = Cast<AAIMantisCharacter>(MovingActor);
	if (!IsValid(AICharacter))
		return;

	// Already performing a traversal->skip
	if (AICharacter->IsPerformingNavLinkJump())
		return;

	const UCharacterMovementComponent* MoveComp = AICharacter->GetCharacterMovement();

	// Falling for an unrelated reason->reroute and wait
	if (MoveComp && MoveComp->IsFalling())
	{
		DisableLinkTemporarily(LinkDisableDuration);
		return;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(AICharacter);
	if (ASC)
	{
		// Fire a GameplayEvent to trigger the Shift ability for traversal
		FGameplayEventData Payload;
		Payload.EventTag = FGameplayTag::RequestGameplayTag(FName("GameplayAbility.EnemyAI.MantisShift.Nav"));
		Payload.Instigator = this;
		Payload.Target = AICharacter;

		// Send the DestinationPoint inside TargetData
		FGameplayAbilityTargetData_LocationInfo* LocationInfo = new FGameplayAbilityTargetData_LocationInfo();
		LocationInfo->TargetLocation.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;
		LocationInfo->TargetLocation.LiteralTransform = FTransform(AICharacter->GetActorRotation(), DestinationPoint);
		Payload.TargetData.Add(LocationInfo);

		ASC->HandleGameplayEvent(Payload.EventTag, &Payload);

		// Disable link temporarily to prevent multiple triggers
		DisableLinkTemporarily(LinkDisableDuration);
	}
}
