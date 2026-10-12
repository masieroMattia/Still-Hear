// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemiesAI/BehaviorTreeTasks/Generic/BTTask_SetNextWaypoint.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "EnemiesAI/Controllers/Base/StillHearAIControllerBase.h"
#include "EnemiesAI/Utility/BlackboardKeyNames.h"
#include "EnemiesAI/Utility/Patrol/Waypoint.h"

UBTTask_SetNextWaypoint::UBTTask_SetNextWaypoint()
{
	NodeName = TEXT("Set Next Waypoint");
}

EBTNodeResult::Type UBTTask_SetNextWaypoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const AStillHearAIControllerBase* AIController = Cast<AStillHearAIControllerBase>(OwnerComp.GetAIOwner());

	if (!IsValid(AIController))
		return EBTNodeResult::Failed;

	AStillHearAICharacterBase* AICharacter = AIController->GetNPCRef();

	if (!IsValid(AICharacter))
		return EBTNodeResult::Failed;

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();

	if (!IsValid(Blackboard))
		return EBTNodeResult::Failed;

	AWaypoint* CurrentWaypoint = AICharacter->GetCurrentWaypoint();
	if (!IsValid(CurrentWaypoint))
		return EBTNodeResult::Failed;

	AWaypoint* NextWaypoint = CurrentWaypoint->GetNextWaypoint();
	if (!IsValid(NextWaypoint))
		return EBTNodeResult::Failed;

	Blackboard->SetValueAsObject(BlackboardKeyNames::KeyNameCurrentWaypoint, NextWaypoint);
	Blackboard->SetValueAsFloat(BlackboardKeyNames::KeyNameWaypointWaitTime, NextWaypoint->GetWaitTime());
	AICharacter->SetCurrentWaypoint(NextWaypoint);

	return EBTNodeResult::Succeeded;
}
