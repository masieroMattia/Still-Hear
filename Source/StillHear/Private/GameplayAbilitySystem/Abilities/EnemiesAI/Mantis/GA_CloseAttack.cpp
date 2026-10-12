#include "GameplayAbilitySystem/Abilities/EnemiesAI/Mantis/GA_CloseAttack.h"

#include "GameplayAbilitySystem/Tags/GameplayTags.h"
#include "EnemiesAI/Pawns/Mantis/AIMantisCharacter.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "EnemiesAI/Utility/DataAssets/Abilities/CloseAttackData.h"

#pragma region CONSTRUCTORS
UGA_CloseAttack::UGA_CloseAttack()
{
	FGameplayTagContainer CurrentAssetTags = GetAssetTags();
	CurrentAssetTags.AddTag(TAG_GameplayAbility_EnemyAI_CloseAttack);
	SetAssetTags(CurrentAssetTags);

	ActivationOwnedTags.AddTag(TAG_GameplayAbility_EnemyAI_CloseAttack_Active);
}
#pragma endregion

#pragma region METHODS
void UGA_CloseAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CachedMantis.IsValid() || !CurrentTarget.IsValid())
		return;

	const UCloseAttackData* CloseData = Cast<UCloseAttackData>(AttackData);
	if (!CloseData || !CloseData->CloseAttackMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}

	// Predict target location and rotate towards it
	const FVector TargetLoc = CurrentTarget->GetActorLocation();
	const FVector Velocity = CurrentTarget->GetVelocity();
	FVector PredictedLoc = TargetLoc;

	if (Velocity.SizeSquared2D() > 100.0f)
	{
		PredictedLoc += Velocity * CloseData->PredictionTimeDelay * CloseData->PredictionFactor;
	}

	const FVector DirToTarget = (PredictedLoc - CachedMantis->GetActorLocation()).GetSafeNormal2D();
	if (!DirToTarget.IsNearlyZero())
	{
		CachedMantis->SetActorRotation(DirToTarget.Rotation());
	}
	
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		CloseData->CloseAttackMontage,
		1.0f
	);

	MontageTask->OnCompleted.AddDynamic(this, &UGA_CloseAttack::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_CloseAttack::OnMontageCancelled);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_CloseAttack::OnMontageCancelled);

	// Play the attack feedback VFX as soon as the attack actually starts
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		ASC->ExecuteGameplayCue(TAG_GameplayCue_EnemyAI_AttackFeedback);

	// Start ticking tracking logic
	GetWorld()->GetTimerManager().SetTimer(TrackingTimerHandle, this, &UGA_CloseAttack::TrackTargetTick, 0.01f, true);

	MontageTask->ReadyForActivation();

	const float MontageLength = CloseData->CloseAttackMontage->GetPlayLength();
	GetWorld()->GetTimerManager().SetTimer(SafetyTimerHandle, this, &UGA_CloseAttack::OnSafetyTimeout, MontageLength + 0.5f, false);
}

void UGA_CloseAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	GetWorld()->GetTimerManager().ClearTimer(TrackingTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(SafetyTimerHandle);

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
#pragma endregion

#pragma region UFUNCTIONS
void UGA_CloseAttack::TrackTargetTick()
{
	if (!CachedMantis.IsValid() || !CurrentTarget.IsValid() || !AttackData)
	{
		GetWorld()->GetTimerManager().ClearTimer(TrackingTimerHandle);
		return;
	}

	const UCloseAttackData* CloseData = Cast<UCloseAttackData>(AttackData);
	if (!CloseData) 
		return;
	
	const FVector TargetLoc = CurrentTarget->GetActorLocation();
	const FVector Velocity = CurrentTarget->GetVelocity();
	FVector PredictedLoc = TargetLoc;

	if (Velocity.SizeSquared2D() > 100.0f)
	{
		PredictedLoc += Velocity * CloseData->PredictionTimeDelay * CloseData->PredictionFactor;
	}

	const FRotator CurrentRot = CachedMantis->GetActorRotation();
	const FRotator TargetRot = (PredictedLoc - CachedMantis->GetActorLocation()).GetSafeNormal2D().Rotation();
	
	const FRotator NewRotation = FMath::RInterpTo(CurrentRot, TargetRot, 0.01f, CloseData->TrackingRotationSpeed);

	CachedMantis->SetActorRotation(NewRotation);
}

void UGA_CloseAttack::OnMontageCompleted()
{
	ApplyCooldown(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo);
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UGA_CloseAttack::OnMontageCancelled()
{
	DisableHitBox();
	ApplyCooldown(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo);
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, true);
}

void UGA_CloseAttack::OnSafetyTimeout()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, true);
}
#pragma endregion