#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BehaviorTree/Decorators/BTDecorator_BlackboardBase.h"
#include "BTD_CheckAIStatus.generated.h"

UCLASS()
class STILLHEAR_API UBTD_CheckAIStatus : public UBTDecorator_BlackboardBase
{
	GENERATED_BODY()

#pragma region UPROPERTIES
public:
	UPROPERTY(EditAnywhere, meta = (Categories = "Status.EnemyAI"), Category = "Tag")
	FGameplayTag TagToCheck;
#pragma endregion

#pragma region CONSTRUCTOR
public:
	UBTD_CheckAIStatus();
#pragma endregion

#pragma region METHODS
protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
#pragma endregion 
};
