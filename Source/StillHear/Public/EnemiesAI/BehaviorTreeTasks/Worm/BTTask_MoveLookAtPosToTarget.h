// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_MoveLookAtPosToTarget.generated.h"

/**
 * 
 */
UCLASS()
class STILLHEAR_API UBTTask_MoveLookAtPosToTarget : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	
#pragma region CONSTRUCTOR
public:
	UBTTask_MoveLookAtPosToTarget();
#pragma endregion

#pragma region METHODS
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
#pragma endregion 
};
