#include "EnemiesAI/BehaviorTreeTasks/Generic/BTTask_ActivateAbilityAndWait.h"

#include "AIController.h"
#include "EnemiesAI/Pawns/Base/StillHearAICharacterBase.h"

#pragma region CONSTRUCTORS
UBTTask_ActivateAbilityAndWait::UBTTask_ActivateAbilityAndWait()
{
	NodeName = "Activate Ability And Wait";
	bNotifyTaskFinished = true;
	bCreateNodeInstance  = true;
}
#pragma endregion
	
#pragma region METHODS
// Called when the task is executed
// Tries to activate the specified ability on the AI character and waits for it to end
EBTNodeResult::Type UBTTask_ActivateAbilityAndWait::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	BehaviorTree = &OwnerComp;

	const AAIController* Controller = OwnerComp.GetAIOwner();
	if (!Controller)
		return EBTNodeResult::Failed;

	const AStillHearAICharacterBase* Pawn = Cast<AStillHearAICharacterBase>(Controller->GetPawn());
	if (!Pawn)
		return EBTNodeResult::Failed;
	
	UAbilitySystemComponent* AbilitySystemComponent = Pawn->GetAbilitySystemComponent();
	if (!AbilitySystemComponent)
		return EBTNodeResult::Failed;
	
	AbilityEndedHandle = AbilitySystemComponent->OnAbilityEnded.AddUObject(this, &UBTTask_ActivateAbilityAndWait::OnAbilityEnded);

	bAbilityEndedDuringExecute = false;
	bIsInsideExecute = true;
	const bool bActivated = AbilitySystemComponent->TryActivateAbilityByClass(AbilityClass);
	bIsInsideExecute = false;
	
	if (!bActivated)
	{
		AbilitySystemComponent->OnAbilityEnded.Remove(AbilityEndedHandle);
		return EBTNodeResult::Failed;
	}

	if (bAbilityEndedDuringExecute)
	{
		AbilitySystemComponent->OnAbilityEnded.Remove(AbilityEndedHandle);
		return EBTNodeResult::Succeeded;
	}
		
	return EBTNodeResult::InProgress;
}

// Called when the ability ends
void UBTTask_ActivateAbilityAndWait::OnAbilityEnded(const FAbilityEndedData& Data)
{
	if (!Data.AbilityThatEnded)
	{
		return;
	}
	
	if (Data.AbilityThatEnded->GetClass() != AbilityClass)
	{
		return;
	}

	if (bIsInsideExecute)
	{
		bAbilityEndedDuringExecute = true;
		return;
	}
	
	if (BehaviorTree)
		FinishLatentTask(*BehaviorTree, EBTNodeResult::Succeeded);
}

// Called when the task is finished
// Cancels the ability if it's still active and removes the delegate
void UBTTask_ActivateAbilityAndWait::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	const AAIController* Controller = OwnerComp.GetAIOwner();
	if (!Controller)
		return;

	const AStillHearAICharacterBase* Pawn = Cast<AStillHearAICharacterBase>(Controller->GetPawn());
	if (!Pawn)
		return;
	
	UAbilitySystemComponent* AbilitySystemComponent = Pawn->GetAbilitySystemComponent();
	if (!AbilitySystemComponent)
		return;
	
	if (AbilityTag.IsValid() && bShouldCancelAbilityWhenTaskEnds)
	{
		FGameplayTagContainer Tags;
		Tags.AddTag(AbilityTag);

		AbilitySystemComponent->CancelAbilities(&Tags);
	}

	AbilitySystemComponent->OnAbilityEnded.Remove(AbilityEndedHandle);
}
#pragma endregion