#include "EnemiesAI/BehaviorTreeDecorators/BTD_CheckMantisAttackType.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "EnemiesAI/Utility/BlackboardKeyNames.h"

UBTD_CheckMantisAttackType::UBTD_CheckMantisAttackType()
{
	NodeName = "Check Mantis Attack Type";
}

bool UBTD_CheckMantisAttackType::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
		return IsInversed(); // If the Blackboard Component is not valid, return true if the condition is inversed, false otherwise

	const uint8 CurrentValue = BB->GetValueAsEnum(BlackboardKeyNames::KeyNameAttackType);
	return CurrentValue == static_cast<uint8>(AttackTypeToCheck);
}

