#include "EnemiesAI/BehaviorTreeTasks/Generic/BTTask_PlayMontage.h"

#include "AIController.h"
#include "GameFramework/Character.h"

UBTTask_PlayMontage::UBTTask_PlayMontage()
{
	NodeName = "Play Montage";
	bCreateNodeInstance = true; 
	PlayRate = 1.0f;
	bWaitAnimationComplete = true;
}

EBTNodeResult::Type UBTTask_PlayMontage::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController || !MontageToPlay) 
		return EBTNodeResult::Failed;

	ACharacter* Character = Cast<ACharacter>(AIController->GetPawn());
	if (!Character) 
		return EBTNodeResult::Failed;

	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if (!AnimInstance) 
		return EBTNodeResult::Failed;

	CachedOwnerComp = &OwnerComp;

	// Play the montage
	const float Duration = Character->PlayAnimMontage(MontageToPlay, PlayRate);
	if (Duration > 0.0f && bWaitAnimationComplete)
	{
		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &UBTTask_PlayMontage::OnMontageFinished, CachedOwnerComp.Get());
		AnimInstance->Montage_SetEndDelegate(EndDelegate, MontageToPlay);
		return EBTNodeResult::InProgress;
	}

	return EBTNodeResult::Succeeded;
}

EBTNodeResult::Type UBTTask_PlayMontage::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController)
	{
		ACharacter* Character = Cast<ACharacter>(AIController->GetPawn());
		if (Character) 
			Character->StopAnimMontage(MontageToPlay);
	}

	return EBTNodeResult::Aborted;
}

void UBTTask_PlayMontage::OnMontageFinished(UAnimMontage* Montage, bool bInterrupted, UBehaviorTreeComponent* OwnerComp) const
{
	if (OwnerComp) FinishLatentTask(*OwnerComp, EBTNodeResult::Succeeded);
}
