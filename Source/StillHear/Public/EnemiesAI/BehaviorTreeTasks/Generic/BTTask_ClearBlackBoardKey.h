#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_ClearBlackBoardKey.generated.h"

UCLASS(HideCategories = ("BlackBoard"))
class STILLHEAR_API UBTTask_ClearBlackBoardKey : public UBTTaskNode
{
	GENERATED_BODY()

#pragma region UPROPERTIES
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keys")
	FBlackboardKeySelector KeyToClear;
#pragma endregion
	
#pragma region CONSTRUCTOR
public:
	UBTTask_ClearBlackBoardKey();
#pragma endregion
	
#pragma region METHODS
public:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
#pragma endregion
};
