#include "GameplayAbilitySystem/Abilities/EnemiesAI/Mantis/GA_MantisShift.h"

#include "AIController.h"
#include "TimerManager.h"
#include "BrainComponent.h"
#include "NavigationSystem.h"
#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Subsystems/TimeManagementSubsystem.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "EnemiesAI/Utility/BlackboardKeyNames.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnemiesAI/Pawns/Mantis/AIMantisCharacter.h"
#include "EnemiesAI/Controllers/Mantis/AIMantisController.h"
#include "EnemiesAI/Utility/DataAssets/Abilities/MantisShiftData.h"

#pragma region CONSTRUCTOR
UGA_MantisShift::UGA_MantisShift()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	// Tags needed for the Ability System to properly identify and handle this ability
    FGameplayTagContainer InitialTags;
    InitialTags.AddTag(FGameplayTag::RequestGameplayTag(FName("GameplayAbility.EnemyAI.MantisShift")));
    SetAssetTags(InitialTags);
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("GameplayAbility.EnemyAI.MantisShift.Active")));
}
#pragma endregion

#pragma region METHODS
void UGA_MantisShift::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	CachedMantis = Cast<AAIMantisCharacter>(ActorInfo->AvatarActor.Get());
	if (!CachedMantis.IsValid() || !ShiftData)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ShiftStartingLocation = CachedMantis->GetActorLocation();
	ShiftStartingRotation = CachedMantis->GetActorRotation();

	bForceHuntOnLanding = false;
	ForceHuntTargetActor = nullptr;

	if (UCharacterMovementComponent* MoveComp = CachedMantis->GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
		MoveComp->SetMovementMode(MOVE_None); // Completely freezes movement and falling
	}
	
	// Distinguish whether it is a Nav Link Traversal based on the tag or the presence of a TargetActor
	if (TriggerEventData)
	{
		bIsNavTraversal = TriggerEventData->EventTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("GameplayAbility.EnemyAI.MantisShift.Nav")));
		
		if (bIsNavTraversal)
		{
			if (TriggerEventData->TargetData.Num() > 0)
			{
				const FGameplayAbilityTargetData* TargetData = TriggerEventData->TargetData.Get(0);
				LandingTarget = TargetData->GetEndPoint();

				if (TargetData->GetScriptStruct() == FGameplayAbilityTargetData_LocationInfo::StaticStruct())
				{
					const auto* LocationData = static_cast<const FGameplayAbilityTargetData_LocationInfo*>(TargetData);
					LandingRotation = LocationData->TargetLocation.LiteralTransform.Rotator();
				}
			}

			// Optional second target data entry: an actor to fully reset state and force-hunt as soon as we land
			if (TriggerEventData->TargetData.Num() > 1)
			{
				const FGameplayAbilityTargetData* HuntData = TriggerEventData->TargetData.Get(1);
				if (HuntData && HuntData->GetScriptStruct() == FGameplayAbilityTargetData_ActorArray::StaticStruct())
				{
					const auto* ActorArrayData = static_cast<const FGameplayAbilityTargetData_ActorArray*>(HuntData);
					if (ActorArrayData->TargetActorArray.Num() > 0 && ActorArrayData->TargetActorArray[0].IsValid())
					{
						bForceHuntOnLanding = true;
						ForceHuntTargetActor = ActorArrayData->TargetActorArray[0].Get();
					}
				}
			}

			CurrentTarget = nullptr;
		}
		else
		{
			CurrentTarget = const_cast<AActor*>(TriggerEventData->Target.Get());
		}
	}
	else
	{
		// Fallback: If launched directly from the Behavior Tree Node, TriggerEventData is nullptr
		// It's definitely an Attack context, so let's fish the target from Blackboard
		bIsNavTraversal = false;
		
		if (const AAIController* AIController = Cast<AAIController>(CachedMantis->GetController()))
		{
			if (const UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent())
			{
				CurrentTarget = Cast<AActor>(Blackboard->GetValueAsObject(BlackboardKeyNames::KeyNameTargetActor));
			}
		}
	}

	// Initial Cue: Burst (when it begins to fade out)
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		FGameplayCueParameters CueParams;
		CueParams.Instigator = CachedMantis.Get();
		CueParams.EffectCauser = CachedMantis.Get();
		CueParams.TargetAttachComponent = CachedMantis->GetRootComponent();
		CueParams.Location = CachedMantis->GetActorLocation();
		CueParams.EffectContext = ASC->MakeEffectContext();
		CueParams.RawMagnitude = ShiftData->FadeOutDuration;
		CueParams.NormalizedMagnitude = 0.0f; // disappearing
		ASC->ExecuteGameplayCue(ShiftData->ShiftBurstCueTag, CueParams);
	}
	
	// Start fade out timer and tick
	FadeElapsedTime = 0.f;
	
	if (!bIsNavTraversal && ShiftData->FadeOutTimeCurve)
	{
		if (UTimeManagementSubsystem* TimeSys = GetWorld()->GetSubsystem<UTimeManagementSubsystem>())
		{
			TimeSys->PlayTimeCurve(ShiftData->FadeOutTimeCurve);
		}
	}
	
	GetWorld()->GetTimerManager().SetTimer(FadeTimerHandle, this, &UGA_MantisShift::FadeOutTick, 0.01f, true);
	GetWorld()->GetTimerManager().SetTimer(PhaseOutTimerHandle, this, &UGA_MantisShift::OnFadeOutComplete, ShiftData->FadeOutDuration, false);
}

