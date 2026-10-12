#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_RotateTowards.generated.h"

UCLASS(HideCategories = ("Node", "Blackboard"))
class STILLHEAR_API UBTTask_RotateTowards : public UBTTaskNode
{
	GENERATED_BODY()
	
#pragma region UPROPERTIES
public:
	// Blackboard Key for the target location or actor
	UPROPERTY(EditAnywhere, Category = "Task")
	FBlackboardKeySelector TargetLocationKey;

	// Is the target an Actor?
	UPROPERTY(EditAnywhere, Category = "Task")
	bool bActor;

	// Allowed error in degrees to consider the rotation complete
	UPROPERTY(EditAnywhere, Category = "Task")
	float ErrorTolerance;
#pragma endregion
	
#pragma region VARIABLES
private:
	// Cached location to rotate towards
	FVector TargetLocation;
#pragma endregion
	
#pragma region CONSTRUCTOR
public:
	UBTTask_RotateTowards();
#pragma endregion
	
#pragma region METHODS
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
#pragma endregion
};
