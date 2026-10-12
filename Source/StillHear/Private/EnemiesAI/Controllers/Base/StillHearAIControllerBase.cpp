#include "EnemiesAI/Controllers/Base/StillHearAIControllerBase.h"

#include "AbilitySystemComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "EnemiesAI/Utility/Patrol/Waypoint.h"
#include "Navigation/CrowdFollowingComponent.h"
#include "EnemiesAI/Utility/BlackboardKeyNames.h"
#include "GameplayAbilitySystem/Tags/GameplayTags.h"
#include "EnemiesAI/Pawns/Base/StillHearAICharacterBase.h"

AStillHearAIControllerBase::AStillHearAIControllerBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.SetDefaultSubobjectClass<UCrowdFollowingComponent>(TEXT("PathFollowingComponent")))
{
	Cast<UCrowdFollowingComponent>(GetPathFollowingComponent())->SetCrowdAvoidanceQuality(ECrowdAvoidanceQuality::High);

	AAIController::SetGenericTeamId(FGenericTeamId(1));
	
	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AI Perception Component"));

	CurrentStatusTag = TAG_Status_EnemyAI_Unaware;
}

void AStillHearAIControllerBase::UpdateCurrentStatusTag(const E_AITag NewTag)
{
	if (GetNPCRef() && GetNPCRef()->GetAbilitySystemComponent())
	{	
		GetNPCRef()->GetAbilitySystemComponent()->RemoveLooseGameplayTag(TAG_Status_EnemyAI_Alerted);
	}
	
	switch (NewTag)
	{
		case E_AITag::UNAWARE:
		{
			CurrentStatusTag = TAG_Status_EnemyAI_Unaware;
			break;
		}
		
		case E_AITag::STUNNED:
		{
			CurrentStatusTag = TAG_Status_EnemyAI_Stunned;
			break;
		}

		case E_AITag::SUSPICIOUS:
		{
			CurrentStatusTag = TAG_Status_EnemyAI_Suspicious;
			break;	
		}

		case E_AITag::ALERTED:
		{
			CurrentStatusTag = TAG_Status_EnemyAI_Alerted;
			GetNPCRef()->GetAbilitySystemComponent()->AddLooseGameplayTag(TAG_Status_EnemyAI_Alerted);	
			break;	
		}

		case E_AITag::HUNTING:
		{
			CurrentStatusTag = TAG_Status_EnemyAI_Hunting;
			break;	
		}
		
		default:
		{
			
			break;
		}
	}

	if (IsValid(Blackboard))
		Blackboard->SetValueAsName(BlackboardKeyNames::KeyNameCurrentStatusTag, CurrentStatusTag.GetTagName());

	// If attacking, don't broadcast the normal status tag to the UI
	if (NewTag != E_AITag::STUNNED && NPCRef && NPCRef->GetAbilitySystemComponent())
	{
		if (NPCRef->GetAbilitySystemComponent()->HasMatchingGameplayTag(TAG_GameplayAbility_EnemyAI_Attack_Active))
		{
			return;
		}
	}

	OnStatusTagChanged.Broadcast(CurrentStatusTag);
}

bool AStillHearAIControllerBase::CheckCurrentStatusTag(const E_AITag TagToCheck) const
{
	switch (TagToCheck)
	{
		case E_AITag::UNAWARE:
		{
			return CurrentStatusTag.MatchesTagExact(TAG_Status_EnemyAI_Unaware);
		}

		case E_AITag::STUNNED:
		{
			return CurrentStatusTag.MatchesTagExact(TAG_Status_EnemyAI_Stunned);
		}
		
		case E_AITag::SUSPICIOUS:
		{
			return CurrentStatusTag.MatchesTagExact(TAG_Status_EnemyAI_Suspicious);
		}

		case E_AITag::ALERTED:
		{
			return CurrentStatusTag.MatchesTagExact(TAG_Status_EnemyAI_Alerted);
		}

		case E_AITag::HUNTING:
		{
			return CurrentStatusTag.MatchesTagExact(TAG_Status_EnemyAI_Hunting);
		}
		
		default:
		{
			
			break;
		}
	}

	return false;
}

// Called when the game starts or when spawned
void AStillHearAIControllerBase::BeginPlay()
{
	Super::BeginPlay();

	RunBehaviorTree(BehaviorTree);

	UpdateCurrentStatusTag(E_AITag::UNAWARE);

	BeginPlayHappened = true;
	SetupBlackboardKeys();
}

void AStillHearAIControllerBase::OnPossess(APawn* InPawn)
{
	NPCRef = Cast<AStillHearAICharacterBase>(InPawn);

	PossessHappened = true;
	SetupBlackboardKeys();
	
	SetupSightInfo();
	SetupHearingInfo();
	
	Super::OnPossess(InPawn);

	AIPerceptionComp->OnTargetPerceptionUpdated.AddUniqueDynamic(this, &ThisClass::PerceptionEventReceived);

}

void AStillHearAIControllerBase::ResetAIState()
{
	UpdateCurrentStatusTag(E_AITag::UNAWARE);
	OnStatusTagChanged.Broadcast(CurrentStatusTag);
	
	if (GetBlackboardComponent())
	{
		GetBlackboardComponent()->ClearValue(BlackboardKeyNames::KeyNameTargetActor);
		GetBlackboardComponent()->ClearValue(BlackboardKeyNames::KeyNameTargetLocation);
		GetBlackboardComponent()->SetValueAsBool(BlackboardKeyNames::KeyNameHasTouched, false);
	}
}

void AStillHearAIControllerBase::SetupBlackboardKeys()
{
	if (!BeginPlayHappened || !PossessHappened)
		return;

	if (!IsValid(NPCRef) || !IsValid(GetBlackboardComponent()))
		return;
	
	if (NPCRef->GetStartingWaypoint())
	{
		Blackboard->SetValueAsObject(BlackboardKeyNames::KeyNameCurrentWaypoint, NPCRef->GetStartingWaypoint());
		Blackboard->SetValueAsFloat(BlackboardKeyNames::KeyNameWaypointWaitTime, NPCRef->GetStartingWaypoint()->GetWaitTime());
	}
}

// Called every frame
void AStillHearAIControllerBase::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);
}

ETeamAttitude::Type AStillHearAIControllerBase::GetTeamAttitudeTowards(const AActor& Other) const
{

	/*
	 * As a default implementation, anything that is not a Pawn or doesn't implement the IGenericTeamAgentInterface
	 * is automatically treated as Hostile.
	 * We decide here that anything belonging to team 255 is Neutral, while anything that has the same id as us is friendly.
	 * Any other Team ID is Hostile.
	 */
	
	if (const APawn* OtherPawn = Cast<APawn>(&Other))
	{
		if (const IGenericTeamAgentInterface* TeamAgent = Cast<IGenericTeamAgentInterface>(OtherPawn->GetController()))
		{
			const FGenericTeamId OtherTeamID = TeamAgent->GetGenericTeamId();
			if (OtherTeamID == 255)
			{
				return ETeamAttitude::Neutral;
			}
			if (OtherTeamID == GetGenericTeamId())
			{
				return ETeamAttitude::Friendly;
			}
		}
	}

	return ETeamAttitude::Hostile;
}

