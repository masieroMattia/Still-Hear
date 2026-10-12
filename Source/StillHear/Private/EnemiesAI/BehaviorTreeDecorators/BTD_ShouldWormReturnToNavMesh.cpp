// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemiesAI/BehaviorTreeDecorators/BTD_ShouldWormReturnToNavMesh.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "EnemiesAI/Utility/BlackboardKeyNames.h"

UBTD_ShouldWormReturnToNavMesh::UBTD_ShouldWormReturnToNavMesh()
{
	NodeName = TEXT("Should Worm Return To NavMesh");
}

bool UBTD_ShouldWormReturnToNavMesh::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();

	const bool bIsOffNavMesh = Blackboard->GetValueAsBool(BlackboardKeyNames::KeyNameIsOffNavMesh);
	const bool bIsDiving = Blackboard->GetValueAsBool(BlackboardKeyNames::KeyNameIsDiving);

	return bIsOffNavMesh && !bIsDiving;
}
