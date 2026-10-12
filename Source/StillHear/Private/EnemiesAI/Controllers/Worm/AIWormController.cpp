// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemiesAI/Controllers/Worm/AIWormController.h"

#include "BrainComponent.h"
#include "EnemiesAI/Utility/DataAssets/AIInfo_DataAssetBase.h"
#include "EnemiesAI/Utility/DataAssets/AIWormInfo_DataAsset.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Touch.h"
#include "DrawDebugHelpers.h"
#include "Animation/AnimInstances/EnemyAI/WormAnimInstance.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnemiesAI/Utility/BlackboardKeyNames.h"
#include "EnemiesAI/Utility/AIEnum.h"
#include "GameplayAbilitySystem/Tags/GameplayTags.h"


class UAISense_Touch;
// Sets default values
AAIWormController::AAIWormController()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void AAIWormController::OnStatusTagChangedCallback(FGameplayTag NewStatusTag)
{
	if (NewStatusTag.MatchesTagExact(TAG_Status_EnemyAI_Stunned))
	{
		GetWorld()->GetTimerManager().ClearTimer(AlertCooldownTimerHandle);
	}
}

void AAIWormController::PerceptionEventReceived(AActor* UpdatedActor, FAIStimulus Stimulus)
{
	/*
	 * The Brain Component is something that is automatically created by Unreal when a Behaviour Tree
	 * starts running. We do this in the Begin Play of the parent class, so we expect it to be running here
	 */
	if (!BrainComponent || !BrainComponent->IsRunning() || !IsValid(UpdatedActor) || (GetCurrentStatusTag() == TAG_Status_EnemyAI_Stunned))
		return;
	
	E_AISense CurrentSenseUsed = E_AISense::NONE;

	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
	{
		CurrentSenseUsed = E_AISense::HEARING;
		
		if (Stimulus.WasSuccessfullySensed())
		{
			
			if (!Stimulus.Tag.ToString().Contains("Vibration"))
			{
				UpdateTargetActor(nullptr);
				return;
			}

			/*
			 * Checking the Walking range, the Running range and the Crouching range to understand if we really detected the sound
			 */
			float StimulusDistance = (GetNPCRef()->GetActorLocation() - Stimulus.StimulusLocation).Size();
			
			if ((Stimulus.Tag.ToString().Contains("Walk") && StimulusDistance <= NPCWormDataAsset->WalkHearingRange)
				|| (Stimulus.Tag.ToString().Contains("Run") && StimulusDistance <= NPCWormDataAsset->RunHearingRange)
				|| (Stimulus.Tag.ToString().Contains("Crouch") && StimulusDistance <= NPCWormDataAsset->CrouchHearingRange))
			{
				// We successfully sensed the character. We need to update some Blackboard Variable
				UpdateCurrentStatusTag(E_AITag::ALERTED);
				WormAnimInstance->SetIsAlerted(true);
				UpdateTargetLocation(Stimulus.StimulusLocation);
				UpdateTargetActor(UpdatedActor);

				// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Worm Noticed!");
			}
			else if (Stimulus.Tag.ToString().Contains("Bell"))
			{
				UpdateCurrentStatusTag(E_AITag::ALERTED);
				WormAnimInstance->SetIsAlerted(true);
				UpdateTargetLocation(Stimulus.StimulusLocation, true);
			}
				
		}
	}
	else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Touch>())
	{

		CurrentSenseUsed = E_AISense::TOUCH;

		if (Stimulus.WasSuccessfullySensed())
		{
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, "Worm Touched!");
			Blackboard->SetValueAsBool(BlackboardKeyNames::KeyNameHasTouched, true);
			Blackboard->SetValueAsVector(BlackboardKeyNames::KeyNameTargetLocation, UpdatedActor->GetActorLocation());
		}
		else
		{
			Blackboard->SetValueAsBool(BlackboardKeyNames::KeyNameHasTouched, false);
		}
	}
}

void AAIWormController::SetIsDiving(const bool bIsDiving)
{
	Blackboard->SetValueAsBool(BlackboardKeyNames::KeyNameIsDiving, bIsDiving);
	bIsInAir = bIsDiving;
}

// Called when the game starts or when spawned
void AAIWormController::BeginPlay()
{
	Super::BeginPlay();

	OnStatusTagChanged.AddUniqueDynamic(this, &ThisClass::OnStatusTagChangedCallback);
}

void AAIWormController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	NPCWormRef = GetWormRef();
	NPCWormDataAsset = GetDataAsset();
	WormAnimInstance = Cast<UWormAnimInstance>(NPCWormRef->GetMesh()->GetAnimInstance());
}

void AAIWormController::SetupHearingInfo()
{
	if (this == nullptr)
		return;


	FAISenseID Id = UAISense::GetSenseID(UAISense_Hearing::StaticClass());
	if (!Id.IsValid())
		return;

	auto Perception = GetAIPerceptionComponent();
	if (Perception == nullptr)
		return;
	
	auto Config = Perception->GetSenseConfig(Id);
	if (Config == nullptr)
		return;
	
	auto ConfigSight = Cast<UAISenseConfig_Hearing>(Config);

	ConfigSight->HearingRange = Cast<UAIWormInfo_DataAsset>(NPCRef->GetAIInfo_DataAsset())->MaxHearingRange;

	Perception->RequestStimuliListenerUpdate();
}

