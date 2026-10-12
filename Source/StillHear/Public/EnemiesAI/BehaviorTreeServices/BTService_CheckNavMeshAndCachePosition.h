// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_CheckNavMeshAndCachePosition.generated.h"

/**
 * 
 */
UCLASS()
class STILLHEAR_API UBTService_CheckNavMeshAndCachePosition : public UBTService
{
	GENERATED_BODY()

#pragma region CONSTRUCTOR
public:
	UBTService_CheckNavMeshAndCachePosition();
#pragma endregion

#pragma region METHODS
protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	bool IsProjectedPointUnderPawn(const FVector& PawnLocation, const FVector& ProjectedLocation) const;
#pragma endregion 
};
