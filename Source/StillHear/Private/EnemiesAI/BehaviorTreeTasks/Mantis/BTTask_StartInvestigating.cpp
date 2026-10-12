#include "EnemiesAI/BehaviorTreeTasks/Mantis/BTTask_StartInvestigating.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "EnemiesAI/Utility/BlackboardKeyNames.h"
#include "EnemiesAI/Controllers/Mantis/AIMantisController.h"
#include "EnemiesAI/Utility/DataAssets/AIMantisInfo_DataAsset.h"
#include "EnemiesAI/Utility/Components/PerceptionsMeterComponent.h"

#pragma region CONSTRUCTOR
UBTTask_StartInvestigating::UBTTask_StartInvestigating()
{
	NodeName = TEXT("Start Investigating");
	bCreateNodeInstance = true;
}
#pragma endregion

#pragma region METHODS
EBTNodeResult::Type UBTTask_StartInvestigating::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Cast Owner Controller to AIMantisController
	AICRef = Cast<AAIMantisController>(OwnerComp.GetAIOwner());
	if (!IsValid(AICRef))
		return EBTNodeResult::Failed;

	CachedOwnerComp = &OwnerComp;

	// Get NPC Ref → Get Data Asset → InvestigateTimer
	const AStillHearAICharacterBase* NPCRef = AICRef->GetNPCRef();
	if (!IsValid(NPCRef))
		return EBTNodeResult::Failed;

	const UAIMantisInfo_DataAsset* DataAsset = Cast<UAIMantisInfo_DataAsset>(NPCRef->GetAIInfo_DataAsset());
	if (!IsValid(DataAsset))
		return EBTNodeResult::Failed;

	// Set Timer by Event → EndInvestigation callback
	GetWorld()->GetTimerManager().SetTimer(
		InvestigationTimerHandle,
		this,
		&UBTTask_StartInvestigating::EndInvestigation,
		DataAsset->InvestigateTimer,
		false
	);

	return EBTNodeResult::InProgress;
}

EBTNodeResult::Type UBTTask_StartInvestigating::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Clear and Invalidate Timer by Handle
	GetWorld()->GetTimerManager().ClearTimer(InvestigationTimerHandle);

	return EBTNodeResult::Aborted;
}

void UBTTask_StartInvestigating::EndInvestigation()
{
	// Clear and Invalidate Timer by Handle
	GetWorld()->GetTimerManager().ClearTimer(InvestigationTimerHandle);

	if (IsValid(AICRef))
	{
		// Get Perceptions Meter Component → Reset Alert
		if (UPerceptionsMeterComponent* PerceptionsMeter = AICRef->GetPerceptionsMeterComponent())
			PerceptionsMeter->ResetAlert(true);

		// Clear Target Key
		AICRef->ClearTargetKey();

		// Clear Disturbance Location Key from Blackboard
		if (UBlackboardComponent* BB = AICRef->GetBlackboardComponent())
			BB->ClearValue(BlackboardKeyNames::DisturbanceLocation);
	}

	// Finish the latent task with success
	if (CachedOwnerComp)
		FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
}
#pragma endregion