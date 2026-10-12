#include "EnemiesAI/Controllers/Mantis/AIMantisController.h"

#include "EngineUtils.h"
#include "BrainComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Touch.h"
#include "EnemiesAI/Utility/AIEnum.h"
#include "Perception/AISense_Hearing.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Touch.h"
#include "Perception/AIPerceptionComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "EnemiesAI/Utility/BlackboardKeyNames.h"
#include "GameplayAbilitySystem/Tags/GameplayTags.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnemiesAI/Utility/Components/PerceptionsMeterComponent.h"
#include "EnemiesAI/Utility/Components/PerceptionVisualizerComponent.h"

#pragma region CONSTRUCTOR
AAIMantisController::AAIMantisController()
{
	PerceptionsMeterComp = CreateDefaultSubobject<UPerceptionsMeterComponent>("Perception Meter Component");
}

E_AISightCone AAIMantisController::GetTypeOfSightCone(const AActor* UpdatedActor) const
{
	if (IsValid(UpdatedActor))
	{
		const FVector AILocation = GetNPCRef()->GetActorLocation();
		const FVector AIEyeLocation = AILocation + FVector(0.f, 0.f, GetDataAsset()->SightHeight);
		const FVector TargetLocation = UpdatedActor->GetActorLocation();

		// Check vertical height difference limit (relative to the AI's eye position)
		const float VerticalDistance = FMath::Abs(TargetLocation.Z - AIEyeLocation.Z);
		if (VerticalDistance > GetDataAsset()->MaxSightHeightDifference)
		{
			return E_AISightCone::NOTSEEN;
		}

		const float CurrentDistance = FVector::Dist(AILocation, TargetLocation);

		// Flatten vectors to ignore the Z axis for horizontal angle calculation
		FVector Forward2D = GetNPCRef()->GetActorForwardVector();
		Forward2D.Z = 0.0f;
		Forward2D.Normalize();

		FVector DirectionToTarget2D = TargetLocation - AILocation;
		DirectionToTarget2D.Z = 0.0f;
		DirectionToTarget2D.Normalize();

		// Calculate horizontal angle
		const float DotProduct = FVector::DotProduct(Forward2D, DirectionToTarget2D);
		const float AngleTowardsTarget = UKismetMathLibrary::DegAcos(DotProduct);

		if (DotProduct >= 0)
		{
			if (AngleTowardsTarget <= GetDataAsset()->SightPeripheralHalfAngleDegree_Narrow)
				if (CurrentDistance <= GetDataAsset()->SightRadius_Narrow)
					return E_AISightCone::NARROW;

			if (AngleTowardsTarget <= GetDataAsset()->SightPeripheralHalfAngleDegree_Wide)
				if (CurrentDistance <= GetDataAsset()->SightRadius_Wide)
					return E_AISightCone::WIDE;
			
			if (AngleTowardsTarget <= GetDataAsset()->SightPeripheralHalfAngleDegree_Peripheral)
				if (CurrentDistance <= GetDataAsset()->SightRadius_Peripheral)
					return E_AISightCone::PERIPHERAL;
			
		}
		else
		{
			if (CurrentDistance <= GetDataAsset()->SightRadius_Backward)
				return E_AISightCone::BACKWARD;
		}
	}

	return E_AISightCone::NOTSEEN;
}
#pragma endregion 
	
#pragma region METHODS
void AAIMantisController::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AAIMantisController::UpdateTarget(AActor* UpdatedActor)
{
	if (!IsValid(UpdatedActor) || !GetBlackboardComponent()) return;

	if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(UpdatedActor))
	{
		if (TargetASC->HasMatchingGameplayTag(TAG_Status_Death))
		{
			ClearTargetKey();
			ClearTargetLocationKey();
			return;
		}
	}

	GetBlackboardComponent()->SetValueAsObject(BlackboardKeyNames::KeyNameTargetActor, UpdatedActor);
}