void UGA_MantisShift::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (CachedMantis.IsValid())
	{
		RestoreAICollision();

		if (UCharacterMovementComponent* MoveComp = CachedMantis->GetCharacterMovement())
		{
			if (MoveComp->MovementMode == MOVE_None)
			{
				MoveComp->SetMovementMode(MOVE_Walking);
			}
		}
		
		if (ShiftData)
		{
			if (USkeletalMeshComponent* Mesh = CachedMantis->GetMesh())
			{
				Mesh->SetScalarParameterValueOnMaterials(ShiftData->FadeOpacityParameterName, 1.0f);
			}
			
			// Make sure to remove the Loop Cue if it hasn't been properly completed
			if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
			{
				FGameplayCueParameters CueParams;
				ASC->RemoveGameplayCue(ShiftData->ShiftLoopCueTag);
			}
		}
	}

	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_MantisShift::FadeOutTick()
{
	if (!CachedMantis.IsValid() || !ShiftData) return;
	FadeElapsedTime += 0.01f;

	const float Alpha = FMath::Clamp(FadeElapsedTime / ShiftData->FadeOutDuration, 0.0f, 1.0f);

	float Opacity = FMath::Lerp(1.0f, 0.0f, Alpha);
	if (ShiftData->DissolveCurve)
	{
		Opacity = ShiftData->DissolveCurve->GetFloatValue(Opacity);
	}

	if (USkeletalMeshComponent* Mesh = CachedMantis->GetMesh())
	{
		Mesh->SetScalarParameterValueOnMaterials(ShiftData->FadeOpacityParameterName, Opacity);
	}
}

void UGA_MantisShift::OnFadeOutComplete()
{
	if (!CachedMantis.IsValid() || !ShiftData)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(FadeTimerHandle);

	// Intermediate Cue: start the Loop (until it reappears)
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		FGameplayCueParameters CueParams;
		CueParams.Instigator = CachedMantis.Get();
		CueParams.EffectCauser = CachedMantis.Get();
		CueParams.TargetAttachComponent = CachedMantis->GetRootComponent();
		CueParams.Location = CachedMantis->GetActorLocation();
		CueParams.EffectContext = ASC->MakeEffectContext();
		ASC->AddGameplayCue(ShiftData->ShiftLoopCueTag, CueParams);
	}

	FVector BaseTargetLocation = LandingTarget;
	FRotator TargetRotation = bIsNavTraversal ? LandingRotation : ShiftStartingRotation;

	if (!bIsNavTraversal && CurrentTarget.IsValid())
	{
		// Try to land near the enemy and face towards them
		FVector PredictedTargetLoc = PredictTargetLocation(CurrentTarget->GetActorLocation());
		
		// The direction from PredictedTargetLoc TO Mantis (if Mantis should land around the target)
		FVector DirFromTarget = (ShiftStartingLocation - PredictedTargetLoc).GetSafeNormal2D();
		if (DirFromTarget.IsNearlyZero())
		{
			DirFromTarget = CurrentTarget->GetActorForwardVector();
		}

		// Calculate exact landing spot by placing it at LandingDistanceOffset away from the prediction
		BaseTargetLocation = PredictedTargetLoc + DirFromTarget * ShiftData->LandingDistanceOffset;
		
		TargetRotation = (-DirFromTarget).Rotation();
	}

	// Collision handling and calculating target
	DisableAICollision();
	ComputeLandingTarget(BaseTargetLocation);
	
	// Apply rotation immediately, translation will happen in tick
	CachedMantis->SetActorRotation(TargetRotation);

	// Start translation tick and fade in timer
	TranslationElapsedTime = 0.f;

	if (!bIsNavTraversal && ShiftData->TranslationTimeCurve)
	{
		if (UTimeManagementSubsystem* TimeSys = GetWorld()->GetSubsystem<UTimeManagementSubsystem>())
		{
			TimeSys->PlayTimeCurve(ShiftData->TranslationTimeCurve);
		}
	}

	GetWorld()->GetTimerManager().SetTimer(TranslationTimerHandle, this, &UGA_MantisShift::TranslateToTargetTick, 0.01f, true);
	GetWorld()->GetTimerManager().SetTimer(TeleportTimerHandle, this, &UGA_MantisShift::OnTranslationComplete, ShiftData->TranslationDuration, false);
}

