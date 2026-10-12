#include "EnemiesAI/Utility/Components/PerceptionsMeterComponent.h"

#include "EnemiesAI/Utility/DataAssets/AIPerceptionsMeterInfo_DataAsset.h"

#pragma region CONSTRUCTOR
UPerceptionsMeterComponent::UPerceptionsMeterComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame
	// You can turn these features off to improve performance if you don't need them
	
	PrimaryComponentTick.bCanEverTick = false;
	bAutoActivate = true;
	bWantsInitializeComponent = true;
}
#pragma endregion
	
#pragma region METHODS
void UPerceptionsMeterComponent::InitializeComponent()
{
	AICRef = Cast<AStillHearAIControllerBase>(GetOwner());
	Super::InitializeComponent();
}

void UPerceptionsMeterComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPerceptionsMeterComponent::SetupPerceptionsMeterValues()
{
	if (!IsValid(AICRef) || !IsValid(AICRef->GetNPCRef())) 
		return;

	UAIInfo_DataAssetBase* AIData = AICRef->GetNPCRef()->GetAIInfo_DataAsset();
	const UAIPerceptionsMeterInfo_DataAsset* AISuspiciousData = Cast<UAIPerceptionsMeterInfo_DataAsset>(AIData);
	
	if (!IsValid(AISuspiciousData)) return;

	// Initialize Awareness values
	MaxAwarenessValue = AISuspiciousData->MaxAwarenessValue;
	AwarenessDecreaseValue = AISuspiciousData->AwarenessDecreaseValue;
	AwarenessDecreaseTime = AISuspiciousData->AwarenessDecreaseTime;
	AwarenessPauseTime = AISuspiciousData->AwarenessPauseTime;
	AwarenessIncreaseOnSight_Narrow = AISuspiciousData->AwarenessIncreaseOnSight_Narrow;
	AwarenessIncreaseOnSight_Wide = AISuspiciousData->AwarenessIncreaseOnSight_Wide;
	AwarenessIncreaseOnSight_Peripheral = AISuspiciousData->AwarenessIncreaseOnSight_Peripheral;
	AwarenessIncreaseOnSight_Backward = AISuspiciousData->AwarenessIncreaseOnSight_Backward;

	// Initialize Hearing values
	AwarenessIncreaseOnHearing_Walk = AISuspiciousData->AwarenessIncreaseOnHearing_Walk;
	AwarenessIncreaseOnHearing_Run = AISuspiciousData->AwarenessIncreaseOnHearing_Run;
	AwarenessIncreaseOnHearing_Crouch = AISuspiciousData->AwarenessIncreaseOnHearing_Crouch;
	AwarenessIncreaseOnHearing_Repeater = AISuspiciousData->AwarenessIncreaseOnHearing_Repeater;

	// Initialize Alert values
	MaxAlertValue = AISuspiciousData->MaxAlertValue;
	AlertDecreaseValue = AISuspiciousData->AlertDecreaseValue;
	AlertDecreaseTime = AISuspiciousData->AlertDecreaseTime;
	AlertPauseTime = AISuspiciousData->AlertPauseTime;
	AlertIncreaseOnSight_Narrow = AISuspiciousData->AlertIncreaseOnSight_Narrow;
	AlertIncreaseOnSight_Wide = AISuspiciousData->AlertIncreaseOnSight_Wide;
	AlertIncreaseOnSight_Peripheral = AISuspiciousData->AlertIncreaseOnSight_Peripheral;
	AlertIncreaseOnSight_Backward = AISuspiciousData->AlertIncreaseOnSight_Backward;

	// Set initial states
	CurrentAwarenessValue = 0.0f;
	CurrentAlertValue = 0.0f;
	bCanUpdateAwareness = true;
	bCanUpdateAlert = false;
	bIsDecreasingAwareness = false;
	bIsDecreasingAlert = false;
}

void UPerceptionsMeterComponent::BroadcastPerceptionValues() const
{
	const float AwarenessValue = MaxAwarenessValue > 0.0f ? CurrentAwarenessValue / MaxAwarenessValue : 0.0f;
	const float AlertValue = MaxAlertValue > 0.0f ? CurrentAlertValue / MaxAlertValue : 0.0f;

	OnPerceptionsUpdated.Broadcast(AwarenessValue, AlertValue);
}

