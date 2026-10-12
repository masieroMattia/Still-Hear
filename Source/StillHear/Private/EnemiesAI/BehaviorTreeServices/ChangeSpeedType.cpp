// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemiesAI/BehaviorTreeServices/ChangeSpeedType.h"

#include "EnemiesAI/Controllers/Base/StillHearAIControllerBase.h"
#include "EnemiesAI/Pawns/Base/StillHearAICharacterBase.h"

UChangeSpeedType::UChangeSpeedType()
{
	NodeName = TEXT("Change Speed Type");

	bNotifyTick = false;
	bNotifyBecomeRelevant = true;
}

void UChangeSpeedType::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const AStillHearAIControllerBase* AIController = Cast<AStillHearAIControllerBase>(OwnerComp.GetAIOwner());

	if (!IsValid(AIController))
		return;
	
	AStillHearAICharacterBase* AICharacter = AIController->GetNPCRef();

	if (!IsValid(AICharacter))
		return;

	AICharacter->ChangeSpeedType(SpeedType);
}