void AAIMantisController::ForceHuntTarget(AActor* TargetActor)
{
	if (!IsValid(TargetActor) || IsStunned())
		return;

	UpdateTarget(TargetActor);

	// Keep DisturbanceLocation in sync too, in case status later decays to Hunting (lost-target search)
	ClearDisturbanceCooldown();
	UpdateDisturbanceLocation(TargetActor);

	if (PerceptionsMeterComp)
	{
		// Fill both meters so the state doesn't immediately decay back down.
		// ForceAlertToMax() sets status to ALERTED, which runs BT_Mantis_Chase
		// (live MoveTo: TargetActor) - this is the actual "chase" behavior we want here.
		PerceptionsMeterComp->ForceAwarenessToMax();
		PerceptionsMeterComp->ForceAlertToMax();
	}

	// ResetAIState (called just before this) stops continuous sight tracking; restart it on the
	// new target so the alert meter keeps getting refreshed instead of decaying back to Suspicious
	if (HasLoseSight)
		ClearLoseSight();
	StartContinuousSightUpdate();
}

void AAIMantisController::UpdateDisturbanceLocation(const AActor* DisturbanceActor)
{
	if (!IsValid(DisturbanceActor) || !GetBlackboardComponent()) 
		return;

	// Only update if the cooldown is not active to prevent spamming the behavior tree
	if (!IsDisturbanceCooldownActive)
	{
		GetBlackboardComponent()->SetValueAsVector(BlackboardKeyNames::DisturbanceLocation, DisturbanceActor->GetActorLocation());
		IsDisturbanceCooldownActive = true;
        
		GetWorld()->GetTimerManager().SetTimer(DisturbanceCooldownTimerHandle, this, &AAIMantisController::ClearDisturbanceCooldown, GetDataAsset()->DisturbanceCooldownTimer, false);
	}
}

void AAIMantisController::ClearDisturbanceCooldown()
{
	GetWorld()->GetTimerManager().ClearTimer(DisturbanceCooldownTimerHandle);
	IsDisturbanceCooldownActive = false;
}

void AAIMantisController::ClearLoseSight()
{
	GetWorld()->GetTimerManager().ClearTimer(LoseSightTimerHandle);
	HasLoseSight = false;
}

void AAIMantisController::ClearTargetKey()
{
	if (GetBlackboardComponent()) 
		GetBlackboardComponent()->ClearValue(BlackboardKeyNames::KeyNameTargetActor);
}

void AAIMantisController::ClearTargetLocationKey()
{
	if (GetBlackboardComponent()) 
		GetBlackboardComponent()->ClearValue(BlackboardKeyNames::KeyNameTargetLocation);
}

void AAIMantisController::BeginPlay()
{
	Super::BeginPlay();

	// Bind to the status tag changed delegate to trigger group awareness propagation
	OnStatusTagChanged.AddUniqueDynamic(this, &AAIMantisController::OnStatusTagChangedCallback);

	if (GetMantisRef() && GetMantisRef()->IsDormant())
	{
		if (UBrainComponent* BrainComp = GetBrainComponent())
			BrainComp->StopLogic(TEXT("Dormant"));
	}
}

void AAIMantisController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	// Initialize the perception meters when the controller takes possession of the Pawn
	if (PerceptionsMeterComp) 
		PerceptionsMeterComp->SetupPerceptionsMeterValues();

	SetupTouchInfo();

	// If the Mantis starts dormant, disable perception and movement now
	if (GetMantisRef() && GetMantisRef()->IsDormant())
	{
		if (AIPerceptionComp)
			AIPerceptionComp->Deactivate();

		if (UCharacterMovementComponent* MoveComp = GetMantisRef()->GetCharacterMovement())
			MoveComp->DisableMovement();

		if (UPerceptionVisualizerComponent* Visualizer = GetMantisRef()->FindComponentByClass<UPerceptionVisualizerComponent>())
			Visualizer->SetComponentTickEnabled(false);

		GetMantisRef()->SetActorTickEnabled(false);
	}
}

