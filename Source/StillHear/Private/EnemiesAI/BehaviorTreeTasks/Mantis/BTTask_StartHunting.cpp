#include "EnemiesAI/BehaviorTreeTasks/Mantis/BTTask_StartHunting.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "EnemiesAI/Controllers/Mantis/AIMantisController.h"
#include "EnemiesAI/Utility/BlackboardKeyNames.h"
#include "EnemiesAI/Utility/Components/PerceptionsMeterComponent.h"
#include "EnemiesAI/Utility/DataAssets/AIMantisInfo_DataAsset.h"

#pragma region CONSTRUCTOR
UBTTask_StartHunting::UBTTask_StartHunting()
{
	NodeName = TEXT("Start Hunting");
	bCreateNodeInstance = true;
}
#pragma endregion

#pragma region METHODS
EBTNodeResult::Type UBTTask_StartHunting::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Cast Owner Controller to AIMantisController
	AICRef = Cast<AAIMantisController>(OwnerComp.GetAIOwner());
	if (!IsValid(AICRef))
		return EBTNodeResult::Failed;

	CachedOwnerComp = &OwnerComp;

	// Get NPC Ref → Get Data Asset → HuntingTimer
	const AStillHearAICharacterBase* NPCRef = AICRef->GetNPCRef();
	if (!IsValid(NPCRef))
		return EBTNodeResult::Failed;

	const UAIMantisInfo_DataAsset* DataAsset = Cast<UAIMantisInfo_DataAsset>(NPCRef->GetAIInfo_DataAsset());
	if (!IsValid(DataAsset))
		return EBTNodeResult::Failed;

	// Set Timer by Event → EndHunting callback
	GetWorld()->GetTimerManager().SetTimer(
		HuntingTimerHandle,
		this,
		&UBTTask_StartHunting::EndHunting,
		DataAsset->HuntingTimer,
		false
	);

	return EBTNodeResult::InProgress;
}

EBTNodeResult::Type UBTTask_StartHunting::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Clear and Invalidate Timer by Handle
	GetWorld()->GetTimerManager().ClearTimer(HuntingTimerHandle);

	return EBTNodeResult::Aborted;
}

void UBTTask_StartHunting::EndHunting()
{
	// Clear and Invalidate Timer by Handle
	GetWorld()->GetTimerManager().ClearTimer(HuntingTimerHandle);

	if (IsValid(AICRef))
	{
		// If the mantis re-escalated to ALERTED during the hunting window, don't reset perception state —
		// the BT will re-evaluate and enter the ALERTED branch on its own.
		if (!AICRef->CheckCurrentStatusTag(E_AITag::ALERTED))
		{
			if (UPerceptionsMeterComponent* PerceptionsMeter = AICRef->GetPerceptionsMeterComponent())
				PerceptionsMeter->ResetAlert(true);

			AICRef->ClearTargetKey();

			if (UBlackboardComponent* BB = AICRef->GetBlackboardComponent())
				BB->ClearValue(BlackboardKeyNames::DisturbanceLocation);
		}
	}

	if (CachedOwnerComp)
		FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
}
#pragma endregion
