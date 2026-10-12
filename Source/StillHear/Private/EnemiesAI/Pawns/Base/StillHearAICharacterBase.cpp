#include "EnemiesAI/Pawns/Base/StillHearAICharacterBase.h"

#include "NiagaraComponent.h"
#include "StillHearGameInstance.h"
#include "Components/CapsuleComponent.h"
#include "UI/Indicator/IndicatorComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnemiesAI/Utility/Patrol/Waypoint.h"
#include "EnemiesAI/Utility/BlackboardKeyNames.h"
#include "GameplayAbilitySystem/Tags/GameplayTags.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnemiesAI/Utility/DataAssets/AIInfo_DataAssetBase.h"
#include "EnemiesAI/Controllers/Base/StillHearAIControllerBase.h"

#pragma region CONSTRUCTOR
AStillHearAICharacterBase::AStillHearAICharacterBase()
{
	IndicatorComponent = CreateDefaultSubobject<UIndicatorComponent>(TEXT("IndicatorComponent"));
	
	PrimaryActorTick.bCanEverTick = true;
	CurrentSpeedType = E_AISpeedType::UNSET;

	// Enable the AI to consider paths that require jumping (like Nav Links)
	GetCharacterMovement()->NavAgentProps.bCanJump = true;
}
#pragma endregion


#pragma region METHODS
void AStillHearAICharacterBase::BeginPlay()
{
	CurrentWaypoint = StartingWaypoint;
	
	Super::BeginPlay();

	OriginalLocation = GetActorLocation();
	OriginalRotation = GetActorRotation();

	if (GetCharacterMovement())
	{
		DefaultAirControl = GetCharacterMovement()->AirControl;
	}

	if (const UWorld* World = GetWorld())
	{
		if (UStillHearGameInstance* GI = World->GetGameInstance<UStillHearGameInstance>())
			GI->OnRequestWorldReset.AddUObject(this, &ThisClass::ResetAfterDeath);
	}
}

void AStillHearAICharacterBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (const UWorld* World = GetWorld())
	{
		if (UStillHearGameInstance* GI = World->GetGameInstance<UStillHearGameInstance>())
			GI->OnRequestWorldReset.RemoveAll(this);
	}

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->RegisterGameplayTagEvent(TAG_Status_EnemyAI_Stunned, EGameplayTagEventType::NewOrRemoved).RemoveAll(this);
		AbilitySystemComponent->RegisterGameplayTagEvent(TAG_GameplayAbility_EnemyAI_Attack_Active, EGameplayTagEventType::NewOrRemoved).RemoveAll(this);
	}

	Super::EndPlay(EndPlayReason);
}

void AStillHearAICharacterBase::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AStillHearAICharacterBase::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	// Restore air control after landing
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->AirControl = DefaultAirControl;
	}
}

void AStillHearAICharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	AICRef = Cast<AStillHearAIControllerBase>(NewController);

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->RegisterGameplayTagEvent(TAG_Status_EnemyAI_Stunned, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &AStillHearAICharacterBase::HandleStun);
		AbilitySystemComponent->RegisterGameplayTagEvent(TAG_GameplayAbility_EnemyAI_Attack_Active, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &AStillHearAICharacterBase::HandleAttack);
		
		// Add default "Enemy" tag to the Ability System Component (GAS)
		AbilitySystemComponent->AddLooseGameplayTag(TAG_Enemy);
	}
}

void AStillHearAICharacterBase::ResetAfterDeath()
{
	if (!IsValid(this) || !GetWorld())
		return;

	// Cancel every active ability so no ongoing ability state bleeds into the reset
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->CancelAllAbilities();
	}

	// Restore air control in case the AI was in the middle of a jump when the reset happened
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->AirControl = DefaultAirControl;
		MoveComp->StopMovementImmediately();
	}

	// Restore original transform
	SetActorLocationAndRotation(OriginalLocation, OriginalRotation, false, nullptr, ETeleportType::TeleportPhysics);

	if (AICRef)
	{
		AICRef->ResetAIState();
		AICRef->GetBlackboardComponent()->SetValueAsObject(BlackboardKeyNames::KeyNameCurrentWaypoint, StartingWaypoint);
	}
}

void AStillHearAICharacterBase::ChangeSpeedType(const E_AISpeedType NewSpeed)
{
	if (CurrentSpeedType != NewSpeed && IsValid(GetAIInfo_DataAsset()))
	{
		switch (NewSpeed) 
		{
			case E_AISpeedType::WALK:

				CurrentSpeedType = E_AISpeedType::WALK;
				GetCharacterMovement()->MaxWalkSpeed = GetAIInfo_DataAsset()->WalkSpeed;

				break;

			case E_AISpeedType::RUN:

				CurrentSpeedType = E_AISpeedType::RUN;
				GetCharacterMovement()->MaxWalkSpeed = GetAIInfo_DataAsset()->RunSpeed;

				break;
				
			default: 
				
				break;
		}
	}
}

void AStillHearAICharacterBase::HandleStun(const FGameplayTag Tag, const int32 NewCount)
{
	if (NewCount <= 0) // if NewCount is greater than 0, it means the tag was added
	{
		GetAICRef()->UpdateCurrentStatusTag(E_AITag::UNAWARE);
	}
	else
	{
		GetAICRef()->UpdateCurrentStatusTag(E_AITag::STUNNED);
	}
}

void AStillHearAICharacterBase::HandleAttack(const FGameplayTag Tag, const int32 NewCount)
{
	if (!AICRef)
	{
		AICRef = Cast<AStillHearAIControllerBase>(GetController());
	}

	if (AICRef)
	{
		if (NewCount > 0)
		{
			AICRef->OnStatusTagChanged.Broadcast(TAG_GameplayAbility_EnemyAI_Attack_Active);
		}
		else
		{
			AICRef->OnStatusTagChanged.Broadcast(AICRef->GetCurrentStatusTag());
		}
	}
}

void AStillHearAICharacterBase::SetCollision(const TArray<TEnumAsByte<ECollisionChannel>>& Channels, bool bIgnore)
{
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		ECollisionResponse Response = bIgnore ? ECR_Ignore : ECR_Block;
		for (ECollisionChannel Channel : Channels)
		{
			Capsule->SetCollisionResponseToChannel(Channel, Response);
		}
	}
}
#pragma endregion