void AAIMantisController::SetupSightInfo()
{
	Super::SetupSightInfo();
	
	if (!AIPerceptionComp) 
		return;

	// Get the sight config
	const FAISenseID SightID = UAISense::GetSenseID<UAISense_Sight>();
	UAISenseConfig_Sight* SightConfig = Cast<UAISenseConfig_Sight>(AIPerceptionComp->GetSenseConfig(SightID));

	if (SightConfig && GetDataAsset())
	{
		// Inject Data Asset values into the perception system
		SightConfig->SightRadius = GetDataAsset()->SightRadius;
		SightConfig->LoseSightRadius = GetDataAsset()->LoseSightRadius;
		SightConfig->PeripheralVisionAngleDegrees = GetDataAsset()->PeripheralVisionAngleDegrees;
        
		AIPerceptionComp->RequestStimuliListenerUpdate();
	}
}

void AAIMantisController::SetupHearingInfo()
{
	Super::SetupHearingInfo();
	
	if (!AIPerceptionComp) 
		return;
	
	// Get the hearing config
	const FAISenseID HearingID = UAISense::GetSenseID<UAISense_Hearing>();
	UAISenseConfig_Hearing* HearingConfig = Cast<UAISenseConfig_Hearing>(AIPerceptionComp->GetSenseConfig(HearingID));
	
	if (HearingConfig && GetDataAsset())
	{
		HearingConfig->HearingRange = GetDataAsset()->RunHearingRange;
		
		AIPerceptionComp->RequestStimuliListenerUpdate();
	}
}

void AAIMantisController::SetupTouchInfo() const
{
	if (!AIPerceptionComp) 
		return;

	// Get the touch config
	const FAISenseID TouchID = UAISense::GetSenseID<UAISense_Touch>();
	UAISenseConfig_Touch* TouchConfig = Cast<UAISenseConfig_Touch>(AIPerceptionComp->GetSenseConfig(TouchID));

	if (!TouchConfig)
	{
		// If the touch config doesn't exist, create it manually
		TouchConfig = NewObject<UAISenseConfig_Touch>(AIPerceptionComp, TEXT("Touch Config"));
		AIPerceptionComp->ConfigureSense(*TouchConfig);
	}

	if (TouchConfig)
	{
		TouchConfig->DetectionByAffiliation.bDetectEnemies = true;
		TouchConfig->DetectionByAffiliation.bDetectFriendlies = true;
		TouchConfig->DetectionByAffiliation.bDetectNeutrals = true;

		AIPerceptionComp->RequestStimuliListenerUpdate();
	}
}

bool AAIMantisController::IsStunned() const
{
	if (!GetNPCRef() || !GetNPCRef()->GetAbilitySystemComponent())
		return false;

	return GetNPCRef()->GetAbilitySystemComponent()->HasMatchingGameplayTag(TAG_Status_EnemyAI_Stunned);
}

void AAIMantisController::SetupBlackboardKeys()
{
	Super::SetupBlackboardKeys();
	
	if (!IsValid(GetBlackboardComponent()) || !IsValid(GetNPCRef()))
		return;
	
	if (IsValid(GetDataAsset()))
	{
		GetBlackboardComponent()->SetValueAsFloat(BlackboardKeyNames::KeyNameEQSRadius, GetDataAsset()->EQSRadius);
		GetBlackboardComponent()->SetValueAsFloat(BlackboardKeyNames::KeyNameEQSDistanceFromEach, GetDataAsset()->EQSDistanceFromEach);
		GetBlackboardComponent()->SetValueAsFloat(BlackboardKeyNames::KeyNameStartHuntingWaitTime, GetDataAsset()->StartHuntingWaitTime);
		GetBlackboardComponent()->SetValueAsFloat(BlackboardKeyNames::KeyNameStartInvestigatingWaitTime, GetDataAsset()->StartInvestigatingWaitTime);
		GetBlackboardComponent()->SetValueAsEnum(BlackboardKeyNames::KeyNameAttackType, static_cast<uint8>(E_MantisAttackType::NONE));
	}
}
#pragma endregion

