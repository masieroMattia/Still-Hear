// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemiesAI/BehaviorTreeTasks/Generic/BTTask_MoveToLocation.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnemiesAI/Utility/BlackboardKeyNames.h"

UBTTask_MoveToLocation::UBTTask_MoveToLocation()
{
	NodeName = TEXT("Move To Location");
	
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_MoveToLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	return EBTNodeResult::InProgress;
}

void UBTTask_MoveToLocation::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();

	if (!IsValid(Blackboard))
		return;
	
	AAIController* AIController = OwnerComp.GetAIOwner();

	if (!IsValid(AIController))
		return;
	
	FVector Target = Blackboard->GetValueAsVector(GetSelectedBlackboardKey());

	const TSubclassOf<UNavigationQueryFilter> Filter = Blackboard->GetValueAsClass(FilterClass.SelectedKeyName);
	
	AIController->MoveToLocation(
		Target,
		AcceptanceDistanceFromTarget,
		true,
		true,
		false,
		false,
		Filter,
		true
	);

	if (FVector::Dist2D(AIController->GetPawn()->GetActorLocation(), Target) <= AcceptanceDistanceFromTarget)
	{
		//Blackboard->ClearValue(GetSelectedBlackboardKey());
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
	
}


