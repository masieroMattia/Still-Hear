#include "Character/StillHearCompanionAIController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Character/StillHearCompanionCharacter.h"
#include "GameFramework/Character.h"
#include "GameplayAbilitySystem/Tags/GameplayTags.h"
#include "Navigation/PathFollowingComponent.h"


AStillHearCompanionAIController::AStillHearCompanionAIController()
{
	PrimaryActorTick.bCanEverTick = true;
	TargetActor = nullptr;
}

void AStillHearCompanionAIController::BeginPlay()
{
	Super::BeginPlay();
}

void AStillHearCompanionAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);

	if (Result.IsSuccess())
	{
		SendEventToTargetActor();
	}
	else
	{
		// I don't think we are ever supposed to enter here
		UE_LOG(LogTemp, Warning, TEXT("Movement Failed: Reason%s"), *UEnum::GetValueAsString(Result.Code));
	}
}

void AStillHearCompanionAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!TargetActor)
		return;

	const FVector TargetLocation = TargetActor->GetActorLocation();
	const FVector Direction = (TargetLocation - GetCharacter()->GetActorLocation()).GetSafeNormal();

	GetCharacter()->AddMovementInput(Direction, 1.0f);

	const float DistanceToTarget = (TargetLocation - GetCharacter()->GetActorLocation()).Length();
	if (DistanceToTarget <= 50.0f)
	{
		SendEventToTargetActor();
		TargetActor = nullptr;
		//GetCharacter()->SetActorEnableCollision(true);
	}
}

void AStillHearCompanionAIController::HandleCompanionMovementInput(const FVector& Right, const FVector& Forward, const FVector2D InputValue) const
{
	GetCharacter()->AddMovementInput(Right, InputValue.X);
	GetCharacter()->AddMovementInput(Forward, InputValue.Y);
}

void AStillHearCompanionAIController::HandleStartCompanionSoundWaveInput()
{
	if (!GetCharacter())
		return;
	
	CastChecked<AStillHearCompanionCharacter>(GetCharacter())->StartSoundWave();
}

void AStillHearCompanionAIController::HandleShootCompanionSoundWaveInput()
{
	if (!GetCharacter())
		return;
	
	CastChecked<AStillHearCompanionCharacter>(GetCharacter())->ShootSoundWave();
}

void AStillHearCompanionAIController::HandleInterruptCompanionSoundWaveInput()
{
	if (!GetCharacter())
		return;
	
	CastChecked<AStillHearCompanionCharacter>(GetCharacter())->InterruptSoundWave();
}

void AStillHearCompanionAIController::HandleCompanionSwitchTargetInput(const FVector2D InputDir2D) const
{
	if (!GetCharacter())
		return;
	
	FGameplayEventData EventData;
	EventData.EventTag = TAG_Event_Companion_SwitchSoundWaveTarget;
	
	FGameplayAbilityTargetData_LocationInfo* LocationData = new FGameplayAbilityTargetData_LocationInfo();
	LocationData->TargetLocation.LiteralTransform.SetLocation(FVector(InputDir2D.X, InputDir2D.Y, 0.f));
	EventData.TargetData.Add(LocationData);
	
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetPawn(), EventData.EventTag, EventData);
}

void AStillHearCompanionAIController::MoveToTargetActor(AActor* TargetActorToMoveTowards)
{
	if (!TargetActorToMoveTowards)
		return;

	GetCharacter()->SetActorEnableCollision(false);
	TargetActor = TargetActorToMoveTowards;
}

void AStillHearCompanionAIController::SendEventToTargetActor() const
{
	FGameplayEventData EventData;
	EventData.EventTag = EventTagToSendWhenRecalled;
	EventData.Instigator = Owner;
	EventData.Target = Owner;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		TargetActor,
		EventData.EventTag,
		EventData
	);
}

