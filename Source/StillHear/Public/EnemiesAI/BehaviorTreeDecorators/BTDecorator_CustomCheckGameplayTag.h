#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_CustomCheckGameplayTag.generated.h"

UCLASS()
class STILLHEAR_API UBTDecorator_CustomCheckGameplayTag : public UBTDecorator
{
	GENERATED_BODY()
	
#pragma region UPROPERTIES
public:
	UPROPERTY(EditAnywhere, Category = "GameplayTags")
	FGameplayTag TagToCheck;
#pragma endregion

#pragma region CONSTRUCTORS
public:
	UBTDecorator_CustomCheckGameplayTag();
#pragma endregion
	
#pragma region METHODS
protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
#pragma endregion
};
