// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemiesAI/BehaviorTreeTasks/Worm/BTTask_MoveLookAtPosToTarget.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "EnemiesAI/Controllers/Worm/AIWormController.h"
#include "EnemiesAI/Utility/Patrol/Waypoint.h"

UBTTask_MoveLookAtPosToTarget::UBTTask_MoveLookAtPosToTarget()
{
	NodeName = TEXT("Move LookAtPos Component To Target");
}

EBTNodeResult::Type UBTTask_MoveLookAtPosToTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{	
	const AAIWormController* AIController = Cast<AAIWormController>(OwnerComp.GetAIOwner());

	if (!IsValid(AIController))
		return EBTNodeResult::Failed;

	const AAIWormCharacter* AICharacter = AIController->GetWormRef();

	if (!IsValid(AICharacter))
		return EBTNodeResult::Failed;

	const UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();

	if (!IsValid(Blackboard))
		return EBTNodeResult::Failed;

	const AActor* Target = Cast<AActor>(Blackboard->GetValueAsObject(GetSelectedBlackboardKey()));

	if (!IsValid(Target))
		return EBTNodeResult::Failed;

	const FVector DirectionToTarget = (Target->GetActorLocation() - AICharacter->GetActorLocation()).GetSafeNormal();
	
	AICharacter->SetLookAtPosLocation(Target->GetActorLocation() + DirectionToTarget * 150.0f);
	
	return EBTNodeResult::Succeeded;
}
