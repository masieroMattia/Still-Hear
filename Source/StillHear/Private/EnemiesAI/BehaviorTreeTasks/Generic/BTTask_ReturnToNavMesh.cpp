// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemiesAI/BehaviorTreeTasks/Generic/BTTask_ReturnToNavMesh.h"

#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnemiesAI/Utility/BlackboardKeyNames.h"

UBTTask_ReturnToNavMesh::UBTTask_ReturnToNavMesh()
{
	NodeName = TEXT("Return to NavMesh");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_ReturnToNavMesh::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FNavMeshRecoveryMemory* Memory = reinterpret_cast<FNavMeshRecoveryMemory*>(NodeMemory);
	Memory->Elapsed = 0.f;

	const AAIController* AIController = OwnerComp.GetAIOwner();
	const UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();

	if (!AIController || !Blackboard)
	{
		return EBTNodeResult::Failed;
	}

	const FVector LocationToReturnTo = Blackboard->GetValueAsVector(BlackboardKeyNames::KeyNameNearestSafeLocation);
	
	OwnerComp.GetAIOwner()->MoveToLocation(
		FVector(LocationToReturnTo.X, LocationToReturnTo.Y, LocationToReturnTo.Z),
		50.f,
		true,
		false,
		false
	);

	return EBTNodeResult::InProgress;
}

void UBTTask_ReturnToNavMesh::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FNavMeshRecoveryMemory* Memory = reinterpret_cast<FNavMeshRecoveryMemory*>(NodeMemory);

	const AAIController* AIController = OwnerComp.GetAIOwner();
	const APawn* Pawn = AIController ? AIController->GetPawn() : nullptr;
	if (!Pawn)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// Did I successfully return to the NavMesh?
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(Pawn->GetWorld());
	FNavLocation ProjectedLocation;
	if (NavSys && NavSys->ProjectPointToNavigation(Pawn->GetActorLocation(), ProjectedLocation, FVector(10.f, 10.f, 100.f)))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	// Fail-safe timeout
	Memory->Elapsed += DeltaSeconds;
	if (Memory->Elapsed >= MaxRecoverySeconds)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
}

uint16 UBTTask_ReturnToNavMesh::GetInstanceMemorySize() const
{
	return sizeof(FNavMeshRecoveryMemory);
}