#pragma region UFUNCTIONS
void AAIMantisController::PerceptionEventReceived(AActor* UpdatedActor, const FAIStimulus Stimulus)
{
	Super::PerceptionEventReceived(UpdatedActor, Stimulus);
	
	if (!IsValid(UpdatedActor))
		return;

	// Skip all perception processing while dormant
	if (GetMantisRef() && GetMantisRef()->IsDormant())
		return;

    // Handle Sight
	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			// Check height limit (relative to the AI's eye position)
			const FVector AIEyeLocation = GetNPCRef()->GetActorLocation() + FVector(0.f, 0.f, GetDataAsset()->SightHeight);
			const FVector TargetLocation = UpdatedActor->GetActorLocation();
			const float VerticalDistance = FMath::Abs(TargetLocation.Z - AIEyeLocation.Z);

			if (VerticalDistance > GetDataAsset()->MaxSightHeightDifference)
			{
				StopContinuousSightUpdate();
				if (!HasLoseSight)
				{
					HasLoseSight = true;
					GetWorld()->GetTimerManager().SetTimer(LoseSightTimerHandle, this, &AAIMantisController::OnLoseSightTimerFinished, GetDataAsset()->LoseSightTimer, false);
				}
				return;
			}

			UpdateTarget(UpdatedActor);
			StartContinuousSightUpdate();
            
			if (HasLoseSight) 
				ClearLoseSight();
		}
		else
		{
			StopContinuousSightUpdate();
            
			if (!HasLoseSight)
			{
				HasLoseSight = true;
				GetWorld()->GetTimerManager().SetTimer(LoseSightTimerHandle, this, &AAIMantisController::OnLoseSightTimerFinished, GetDataAsset()->LoseSightTimer, false);
			}
		}
	}
    // Handle Hearing
    else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
    {
        if (Stimulus.WasSuccessfullySensed())
        {
            UpdateDisturbanceLocation(UpdatedActor);

            if (PerceptionsMeterComp && !IsStunned())
            {
            	// Determine the hearing type from the Stimulus tag (Walk, Run, Crouch, Repeater)
            	E_AIHearingType HearingType = E_AIHearingType::WALK;
            	const FString TagString = Stimulus.Tag.ToString();
            	
            	if (TagString.Contains(TEXT("Run")))
            		HearingType = E_AIHearingType::RUN;
            	else if (TagString.Contains(TEXT("Crouch")))
            		HearingType = E_AIHearingType::CROUCH;
            	else if (TagString.Contains(TEXT("Repeater")))
            		HearingType = E_AIHearingType::REPEATER;

            	PerceptionsMeterComp->ProcessSensoryInput(E_AISense::HEARING, E_AISightCone::NONE, HearingType);
            }
        }
    }
    // Handle Touch
    else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Touch>())
    {
        if (Stimulus.WasSuccessfullySensed())
        {
            UpdateTarget(UpdatedActor);
            UpdateDisturbanceLocation(UpdatedActor);
        	
            if (GetBlackboardComponent()) 
            	GetBlackboardComponent()->SetValueAsBool(BlackboardKeyNames::KeyNameHasTouched, true);
            
        	if (PerceptionsMeterComp && !IsStunned()) 
        		PerceptionsMeterComp->ProcessSensoryInput(E_AISense::TOUCH);
        }
        else
        {
            if (GetBlackboardComponent()) 
            	GetBlackboardComponent()->SetValueAsBool(BlackboardKeyNames::KeyNameHasTouched, false);
        }
    }
}

void AAIMantisController::StartContinuousSightUpdate()
{
	if (!GetWorld()->GetTimerManager().IsTimerActive(ContinuousSightTimerHandle))
		GetWorld()->GetTimerManager().SetTimer(ContinuousSightTimerHandle, this, &AAIMantisController::UpdateSightLogic, 0.1f, true);
}

void AAIMantisController::StopContinuousSightUpdate()
{
	GetWorld()->GetTimerManager().ClearTimer(ContinuousSightTimerHandle);
}

