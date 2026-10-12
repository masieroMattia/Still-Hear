#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "Abilities/GameplayAbility.h"
#include "BTTask_ActivateAbilityAndWait.generated.h"

UCLASS(HideCategories = ("Blackboard"))
class STILLHEAR_API UBTTask_ActivateAbilityAndWait : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	
#pragma region UPROPERTIES
public:
	UPROPERTY(EditAnywhere, Category = "Ability")
	TSubclassOf<UGameplayAbility> AbilityClass; // The class of the ability to activate
	UPROPERTY(EditAnywhere, Category="Ability")
	FGameplayTag AbilityTag; // The tag of the ability to activate
	UPROPERTY(EditAnywhere, Category = "Ability")
	bool bShouldCancelAbilityWhenTaskEnds;
#pragma endregion
	
#pragma region VARIABLES
private:
	UPROPERTY()
	TObjectPtr<UBehaviorTreeComponent> BehaviorTree;

	FDelegateHandle AbilityEndedHandle;

	bool bIsInsideExecute = false;
	bool bAbilityEndedDuringExecute = false;
#pragma endregion

#pragma region CONSTRUCTORS
public:
	UBTTask_ActivateAbilityAndWait();
#pragma endregion
	
#pragma region METHODS
public:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
	
private:
	void OnAbilityEnded(const FAbilityEndedData& Data);
#pragma endregion
};
