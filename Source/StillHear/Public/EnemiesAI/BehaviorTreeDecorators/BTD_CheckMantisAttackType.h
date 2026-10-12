#pragma once

#include "CoreMinimal.h"
#include "EnemiesAI/Utility/AIEnum.h"
#include "BehaviorTree/Decorators/BTDecorator_BlackboardBase.h"
#include "BTD_CheckMantisAttackType.generated.h"

/*
 * Decorator that checks the MantisAttackType blackboard key against a specified value.
 * Use on each attack branch so the BT only enters the branch when the service has decided that attack.
 */
UCLASS()
class STILLHEAR_API UBTD_CheckMantisAttackType : public UBTDecorator_BlackboardBase
{
	GENERATED_BODY()

#pragma region UPROPERTIES
public:
	UPROPERTY(EditAnywhere, Category = "Attack")
	E_MantisAttackType AttackTypeToCheck = E_MantisAttackType::NONE;
#pragma endregion

#pragma region CONSTRUCTOR
public:
	UBTD_CheckMantisAttackType();
#pragma endregion

#pragma region METHODS
protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
#pragma endregion
};

