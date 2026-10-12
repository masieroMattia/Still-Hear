// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemiesAI/BehaviorTreeServices/Worm/CheckDistanceFromTarget.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "EnemiesAI/Controllers/Worm/AIWormController.h"
#include "EnemiesAI/Utility/BlackboardKeyNames.h"

UCheckDistanceFromTarget::UCheckDistanceFromTarget()
{
	NodeName = TEXT("Check Distance From Target");

	bTickIntervals = 0.3f;
	bNotifyTick = true;
}

void UCheckDistanceFromTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	const AAIWormController* AIController = Cast<AAIWormController>(OwnerComp.GetAIOwner());

	if (!IsValid(AIController))
		return;

	AAIWormCharacter* AICharacter = Cast<AAIWormCharacter>(AIController->GetWormRef());

	if (!IsValid(AICharacter))
		return;
	
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	
	FVector Target = Blackboard->GetValueAsVector(GetSelectedBlackboardKey());
	float DistanceFromTarget = (AICharacter->GetActorLocation() - Target).Size();
	
	if (DistanceFromTarget < AIController->GetDataAsset()->MaxDolphinDiveDistance
		&& DistanceFromTarget > AIController->GetDataAsset()->MinDolphinDiveDistance)
	{
		Blackboard->SetValueAsBool(BlackboardKeyNames::KeyNameIsCloseEnoughToTarget, true);
	}
}
