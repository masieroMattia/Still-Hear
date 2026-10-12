// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemiesAI/BehaviorTreeTasks/Worm/BTTask_UpdateTargetLocationFromActor.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Character/StillHearMainCharacter.h"
#include "EnemiesAI/Utility/BlackboardKeyNames.h"

UBTTask_UpdateTargetLocationFromActor::UBTTask_UpdateTargetLocationFromActor()
{
	NodeName = TEXT("Update Target Location From Actor");
}

EBTNodeResult::Type UBTTask_UpdateTargetLocationFromActor::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();

	const AStillHearMainCharacter* MainCharacter = Cast<AStillHearMainCharacter>(Blackboard->GetValueAsObject(BlackboardKeyNames::KeyNameTargetActor));

	if (!MainCharacter)
		return EBTNodeResult::Failed;

	Blackboard->SetValueAsVector(BlackboardKeyNames::KeyNameTargetLocation, MainCharacter->GetActorLocation());
	Blackboard->ClearValue(BlackboardKeyNames::KeyNameTargetActor);
	
	return EBTNodeResult::Succeeded;
}


