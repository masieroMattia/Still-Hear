#include "EnemiesAI/BehaviorTreeServices/Mantis/BTService_MantisSelectAttack.h"

#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "EnemiesAI/Utility/AIEnum.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnemiesAI/Utility/BlackboardKeyNames.h"
#include "EnemiesAI/Pawns/Base/StillHearAICharacterBase.h"
#include "EnemiesAI/Utility/DataAssets/Abilities/MantisShiftData.h"
#include "EnemiesAI/Utility/DataAssets/Abilities/CloseAttackData.h"
#include "GameplayAbilitySystem/Abilities/EnemiesAI/Mantis/GA_MantisShift.h"
#include "GameplayAbilitySystem/Abilities/EnemiesAI/Mantis/GA_CloseAttack.h"

#pragma region CONSTRUCTOR
UBTService_MantisSelectAttack::UBTService_MantisSelectAttack()
{
	NodeName = TEXT("Mantis Select Attack");
	bNotifyTick = true;
	bNotifyBecomeRelevant = true;
}
#pragma endregion

#pragma region METHODS
void UBTService_MantisSelectAttack::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	bDataCached = false;
	CacheDataAssets(OwnerComp);

	// Compute the correct attack type immediately so the BT evaluates with the right value
	TickNode(OwnerComp, NodeMemory, 0.f);
}

void UBTService_MantisSelectAttack::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, const float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	if (!bDataCached)
		CacheDataAssets(OwnerComp);

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
		return;

	// Get target actor from blackboard
	const AActor* TargetActor = Cast<AActor>(BB->GetValueAsObject(BlackboardKeyNames::KeyNameTargetActor));
	if (!TargetActor)
	{
		BB->SetValueAsEnum(BlackboardKeyNames::KeyNameAttackType, static_cast<uint8>(E_MantisAttackType::NONE));
		return;
	}

	// Get the owning pawn
	const AAIController* Controller = OwnerComp.GetAIOwner();
	const APawn* Pawn = Controller ? Controller->GetPawn() : nullptr;
	if (!Pawn)
	{
		BB->SetValueAsEnum(BlackboardKeyNames::KeyNameAttackType, static_cast<uint8>(E_MantisAttackType::NONE));
		return;
	}

	const float Distance = FVector::Dist(Pawn->GetActorLocation(), TargetActor->GetActorLocation());
	const float Distance2D = FVector::Dist2D(Pawn->GetActorLocation(), TargetActor->GetActorLocation());
	const float ZDiff = FMath::Abs(Pawn->GetActorLocation().Z - TargetActor->GetActorLocation().Z);

	// Determine attack type based on distance thresholds
	// Priority: Close > Shift > None (chase)
	E_MantisAttackType AttackType = E_MantisAttackType::NONE;

	// Use 2D distance for close attack if Z difference is acceptable (e.g. less than 150 units, allows hitting on low walls)
	bool bCanCloseAttack = false;
	if (CachedCloseData.IsValid())
	{
		if (Distance <= CachedCloseData->CloseAttackDistance)
		{
			bCanCloseAttack = true;
		}
		else if (Distance2D <= CachedCloseData->CloseAttackDistance && ZDiff < 200.f)
		{
			bCanCloseAttack = true;
		}
	}

	if (bCanCloseAttack)
		AttackType = E_MantisAttackType::CLOSE_ATTACK;
	
	else if (CachedShiftData.IsValid() && Distance >= CachedShiftData->MinShiftDistance && Distance <= CachedShiftData->MaxShiftDistance)
		AttackType = E_MantisAttackType::SHIFT;

	BB->SetValueAsEnum(BlackboardKeyNames::KeyNameAttackType, static_cast<uint8>(AttackType));
}

void UBTService_MantisSelectAttack::CacheDataAssets(const UBehaviorTreeComponent& OwnerComp)
{
	const AAIController* Controller = OwnerComp.GetAIOwner();
	const AStillHearAICharacterBase* Pawn = Controller ? Cast<AStillHearAICharacterBase>(Controller->GetPawn()) : nullptr;
	if (!Pawn)
		return;

	UAbilitySystemComponent* ASC = Pawn->GetAbilitySystemComponent();
	if (!ASC)
		return;

	// Iterate granted abilities and cache data assets
	for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		const UGameplayAbility* Ability = Spec.Ability;
		if (!Ability)
			continue;

		if (const UGA_CloseAttack* CloseAbility = Cast<UGA_CloseAttack>(Ability))
		{
			if (const UCloseAttackData* CloseData = Cast<UCloseAttackData>(CloseAbility->GetAttackData()))
				CachedCloseData = CloseData;
		}
		else if (const UGA_MantisShift* ShiftAbility = Cast<UGA_MantisShift>(Ability))
		{
			if (const UMantisShiftData* ShiftData = ShiftAbility->GetShiftData())
			{
				// Cache only the "Attack" version of the Shift for the Behavior Tree
				if (ShiftData->bPerformAttackAfterShift)
				{
					CachedShiftData = ShiftData;
				}
			}
		}
	}

	bDataCached = CachedCloseData.IsValid() || CachedShiftData.IsValid();
}
#pragma endregion
