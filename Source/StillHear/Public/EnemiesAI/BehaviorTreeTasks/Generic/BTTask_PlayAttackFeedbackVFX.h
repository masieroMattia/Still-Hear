// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_PlayAttackFeedbackVFX.generated.h"

/**
 * 
 */
UCLASS()
class STILLHEAR_API UBTTask_PlayAttackFeedbackVFX : public UBTTaskNode
{
	GENERATED_BODY()

#pragma region CONSTRUCTOR
public:
	UBTTask_PlayAttackFeedbackVFX();
#pragma endregion
	
#pragma region METHODS
public:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
#pragma endregion
};
