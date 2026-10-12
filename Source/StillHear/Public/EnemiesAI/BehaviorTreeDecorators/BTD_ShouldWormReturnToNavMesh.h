// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTD_ShouldWormReturnToNavMesh.generated.h"

/**
 * 
 */
UCLASS()
class STILLHEAR_API UBTD_ShouldWormReturnToNavMesh : public UBTDecorator
{
	GENERATED_BODY()

#pragma region CONSTRUCTOR
public:
	UBTD_ShouldWormReturnToNavMesh();
#pragma endregion

#pragma region METHODS
protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
#pragma endregion
};
