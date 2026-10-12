// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_UpdateTargetLocationFromActor.generated.h"

/**
 * 
 */
UCLASS()
class STILLHEAR_API UBTTask_UpdateTargetLocationFromActor : public UBTTaskNode
{
	GENERATED_BODY()

#pragma region CONSTRUCTOR
public:
	UBTTask_UpdateTargetLocationFromActor();
#pragma endregion

#pragma region METHODS
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
#pragma endregion 
};