void UPerceptionsMeterComponent::ProcessSensoryInput(const E_AISense InputSense, const E_AISightCone CurrentTypeOfCone, const E_AIHearingType HearingType)
{
	if (bCanUpdateAwareness)
	{
		switch (InputSense)
		{
			case E_AISense::SIGHT:
				UpdateAwareness(GetCorrectSightAwareness(CurrentTypeOfCone));
				break;
			case E_AISense::HEARING:
				UpdateAwareness(GetCorrectHearingAwareness(HearingType));
				break;
			case E_AISense::TOUCH:
				UpdateAwareness(MaxAwarenessValue);
				break;
			default: 
				break;
		}
	}
	else if (bCanUpdateAlert)
	{
		switch (InputSense)
		{
		case E_AISense::SIGHT:
			UpdateAlert(GetCorrectSightAlert(CurrentTypeOfCone));
			break;
		case E_AISense::TOUCH:
			UpdateAlert(MaxAlertValue, true);
			break;
		default: 
			break;
		}
	}
}

void UPerceptionsMeterComponent::ProcessContinuousSight(const E_AISightCone CurrentTypeOfCone, const float DeltaTime)
{
	if (bCanUpdateAwareness)
	{
		const float IncreaseValue = GetCorrectSightAwareness(CurrentTypeOfCone) * DeltaTime;
		UpdateAwareness(IncreaseValue);
	}
	else if (bCanUpdateAlert)
	{
		const float IncreaseValue = GetCorrectSightAlert(CurrentTypeOfCone) * DeltaTime;
		UpdateAlert(IncreaseValue, false);
	}
}

void UPerceptionsMeterComponent::UpdateAwareness(const float DeltaValue)
{
	if (!IsValid(AICRef) || !bCanUpdateAwareness) 
		return;

	if (DeltaValue > 0)
	{
		// Stop decreasing if we receive new input
		if (bIsDecreasingAwareness)
		{
			bIsDecreasingAwareness = false;
			GetWorld()->GetTimerManager().ClearTimer(AwarenessDecreaseTimerHandle);
		}

		// Reset pause timer
		if (AwarenessPauseTimerHandle.IsValid())
			GetWorld()->GetTimerManager().ClearTimer(AwarenessPauseTimerHandle);

		GetWorld()->GetTimerManager().SetTimer(AwarenessPauseTimerHandle, this, &UPerceptionsMeterComponent::OnAwarenessPauseFinished, AwarenessPauseTime, false);
	}

	CurrentAwarenessValue = FMath::Clamp(CurrentAwarenessValue + DeltaValue, 0.0f, MaxAwarenessValue);

	BroadcastPerceptionValues();
	
	// Check if awareness meter is full
	if (CurrentAwarenessValue >= MaxAwarenessValue)
	{
		CurrentAwarenessValue = MaxAwarenessValue;
		StopAwareness();
        
		// Transition to Alert phase
		bCanUpdateAlert = true;
		AICRef->UpdateCurrentStatusTag(E_AITag::SUSPICIOUS);

		// Start the Alert pause timer so that if no alert stimuli arrive, the system will eventually fall back to the Awareness phase and decay
		GetWorld()->GetTimerManager().SetTimer(AlertPauseTimerHandle, this, &UPerceptionsMeterComponent::OnAlertPauseFinished, AlertPauseTime, false);
	}
}

void UPerceptionsMeterComponent::OnAwarenessPauseFinished()
{
	GetWorld()->GetTimerManager().ClearTimer(AwarenessPauseTimerHandle);

	if (!bIsDecreasingAwareness)
	{
		bIsDecreasingAwareness = true;
		OnDecreaseAwareness();
		GetWorld()->GetTimerManager().SetTimer(AwarenessDecreaseTimerHandle, this, &UPerceptionsMeterComponent::OnDecreaseAwareness, AwarenessDecreaseTime, true);
	}
}

