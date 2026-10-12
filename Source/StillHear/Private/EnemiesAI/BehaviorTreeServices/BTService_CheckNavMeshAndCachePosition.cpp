// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemiesAI/BehaviorTreeServices/BTService_CheckNavMeshAndCachePosition.h"

#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnemiesAI/Utility/BlackboardKeyNames.h"

UBTService_CheckNavMeshAndCachePosition::UBTService_CheckNavMeshAndCachePosition()
{
	NodeName = TEXT("Check NavMesh and Cache Position");

	bTickIntervals = 0.2f;
	bNotifyTick = true;
}

void UBTService_CheckNavMeshAndCachePosition::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	FNavLocation ProjectedLocation;

	// Checking if the current location is on the NavMesh and caching if it is
	const FVector QueryExtent = FVector(10.f, 10.f, 100.f);

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	
	if (NavSys->ProjectPointToNavigation(OwnerComp.GetAIOwner()->GetPawn()->GetActorLocation(), ProjectedLocation, QueryExtent))
	{
		Blackboard->SetValueAsBool(BlackboardKeyNames::KeyNameIsOffNavMesh, false);
		return;
	}

	const FVector PawnLocation = OwnerComp.GetAIOwner()->GetPawn()->GetActorLocation();
	
	for (const float Extent : {100.f, 200.f, 400.f, 800.f, 1500.f})
	{
		if (NavSys->ProjectPointToNavigation(PawnLocation, ProjectedLocation, FVector(Extent, Extent, Extent)) && !IsProjectedPointUnderPawn(PawnLocation, ProjectedLocation.Location))
		{
			break;
		}

		if (NavSys->ProjectPointToNavigation(FVector(PawnLocation.X + Extent, PawnLocation.Y, PawnLocation.Z), ProjectedLocation, FVector(Extent, Extent, Extent)) && !IsProjectedPointUnderPawn(PawnLocation, ProjectedLocation.Location))
		{
			break;
		}

		if (NavSys->ProjectPointToNavigation(FVector(PawnLocation.X - Extent, PawnLocation.Y, PawnLocation.Z), ProjectedLocation, FVector(Extent, Extent, Extent)) && !IsProjectedPointUnderPawn(PawnLocation, ProjectedLocation.Location))
		{
			break;
		}

		if (NavSys->ProjectPointToNavigation(FVector(PawnLocation.X, PawnLocation.Y + Extent, PawnLocation.Z), ProjectedLocation, FVector(Extent, Extent, Extent)) && !IsProjectedPointUnderPawn(PawnLocation, ProjectedLocation.Location))
		{
			break;
		}
		
		if (NavSys->ProjectPointToNavigation(FVector(PawnLocation.X, PawnLocation.Y - Extent, PawnLocation.Z), ProjectedLocation, FVector(Extent, Extent, Extent)) && !IsProjectedPointUnderPawn(PawnLocation, ProjectedLocation.Location))
		{
			break;
		}		
	}
	Blackboard->SetValueAsVector(BlackboardKeyNames::KeyNameNearestSafeLocation, ProjectedLocation.Location);
	Blackboard->SetValueAsBool(BlackboardKeyNames::KeyNameIsOffNavMesh, true);
}

bool UBTService_CheckNavMeshAndCachePosition::IsProjectedPointUnderPawn(const FVector& PawnLocation, const FVector& ProjectedLocation) const
{
	return ProjectedLocation.X <= PawnLocation.X + 25.f && ProjectedLocation.X >= PawnLocation.X - 25.f && ProjectedLocation.Y <= PawnLocation.Y + 25.f && ProjectedLocation.Y >= PawnLocation.Y - 25.f;
}
