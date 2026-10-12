// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SetNextWaypoint.generated.h"

/**
 * 
 */
UCLASS()
class STILLHEAR_API UBTTask_SetNextWaypoint : public UBTTaskNode
{
	GENERATED_BODY()

#pragma region CONSTRUCTOR
public:
	UBTTask_SetNextWaypoint();
#pragma endregion

#pragma region METHODS
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
#pragma endregion
};
