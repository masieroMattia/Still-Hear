#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_StartHunting.generated.h"

class AAIMantisController;

UCLASS()
class STILLHEAR_API UBTTask_StartHunting : public UBTTaskNode
{
	GENERATED_BODY()

#pragma region VARIABLES
private:
	UPROPERTY()
	TObjectPtr<AAIMantisController> AICRef;

	UPROPERTY()
	TObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;

	FTimerHandle HuntingTimerHandle;
#pragma endregion

#pragma region CONSTRUCTOR
public:
	UBTTask_StartHunting();
#pragma endregion

#pragma region METHODS
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	void EndHunting();
#pragma endregion
};
