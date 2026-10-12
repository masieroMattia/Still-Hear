#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_StartInvestigating.generated.h"

class AAIMantisController;

UCLASS()
class STILLHEAR_API UBTTask_StartInvestigating : public UBTTaskNode
{
	GENERATED_BODY()

#pragma region VARIABLES
private:
	UPROPERTY()
	TObjectPtr<AAIMantisController> AICRef;

	UPROPERTY()
	TObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;

	FTimerHandle InvestigationTimerHandle;
#pragma endregion

#pragma region CONSTRUCTOR
public:
	UBTTask_StartInvestigating();
#pragma endregion

#pragma region METHODS
public:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	void EndInvestigation();
#pragma endregion
};
