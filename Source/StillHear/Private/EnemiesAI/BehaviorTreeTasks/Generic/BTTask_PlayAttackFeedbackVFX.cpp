// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemiesAI/BehaviorTreeTasks/Generic/BTTask_PlayAttackFeedbackVFX.h"

#include "AIController.h"
#include "EnemiesAI/Pawns/Base/StillHearAICharacterBase.h"
#include "GameplayAbilitySystem/Tags/GameplayTags.h"

UBTTask_PlayAttackFeedbackVFX::UBTTask_PlayAttackFeedbackVFX()
{
	NodeName = TEXT("Play Attack Feedback VFX");
}

EBTNodeResult::Type UBTTask_PlayAttackFeedbackVFX::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const AStillHearAICharacterBase* Pawn = Cast<AStillHearAICharacterBase>(OwnerComp.GetAIOwner()->GetPawn());

	if (!IsValid(Pawn))
	{
		return EBTNodeResult::Failed;
	}

	UAbilitySystemComponent* Asc = Pawn->GetAbilitySystemComponent();

	if (!IsValid(Asc))
	{
		return EBTNodeResult::Failed;
	}

	Asc->ExecuteGameplayCue(TAG_GameplayCue_EnemyAI_AttackFeedback);
	
	return EBTNodeResult::Succeeded;
}
