// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemiesAI/BehaviorTreeServices/Worm/CheckDistanceFromActor.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "EnemiesAI/Controllers/Worm/AIWormController.h"
#include "EnemiesAI/Utility/BlackboardKeyNames.h"
#include "GameplayAbilitySystem/Tags/GameplayTags.h"

UCheckDistanceFromActor::UCheckDistanceFromActor()
{
	NodeName = TEXT("Check Distance From Actor");

	bTickIntervals = 0.3f;
	bNotifyTick = true;
}

void UCheckDistanceFromActor::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	const AAIWormController* AIController = Cast<AAIWormController>(OwnerComp.GetAIOwner());

	if (!IsValid(AIController))
		return;

	AAIWormCharacter* AICharacter = Cast<AAIWormCharacter>(AIController->GetWormRef());

	if (!IsValid(AICharacter))
		return;
	
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();

	const AActor* TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(GetSelectedBlackboardKey()));

	if (!IsValid(TargetActor))
	{
		return;
	}
	
	const float DistanceFromTarget = (AICharacter->GetActorLocation() - TargetActor->GetActorLocation()).Size();

	const bool bIsDiveInCooldown = AICharacter->GetAbilitySystemComponent()->HasMatchingGameplayTag(TAG_Status_EnemyAI_WormDolphinDiveCooldown);
	
	if (DistanceFromTarget < AIController->GetDataAsset()->MaxDolphinDiveDistance
		&& DistanceFromTarget > AIController->GetDataAsset()->MinDolphinDiveDistance
		&& !bIsDiveInCooldown)
	{
		Blackboard->SetValueAsBool(BlackboardKeyNames::KeyNameIsCloseEnoughToTarget, true);
	}
}
