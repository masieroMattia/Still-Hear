// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "CheckDistanceFromTarget.generated.h"

/**
 * 
 */
UCLASS()
class STILLHEAR_API UCheckDistanceFromTarget : public UBTService_BlackboardBase
{
	GENERATED_BODY()

#pragma region CONSTRUCTOR
public:
	UCheckDistanceFromTarget();
#pragma endregion 

#pragma region METHODS
protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
#pragma endregion 
};