void UPerceptionsMeterComponent::OnDecreaseAwareness()
{
	if (!bIsDecreasingAwareness) return;

	UpdateAwareness(-AwarenessDecreaseValue);

	if (CurrentAwarenessValue <= 0.0f)
	{
		bIsDecreasingAwareness = false;
		GetWorld()->GetTimerManager().ClearTimer(AwarenessDecreaseTimerHandle);

		// When awareness fully decays, transition back to UNAWARE
		if (IsValid(AICRef) && !AICRef->CheckCurrentStatusTag(E_AITag::UNAWARE))
			AICRef->UpdateCurrentStatusTag(E_AITag::UNAWARE);
	}
}

void UPerceptionsMeterComponent::StopAwareness()
{
	GetWorld()->GetTimerManager().ClearTimer(AwarenessDecreaseTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(AwarenessPauseTimerHandle);
	bCanUpdateAwareness = false;
}

float UPerceptionsMeterComponent::GetCorrectSightAwareness(const E_AISightCone SightConeType) const
{
	switch (SightConeType)
	{
		case E_AISightCone::NARROW:     
			return AwarenessIncreaseOnSight_Narrow;
		
		case E_AISightCone::WIDE:       
			return AwarenessIncreaseOnSight_Wide;
		
		case E_AISightCone::PERIPHERAL: 
			return AwarenessIncreaseOnSight_Peripheral;
		
		case E_AISightCone::BACKWARD:   
			return AwarenessIncreaseOnSight_Backward;
		
		default:                        
			return 0.0f;
	}
}

float UPerceptionsMeterComponent::GetCorrectHearingAwareness(const E_AIHearingType HearingType) const
{
	switch (HearingType)
	{
		case E_AIHearingType::WALK:
			return AwarenessIncreaseOnHearing_Walk;

		case E_AIHearingType::RUN:
			return AwarenessIncreaseOnHearing_Run;

		case E_AIHearingType::CROUCH:
			return AwarenessIncreaseOnHearing_Crouch;

		case E_AIHearingType::REPEATER:
			return AwarenessIncreaseOnHearing_Repeater;

		default:
			return 0.0f;
	}
}

void UPerceptionsMeterComponent::ResetAwareness()
{
	StopAwareness(); // Clears timers and sets bIsDecreasing to false
	
	CurrentAwarenessValue = 0.f;
	bCanUpdateAwareness = true;
	
	AICRef->UpdateCurrentStatusTag(E_AITag::UNAWARE);
}

void UPerceptionsMeterComponent::ForceAwarenessToMax()
{
	if (!IsValid(AICRef))
		return;

	CurrentAwarenessValue = MaxAwarenessValue;
	StopAwareness();
	bCanUpdateAlert = true;
	AICRef->UpdateCurrentStatusTag(E_AITag::SUSPICIOUS);

	// Start the Alert pause timer so the system can fall back if no alert stimuli arrive
	GetWorld()->GetTimerManager().SetTimer(AlertPauseTimerHandle, this, &UPerceptionsMeterComponent::OnAlertPauseFinished, AlertPauseTime, false);

	BroadcastPerceptionValues();
}

void UPerceptionsMeterComponent::UpdateAlert(const float DeltaValue, const bool bIsFromTouching)
{
	if (!IsValid(AICRef) || !bCanUpdateAlert) 
		return;

	if (DeltaValue > 0)
	{
		// Stop decreasing if we receive new input
		if (bIsDecreasingAlert)
		{
			bIsDecreasingAlert = false;
			GetWorld()->GetTimerManager().ClearTimer(AlertDecreaseTimerHandle);
		}
		
		if (AICRef->CheckCurrentStatusTag(E_AITag::HUNTING))
			AICRef->UpdateCurrentStatusTag(E_AITag::ALERTED);

		// Reset pause timer
		if (AlertPauseTimerHandle.IsValid())
			GetWorld()->GetTimerManager().ClearTimer(AlertPauseTimerHandle);

		GetWorld()->GetTimerManager().SetTimer(AlertPauseTimerHandle, this, &UPerceptionsMeterComponent::OnAlertPauseFinished, AlertPauseTime, false);
	}

	CurrentAlertValue = FMath::Clamp(CurrentAlertValue + DeltaValue, 0.f, MaxAlertValue);

	BroadcastPerceptionValues();
	
	// Check if alert meter is full
	if (CurrentAlertValue >= MaxAlertValue)
	{
		if (!AICRef->CheckCurrentStatusTag(E_AITag::ALERTED) && !AICRef->CheckCurrentStatusTag(E_AITag::HUNTING))
			AICRef->UpdateCurrentStatusTag(bIsFromTouching ? E_AITag::HUNTING : E_AITag::ALERTED);
	}
}

void UPerceptionsMeterComponent::OnAlertPauseFinished()
{
	GetWorld()->GetTimerManager().ClearTimer(AlertPauseTimerHandle);

	if (!bIsDecreasingAlert)
	{
		bIsDecreasingAlert = true;
		OnDecreaseAlert();
		GetWorld()->GetTimerManager().SetTimer(AlertDecreaseTimerHandle, this, &UPerceptionsMeterComponent::OnDecreaseAlert, AlertDecreaseTime, true);
	}
}

void UPerceptionsMeterComponent::OnDecreaseAlert()
{
	if (!bIsDecreasingAlert)
		return;

	UpdateAlert(-AlertDecreaseValue);

	if (CurrentAlertValue <= 0)
	{
		bIsDecreasingAlert = false;
		GetWorld()->GetTimerManager().ClearTimer(AlertDecreaseTimerHandle);
		
		bCanUpdateAlert = false;
		bCanUpdateAwareness = true;
		
		if (AICRef->CheckCurrentStatusTag(E_AITag::ALERTED) || AICRef->CheckCurrentStatusTag(E_AITag::HUNTING))
			AICRef->UpdateCurrentStatusTag(E_AITag::SUSPICIOUS);
		
		GetWorld()->GetTimerManager().SetTimer(AwarenessPauseTimerHandle, this, &UPerceptionsMeterComponent::OnAwarenessPauseFinished, AwarenessPauseTime, false);
	}
}

void UPerceptionsMeterComponent::StopAlert()
{
	GetWorld()->GetTimerManager().ClearTimer(AlertDecreaseTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(AlertPauseTimerHandle);
	bCanUpdateAlert = false;
}

float UPerceptionsMeterComponent::GetCorrectSightAlert(const E_AISightCone SightConeType) const
{
	switch (SightConeType)
	{
		case E_AISightCone::NARROW:     
			return AlertIncreaseOnSight_Narrow;
		
		case E_AISightCone::WIDE:       
			return AlertIncreaseOnSight_Wide;
		
		case E_AISightCone::PERIPHERAL: 
			return AlertIncreaseOnSight_Peripheral;
		
		case E_AISightCone::BACKWARD:   
			return AlertIncreaseOnSight_Backward;
		
		default:                        
			return 0.f;
	}
}

void UPerceptionsMeterComponent::ResetAlert(const bool bResetAwareness)
{
	StopAlert(); // Clears timers
	
	CurrentAlertValue = 0.f;
	bCanUpdateAlert = false;

	if (AICRef->CheckCurrentStatusTag(E_AITag::ALERTED) || AICRef->CheckCurrentStatusTag(E_AITag::HUNTING))
		AICRef->UpdateCurrentStatusTag(E_AITag::SUSPICIOUS);

	if (bResetAwareness)
		ResetAwareness();
}

void UPerceptionsMeterComponent::ForceAlertToMax()
{
	if (!IsValid(AICRef))
		return;

	// Cancel any pending decay (e.g. scheduled by a prior ForceAwarenessToMax) so the forced
	// Alerted/Hunting state doesn't immediately start counting down back to Suspicious
	GetWorld()->GetTimerManager().ClearTimer(AlertPauseTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(AlertDecreaseTimerHandle);
	bIsDecreasingAlert = false;

	CurrentAlertValue = MaxAlertValue;
	bCanUpdateAlert = true;
	AICRef->UpdateCurrentStatusTag(E_AITag::ALERTED);

	BroadcastPerceptionValues();
}

void UPerceptionsMeterComponent::ResetAll()
{
	StopAwareness();
	StopAlert();
	
	CurrentAwarenessValue = 0.0f;
	CurrentAlertValue = 0.0f;
	
	bCanUpdateAwareness = true;
	bCanUpdateAlert = false;
	bIsDecreasingAwareness = false;
	bIsDecreasingAlert = false;
	
	BroadcastPerceptionValues();
}
#pragma endregion
