// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ReturnToNavMesh.generated.h"

/**
 * 
 */
struct FNavMeshRecoveryMemory { float Elapsed = 0.f; };

UCLASS()
class STILLHEAR_API UBTTask_ReturnToNavMesh : public UBTTaskNode
{
	GENERATED_BODY()

#pragma region UPROPERTY
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float MaxRecoverySeconds;
#pragma endregion 
	
#pragma region CONSTRUCTOR
public:
	UBTTask_ReturnToNavMesh();
#pragma endregion

#pragma region METHODS
public:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual uint16 GetInstanceMemorySize() const override;
#pragma endregion 
};