void AAIWormController::UpdateTargetLocation(const FVector& TargetLocation, bool WasBell)
{
	Blackboard->SetValueAsVector(BlackboardKeyNames::KeyNameTargetLocation, TargetLocation);
	CurrentTargetLocation = TargetLocation;

	if (WasBell)
	{
		Blackboard->SetValueAsBool(BlackboardKeyNames::KeyNameWasBellPlayed, true);
		Blackboard->ClearValue(BlackboardKeyNames::KeyNameTargetActor);
		HandleAlertTimer(WasBell);
	}

}

void AAIWormController::UpdateTargetActor(AActor* TargetActor)
{
	if (IsValid(TargetActor))
	{
		Blackboard->SetValueAsObject(BlackboardKeyNames::KeyNameTargetActor, TargetActor);
	}
	else
	{
		Blackboard->ClearValue(BlackboardKeyNames::KeyNameTargetActor);
	}
	
	CurrentTargetActor = TargetActor;

	HandleAlertTimer();
}

void AAIWormController::ClearAlertStatus()
{
	GetWorld()->GetTimerManager().ClearTimer(AlertCooldownTimerHandle);

	if (GetCurrentStatusTag() == TAG_Status_EnemyAI_Stunned)
		return;
	
	UpdateCurrentStatusTag(E_AITag::UNAWARE);
	WormAnimInstance->SetIsAlerted(false);
	Blackboard->ClearValue(BlackboardKeyNames::KeyNameHasRoared);
}

void AAIWormController::HandleAlertTimer(bool WasBell)
{
	if (AlertCooldownTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(AlertCooldownTimerHandle);
	}

	const float TimerDuration = WasBell ? GetDataAsset()->BellCooldownTimer : GetDataAsset()->AlertCooldownTimer;
	GetWorld()->GetTimerManager().SetTimer(AlertCooldownTimerHandle, this, &ThisClass::ClearAlertStatus, TimerDuration, false);
}

void AAIWormController::UpdateAnimInstance()
{
	// Handling Anim Instance statuses
	const FVector Velocity = NPCWormRef->GetVelocity();

	E_AISpeedType CurrentWormSpeed = NPCWormRef->GetSpeedType();
	
	if (!Velocity.IsZero())
	{

		if (!bIsInAir && !NPCWormRef->GetAbilitySystemComponent()->IsGameplayCueActive(TAG_GameplayCue_GroundDebris))
		{
			NPCWormRef->GetAbilitySystemComponent()->AddGameplayCue(TAG_GameplayCue_GroundDebris);
		}
		else if (bIsInAir)
		{
			NPCWormRef->GetAbilitySystemComponent()->RemoveGameplayCue(TAG_GameplayCue_GroundDebris);
		}
		
		if (CurrentWormSpeed == E_AISpeedType::WALK)
		{
			WormAnimInstance->SetIsWalking(true);
			WormAnimInstance->SetIsRunning(false);
		}
		else if (CurrentWormSpeed == E_AISpeedType::RUN)
		{
			WormAnimInstance->SetIsRunning(true);
		}
	}
	else
	{
		WormAnimInstance->SetIsWalking(false);
		WormAnimInstance->SetIsRunning(false);
		NPCWormRef->GetAbilitySystemComponent()->RemoveGameplayCue(TAG_GameplayCue_GroundDebris);
	}
}

// Called every frame
void AAIWormController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateAnimInstance();
	
	// Drawing Debug circles to visualize hearing ranges
#if WITH_EDITOR
	if (NPCWormDataAsset && NPCWormDataAsset->ShowDebugCircles)
	{
		FVector Offset = FVector(0, 0, 10);
		
		DrawDebugCircle(
			GetWorld(),
			NPCWormRef->GetActorLocation() + Offset,
			NPCWormDataAsset->RunHearingRange,
			32,
			NPCWormDataAsset->RunHearingRangeColor,
			false,
			-1.0f,
			0,
			2.0f,
			NPCWormRef->GetActorRightVector(),
			NPCWormRef->GetActorForwardVector()
		);

		DrawDebugCircle(
			GetWorld(),
			NPCWormRef->GetActorLocation() + Offset,
			NPCWormDataAsset->WalkHearingRange,
			32,
			NPCWormDataAsset->WalkHearingRangeColor,
			false,
			-1.0f,
			0,
			2.0f,
			NPCWormRef->GetActorRightVector(),
			NPCWormRef->GetActorForwardVector()
		);

		DrawDebugCircle(
			GetWorld(),
			NPCWormRef->GetActorLocation() + Offset,
			NPCWormDataAsset->CrouchHearingRange,
			32,
			NPCWormDataAsset->CrouchHearingRangeColor,
			false,
			-1.0f,
			0,
			2.0f,
			NPCWormRef->GetActorRightVector(),
			NPCWormRef->GetActorForwardVector()
		);
	}
#endif
	
}

