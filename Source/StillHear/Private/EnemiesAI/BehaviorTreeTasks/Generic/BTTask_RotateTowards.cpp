#include "EnemiesAI/BehaviorTreeTasks/Generic/BTTask_RotateTowards.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnemiesAI/Pawns/Base/StillHearAICharacterBase.h"
#include "EnemiesAI/Utility/DataAssets/AIInfo_DataAssetBase.h"
#include "Kismet/KismetMathLibrary.h"

UBTTask_RotateTowards::UBTTask_RotateTowards()
{
    NodeName = "Rotate Towards";
    bNotifyTick = true;
    bCreateNodeInstance = true; // Required to safely store TargetLocation per-instance in the class
    bActor = false;
    ErrorTolerance = 1.0f;
}

EBTNodeResult::Type UBTTask_RotateTowards::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    const AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController) 
        return EBTNodeResult::Failed;

    const APawn* ControlledPawn = AIController->GetPawn();
    if (!ControlledPawn) 
        return EBTNodeResult::Failed;

    const UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp) 
        return EBTNodeResult::Failed;

    // Retrieve target location based on the boolean flag
    if (bActor)
    {
        const AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetLocationKey.SelectedKeyName));
        if (!TargetActor) 
            return EBTNodeResult::Failed;
        
        TargetLocation = TargetActor->GetActorLocation();
    }
    else
        TargetLocation = BlackboardComp->GetValueAsVector(TargetLocationKey.SelectedKeyName);

    return EBTNodeResult::InProgress;
}

void UBTTask_RotateTowards::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, const float DeltaSeconds)
{
    const AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController) 
        return FinishLatentTask(OwnerComp, EBTNodeResult::Failed);

    APawn* ControlledPawn = AIController->GetPawn();
    if (!ControlledPawn) 
        return FinishLatentTask(OwnerComp, EBTNodeResult::Failed);

    AStillHearAICharacterBase* AICharacter = Cast<AStillHearAICharacterBase>(ControlledPawn);
    if (!AICharacter)     
        return FinishLatentTask(OwnerComp, EBTNodeResult::Failed);

    const float RotationSpeed = AICharacter->GetAIInfo_DataAsset()->RotationSpeed;

    const FRotator CurrentRotation = AICharacter->GetActorRotation();
    const FRotator FinalRotation = UKismetMathLibrary::FindLookAtRotation(AICharacter->GetActorLocation(), TargetLocation);

    // Interpolate rotation
    const FRotator NewRotation = FMath::RInterpTo(CurrentRotation, FinalRotation, DeltaSeconds, RotationSpeed);
    AICharacter->SetActorRotation(NewRotation);

    // Calculate the shortest angle difference between the new yaw and target yaw
    const float YawDifference = FMath::Abs(FMath::FindDeltaAngleDegrees(NewRotation.Yaw, FinalRotation.Yaw));

    // Check if the rotation is within the error tolerance
    if (YawDifference <= ErrorTolerance) 
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
}