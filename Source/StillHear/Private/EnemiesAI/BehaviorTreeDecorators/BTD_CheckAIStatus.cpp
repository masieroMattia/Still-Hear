#include "EnemiesAI/BehaviorTreeDecorators/BTD_CheckAIStatus.h"

#include "EnemiesAI/Controllers/Base/StillHearAIControllerBase.h"

UBTD_CheckAIStatus::UBTD_CheckAIStatus()
{
	NodeName = "Check AI Status Tag";
}

bool UBTD_CheckAIStatus::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const AStillHearAIControllerBase* AIController = Cast<AStillHearAIControllerBase>(OwnerComp.GetAIOwner());
	
	// Neutralize the Editor's 'Inverse Condition' toggle to safely abort if the pointer is invalid
	if (!IsValid(AIController))
		return IsInversed();

	const FGameplayTag CurrentTag = AIController->GetCurrentStatusTag();
	const bool bRawResult = TagToCheck.MatchesTagExact(CurrentTag);

	// Directly return the boolean evaluation
	return bRawResult;
}