void AAIMantisController::UpdateSightLogic()
{
	if (!GetBlackboardComponent())
		return;

	if (IsStunned())
		return;
	
	AActor* TargetActor = Cast<AActor>(GetBlackboardComponent()->GetValueAsObject(BlackboardKeyNames::KeyNameTargetActor));
	if (IsValid(TargetActor))
	{
		if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor))
		{
			if (TargetASC->HasMatchingGameplayTag(TAG_Status_Death))
			{
				ClearTargetKey();
				ClearTargetLocationKey();
				StopContinuousSightUpdate();
				return;
			}
		}

		const E_AISightCone CurrentCone = GetTypeOfSightCone(TargetActor);

		// If the target is in a valid cone, continuously fill the meter
		if (CurrentCone != E_AISightCone::NOTSEEN && PerceptionsMeterComp)
		{
			UpdateDisturbanceLocation(TargetActor);
			PerceptionsMeterComp->ProcessContinuousSight(CurrentCone, 0.1f); // 0.1f matches the timer interval
		}
		else
		{
			// Target is not seen (too high or out of cone). If too low, trigger lose sight
			const FVector AIEyeLocation = GetNPCRef()->GetActorLocation() + FVector(0.f, 0.f, GetDataAsset()->SightHeight);
			const FVector TargetLocation = TargetActor->GetActorLocation();
			const float VerticalDistance = FMath::Abs(TargetLocation.Z - AIEyeLocation.Z);
			if (VerticalDistance > GetDataAsset()->MaxSightHeightDifference)
			{
				StopContinuousSightUpdate();
				if (!HasLoseSight)
				{
					HasLoseSight = true;
					GetWorld()->GetTimerManager().SetTimer(LoseSightTimerHandle, this, &AAIMantisController::OnLoseSightTimerFinished, GetDataAsset()->LoseSightTimer, false);
				}
			}
		}
	}
}

void AAIMantisController::OnLoseSightTimerFinished()
{
	if (HasLoseSight)
	{
		if (CheckCurrentStatusTag(E_AITag::ALERTED))
			UpdateCurrentStatusTag(E_AITag::HUNTING);
		
		// Save the last known position before forgetting the target completely
		if (GetBlackboardComponent())
		{
			AActor* TargetActor = Cast<AActor>(GetBlackboardComponent()->GetValueAsObject(BlackboardKeyNames::KeyNameTargetActor));
			
			// Check if actor is still alive and valid before getting its location
			if (IsValid(TargetActor))
				UpdateDisturbanceLocation(TargetActor);
		}
        
		ClearTargetKey();
	}

	ClearLoseSight();
}

void AAIMantisController::OnStatusTagChangedCallback(const FGameplayTag NewStatusTag)
{
	// STUN
	if (NewStatusTag.MatchesTagExact(TAG_Status_EnemyAI_Stunned))
	{
		ClearTargetLocationKey();

		if (GetBlackboardComponent())
			GetBlackboardComponent()->SetValueAsBool(BlackboardKeyNames::KeyNameHasTouched, false);
		
		if (PerceptionsMeterComp)
			PerceptionsMeterComp->ResetAll();
		
		return;
	}
	
	// Don't propagate if this change was itself caused by a propagation
	if (bIsPropagating)
		return;
	
	const UAIMantisInfo_DataAsset* DataAsset = GetDataAsset();
	if (!IsValid(DataAsset) || !IsValid(GetBlackboardComponent()))
		return;

	// SUSPICIOUS propagation
	if (NewStatusTag.MatchesTagExact(TAG_Status_EnemyAI_Suspicious) && DataAsset->bEnableSuspiciousPropagation)
	{
		const FVector DisturbanceLoc = GetBlackboardComponent()->GetValueAsVector(BlackboardKeyNames::DisturbanceLocation);
		PropagateGroupAwareness(E_AITag::SUSPICIOUS, nullptr, DisturbanceLoc);
	}
	// ALERTED propagation
	else if (NewStatusTag.MatchesTagExact(TAG_Status_EnemyAI_Alerted) && DataAsset->bEnableAlertedPropagation)
	{
		AActor* Target = Cast<AActor>(GetBlackboardComponent()->GetValueAsObject(BlackboardKeyNames::KeyNameTargetActor));
		const FVector DisturbanceLoc = GetBlackboardComponent()->GetValueAsVector(BlackboardKeyNames::DisturbanceLocation);
		PropagateGroupAwareness(E_AITag::ALERTED, Target, DisturbanceLoc);
	}
}

