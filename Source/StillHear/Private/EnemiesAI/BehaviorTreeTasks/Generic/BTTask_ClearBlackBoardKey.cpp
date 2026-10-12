#include "EnemiesAI/BehaviorTreeTasks/Generic/BTTask_ClearBlackBoardKey.h"

#include "BehaviorTree/BlackboardComponent.h"

#pragma region CONSTRUCTOR
UBTTask_ClearBlackBoardKey::UBTTask_ClearBlackBoardKey()
{
	NodeName = "Clear BlackBoard Key";
}
#pragma endregion
	
#pragma region METHODS
// Called when the task is executed
// Clears the specified Object key in the Blackboard
EBTNodeResult::Type UBTTask_ClearBlackBoardKey::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	KeyToClear.ResolveSelectedKey(*Blackboard->GetBlackboardAsset()); // Make sure the key refers to a valid blackboard entry

	OwnerComp.GetBlackboardComponent()->ClearValue(KeyToClear.SelectedKeyName);
	
	return EBTNodeResult::Succeeded;
}
#pragma endregion