void UGA_MantisShift::TranslateToTargetTick()
{
	if (!CachedMantis.IsValid() || !ShiftData) return;
	
	TranslationElapsedTime += 0.01f;

	const float Alpha = FMath::Clamp(TranslationElapsedTime / ShiftData->TranslationDuration, 0.0f, 1.0f);

	const FVector NewLocation = FMath::Lerp(ShiftStartingLocation, LandingTarget, Alpha);
	FRotator NewRotation = CachedMantis->GetActorRotation();

	if (!bIsNavTraversal && CurrentTarget.IsValid())
	{
		// Continuously rotate towards the target during translation
		const FVector DirToTarget = (CurrentTarget->GetActorLocation() - NewLocation).GetSafeNormal2D();
		if (!DirToTarget.IsNearlyZero())
		{
			NewRotation = FMath::RInterpTo(NewRotation, DirToTarget.Rotation(), 0.01f, 50.0f); // Fast and smooth rotation
		}
	}

	CachedMantis->SetActorLocationAndRotation(NewLocation, NewRotation, false, nullptr, ETeleportType::TeleportPhysics);
}

void UGA_MantisShift::OnTranslationComplete()
{
	GetWorld()->GetTimerManager().ClearTimer(TranslationTimerHandle);

	if (!bIsNavTraversal && CurrentTarget.IsValid())
	{
		// Force exact rotation to the target at the end of translation
		const FVector DirToTarget = (CurrentTarget->GetActorLocation() - CachedMantis->GetActorLocation()).GetSafeNormal2D();
		if (!DirToTarget.IsNearlyZero())
		{
			CachedMantis->SetActorRotation(DirToTarget.Rotation());
		}
	}

	// Now that the AI has actually arrived, wipe its previous state and force it straight into Hunting
	if (bForceHuntOnLanding && ForceHuntTargetActor.IsValid())
	{
		if (AAIMantisController* MantisAIC = Cast<AAIMantisController>(CachedMantis->GetController()))
		{
			MantisAIC->ResetAIState();
			MantisAIC->ForceHuntTarget(ForceHuntTargetActor.Get());

			// Let the AI start chasing immediately instead of waiting for the (purely visual) fade-in
			RestoreAICollision();
			if (UCharacterMovementComponent* MoveComp = CachedMantis->GetCharacterMovement())
			{
				MoveComp->SetMovementMode(MOVE_Walking);
			}

			if (UBrainComponent* BrainComp = MantisAIC->GetBrainComponent())
				BrainComp->RestartLogic();
		}
	}

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->RemoveGameplayCue(ShiftData->ShiftLoopCueTag);
	}

	// Start Fade In ticking
	FadeElapsedTime = 0.f;

	if (!bIsNavTraversal && ShiftData->FadeInTimeCurve)
	{
		if (UTimeManagementSubsystem* TimeSys = GetWorld()->GetSubsystem<UTimeManagementSubsystem>())
		{
			TimeSys->PlayTimeCurve(ShiftData->FadeInTimeCurve);
		}
	}

	GetWorld()->GetTimerManager().SetTimer(FadeTimerHandle, this, &UGA_MantisShift::FadeInTick, 0.01f, true);
	GetWorld()->GetTimerManager().SetTimer(PhaseOutTimerHandle, this, &UGA_MantisShift::OnFadeInComplete, ShiftData->FadeInDuration, false);
}

void UGA_MantisShift::FadeInTick()
{
	if (!CachedMantis.IsValid() || !ShiftData) return;
	FadeElapsedTime += 0.01f;

	const float Alpha = FMath::Clamp(FadeElapsedTime / ShiftData->FadeInDuration, 0.0f, 1.0f);

	float Opacity = FMath::Lerp(0.0f, 1.0f, Alpha);
	if (ShiftData->DissolveCurve)
	{
		Opacity = ShiftData->DissolveCurve->GetFloatValue(Opacity);
	}

	if (USkeletalMeshComponent* Mesh = CachedMantis->GetMesh())
	{
		Mesh->SetScalarParameterValueOnMaterials(ShiftData->FadeOpacityParameterName, Opacity);
	}
}