void AAIMantisController::PropagateGroupAwareness(const E_AITag NewTag, AActor* TargetActor, const FVector& DisturbanceLoc) const
{
	if (!IsValid(GetNPCRef()) || !IsValid(GetDataAsset()))
		return;

	const float RadiusSq = FMath::Square(GetDataAsset()->GroupAwarenessRadius);
	const FVector MyLocation = GetNPCRef()->GetActorLocation();

	for (TActorIterator<AAIMantisCharacter> Iterator(GetWorld()); Iterator; ++Iterator)
	{
		const AAIMantisCharacter* OtherMantis = *Iterator;
		
		// Skip self
		if (!IsValid(OtherMantis) || OtherMantis == GetNPCRef())
			continue;

		// Check distance
		if (FVector::DistSquared(MyLocation, OtherMantis->GetActorLocation()) > RadiusSq)
			continue;

		// Get the other Mantis's controller
		AAIMantisController* OtherController = Cast<AAIMantisController>(OtherMantis->GetController());
		if (!IsValid(OtherController))
			continue;

		OtherController->ReceiveGroupAlert(NewTag, TargetActor, DisturbanceLoc);
	}
}

void AAIMantisController::ReceiveGroupAlert(const E_AITag IncomingTag, AActor* TargetActor, const FVector& DisturbanceLoc)
{
	// Prevent re-propagation
	bIsPropagating = true;

	if (!IsValid(GetBlackboardComponent()) || !IsValid(PerceptionsMeterComp))
	{
		bIsPropagating = false;
		return;
	}

	// A stunned mantis must not be forced into ALERTED/SUSPICIOUS or have its target re-acquired by a sibling's alert
	if (IsStunned())
	{
		bIsPropagating = false;
		return;
	}

	// Don't downgrade: if already at the same or a higher state, skip
	if (IncomingTag == E_AITag::SUSPICIOUS && 
		(CheckCurrentStatusTag(E_AITag::SUSPICIOUS) || CheckCurrentStatusTag(E_AITag::ALERTED) || CheckCurrentStatusTag(E_AITag::HUNTING)))
	{
		bIsPropagating = false;
		return;
	}

	if (IncomingTag == E_AITag::ALERTED && 
		(CheckCurrentStatusTag(E_AITag::ALERTED) || CheckCurrentStatusTag(E_AITag::HUNTING)))
	{
		bIsPropagating = false;
		return;
	}

	// Set the disturbance location so the BT knows where to go
	GetBlackboardComponent()->SetValueAsVector(BlackboardKeyNames::DisturbanceLocation, DisturbanceLoc);

	if (IncomingTag == E_AITag::SUSPICIOUS)
	{
		// Force awareness to max → SUSPICIOUS
		PerceptionsMeterComp->StopAwareness();
		PerceptionsMeterComp->ForceAwarenessToMax();
	}
	else if (IncomingTag == E_AITag::ALERTED)
	{
		// Force both meters to max → ALERTED
		PerceptionsMeterComp->StopAwareness();
		PerceptionsMeterComp->ForceAwarenessToMax();
		PerceptionsMeterComp->ForceAlertToMax();

		// Give the target actor so the AI can chase directly
		if (IsValid(TargetActor))
			UpdateTarget(TargetActor);
	}

	bIsPropagating = false;
}

void AAIMantisController::ResetAIState()
{
	Super::ResetAIState();

	ClearLoseSight();
	ClearDisturbanceCooldown();
	StopContinuousSightUpdate();

	if (PerceptionsMeterComp)
		PerceptionsMeterComp->ResetAll();
}
#pragma endregion