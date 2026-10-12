#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_PlayMontage.generated.h"

UCLASS(HideCategories = ("Node", "Blackboard"))
class STILLHEAR_API UBTTask_PlayMontage : public UBTTaskNode
{
	GENERATED_BODY()
	
#pragma region UPROPERTIES
public:
	UPROPERTY(EditAnywhere, Category = "Task")
	UAnimMontage* MontageToPlay;

	UPROPERTY(EditAnywhere, Category = "Task")
	float PlayRate;

	UPROPERTY(EditAnywhere, Category = "Task")
	bool bWaitAnimationComplete;
#pragma endregion
	
#pragma region VARIABLES
private:
	UPROPERTY()
	TObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;
#pragma endregion
	
#pragma region CONSTRUCTOR
public:
	UBTTask_PlayMontage();
#pragma endregion
	
#pragma region METHODS
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	// Callback function when montage finishes
	void OnMontageFinished(UAnimMontage* Montage, bool bInterrupted, UBehaviorTreeComponent* OwnerComp) const;
#pragma endregion
};