void UGA_MantisShift::OnFadeInComplete()
{
	GetWorld()->GetTimerManager().ClearTimer(FadeTimerHandle);
	RestoreAICollision();

	if (CachedMantis.IsValid())
	{
		if (UCharacterMovementComponent* MoveComp = CachedMantis->GetCharacterMovement())
		{
			MoveComp->SetMovementMode(MOVE_Walking);
		}

		// 3. Final Cue: remove Loop and execute Burst (when it fully reappears)
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			FGameplayCueParameters CueParams;
			CueParams.Instigator = CachedMantis.Get();
			CueParams.EffectCauser = CachedMantis.Get();
			CueParams.TargetAttachComponent = CachedMantis->GetRootComponent();
			CueParams.Location = CachedMantis->GetActorLocation();
			CueParams.EffectContext = ASC->MakeEffectContext();
			CueParams.RawMagnitude = ShiftData->FadeInDuration;
			CueParams.NormalizedMagnitude = 1.0f; // reappearing
			ASC->ExecuteGameplayCue(ShiftData->ShiftBurstCueTag, CueParams);
		}
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

bool UGA_MantisShift::ComputeLandingTarget(const FVector& BaseTarget)
{
	if (!CachedMantis.IsValid() || !ShiftData) return false;

	float CapsuleHalfHeight = 0.0f;
	if (UCapsuleComponent* Capsule = CachedMantis->GetCapsuleComponent())
	{
		CapsuleHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
	}

	if (bIsNavTraversal)
	{
		LandingTarget = BaseTarget;
		LandingTarget.Z += CapsuleHalfHeight + ShiftData->LandingZOffset;
		return true;
	}

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if(!NavSys) return false;
	
	FNavLocation NavLocation;
	if (NavSys->ProjectPointToNavigation(BaseTarget, NavLocation, ShiftData->NavMeshQueryExtent))
	{
		LandingTarget = NavLocation.Location;
		LandingTarget.Z += CapsuleHalfHeight + ShiftData->LandingZOffset;
		return true;
	}
	
	// Fallback 1: Try projecting with a larger extent in case the offset pushed it out of bounds
	if (NavSys->ProjectPointToNavigation(BaseTarget, NavLocation, FVector(800.0f, 800.0f, 1000.0f)))
	{
		LandingTarget = NavLocation.Location;
		LandingTarget.Z += CapsuleHalfHeight + ShiftData->LandingZOffset;
		return true;
	}

	// Fallback 2: If we really can't find a spot around the player, just use the player's ground location
	if (!bIsNavTraversal && CurrentTarget.IsValid())
	{
		if (NavSys->ProjectPointToNavigation(CurrentTarget->GetActorLocation(), NavLocation, ShiftData->NavMeshQueryExtent))
		{
			LandingTarget = NavLocation.Location;
			LandingTarget.Z += CapsuleHalfHeight + ShiftData->LandingZOffset;
			return true;
		}
	}
	
	// Absolute worst case
	LandingTarget = BaseTarget;
	LandingTarget.Z += CapsuleHalfHeight + ShiftData->LandingZOffset;
	return false;
}

FVector UGA_MantisShift::PredictTargetLocation(const FVector& TargetLoc) const
{
	if(!CurrentTarget.IsValid() || !ShiftData) return TargetLoc;

	// Calculate a point IN FRONT of the player's movement or face direction
	const FVector Velocity = CurrentTarget->GetVelocity();
	FVector PredictedLoc = TargetLoc;
	
	if (Velocity.SizeSquared2D() > 100.0f)
	{
		PredictedLoc += Velocity * ShiftData->PredictionTimeDelay * ShiftData->PredictionFactor;
	}
	else
	{
		// Fallback if target is stationary: shift in front of their facing direction
		PredictedLoc += CurrentTarget->GetActorForwardVector() * (ShiftData->LandingDistanceOffset * 0.5f);
	}
	
	return PredictedLoc;
}

void UGA_MantisShift::DisableAICollision() const
{
	if(CachedMantis.IsValid() && CachedMantis->GetCapsuleComponent())
	{
		CachedMantis->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void UGA_MantisShift::RestoreAICollision() const
{
	if(CachedMantis.IsValid() && CachedMantis->GetCapsuleComponent())
	{
		CachedMantis->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
}
#pragma endregion

