#include "GameplayAbilitySystem/Abilities/Companion/GA_SoundWave.h"

#include "Interfaces/Targetable.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemComponent.h"
#include "Perception/AISense_Hearing.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Data/DataAssets/SoundWaveData.h"
#include "Character/FloatingCompanionComponent.h"
#include "Interactions/Components/TargetSpot.h"
#include "GameplayAbilitySystem/Tags/GameplayTags.h"
#include "Camera/CameraEffects/CameraEffectsComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GameFramework/ForceFeedbackEffect.h"

#pragma region CONSTRUCTOR
UGA_SoundWave::UGA_SoundWave()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	// "Name of the ability" TAG
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(TAG_GameplayAbility_Companion_SoundWave);
	SetAssetTags(AssetTags);
	
	// When the ability is activated, these tags are added to the owner
	ActivationOwnedTags.AddTag(TAG_GameplayAbility_Companion_SoundWave_Active);
	ActivationOwnedTags.AddTag(TAG_Status_Companion_Aiming);
}
#pragma endregion
	
#pragma region METHODS
void UGA_SoundWave::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	CachedPlayerController = nullptr;
    CachedCompanionComponent = nullptr;
	
	// Try to get the Player Controller directly from the ActorInfo (if a Player uses this ability)
	if (ActorInfo && ActorInfo->PlayerController.IsValid())
		CachedPlayerController = ActorInfo->PlayerController.Get();

	const AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!CachedPlayerController || !AvatarActor)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	CachedCompanionComponent = AvatarActor->GetComponentByClass(UFloatingCompanionComponent::StaticClass()) ? Cast<UFloatingCompanionComponent>(AvatarActor->GetComponentByClass(UFloatingCompanionComponent::StaticClass())) : nullptr;
	
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	
	CachedCameraEffectComponent = CachedPlayerController->GetComponentByClass(UCameraEffectsComponent::StaticClass()) ? Cast<UCameraEffectsComponent>(CachedPlayerController->GetComponentByClass(UCameraEffectsComponent::StaticClass())) : nullptr;
	
	if (SoundWaveData && SoundWaveData->ChargeEffectClass)
	{
		ApplyEffectWithSetByCaller(SoundWaveData->ChargeEffectClass, TAG_Data_SoundWaveChargeDuration, SoundWaveData->ChargeSound->GetDuration());
		
		if (AbilitySystemComponent)
			AbilitySystemComponent->ExecuteGameplayCue(TAG_GameplayCue_Companion_SoundWaveCharge);
	}
	
	AvailableTargets.Empty();
	CurrentTargetIndex = 0;
	LastSwitchTime = 0.0f;
	
	if (CachedCameraEffectComponent)
		CachedCameraEffectComponent->PlayEffectPreset(SoundWaveData->ChargeEffectPreset);

	if (CachedCompanionComponent)
		CachedCompanionComponent->SetCompanionState(ECompanionState::Angry);
	
	SearchForTargets();
	
	// Update targets every 0.1 seconds to keep track of movement and camera rotation
	GetWorld()->GetTimerManager().SetTimer(UpdateTargetsTimerHandle, this, &UGA_SoundWave::UpdateTargets, 0.1f, true);
	
	WaitSwitchEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
	   this,
	   TAG_Event_Companion_SwitchSoundWaveTarget,
	   nullptr,
	   false,
	   true
	);

	WaitSwitchEvent->EventReceived.AddDynamic(this, &UGA_SoundWave::OnSwitchTargetEventReceived);
	WaitSwitchEvent->ReadyForActivation();
	
	WaitShootEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
	   this,
	   TAG_Event_Companion_ShootSoundWave,
	   nullptr,
	   false,
	   true
	);

	WaitShootEvent->EventReceived.AddDynamic(this, &UGA_SoundWave::OnShootEventReceived);
	WaitShootEvent->ReadyForActivation();
	
	ApplySpeedMultiplierToSelf(SoundWaveData ? SoundWaveData->SpeedMultiplier : 1.0f);

	// Play continuous (looping) light force feedback vibration while aiming
	if (SoundWaveData && SoundWaveData->ActiveAimForceFeedback && CachedPlayerController)
	{
		FForceFeedbackParameters Params;
		Params.bLooping = true;
		Params.bIgnoreTimeDilation = true;
		Params.Tag = FName("AimActive");
		CachedPlayerController->ClientPlayForceFeedback(SoundWaveData->ActiveAimForceFeedback, Params);
	}
}

void UGA_SoundWave::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	RemoveChargeAndAimEffects();
	
	if (AvailableTargets.IsValidIndex(CurrentTargetIndex))
	{
		ITargetable* TargetInterface = Cast<ITargetable>(AvailableTargets[CurrentTargetIndex]);
		if (TargetInterface)
			TargetInterface->SetUntargeted();
	}
	
	AvailableTargets.Empty();
	
	GetWorld()->GetTimerManager().ClearTimer(UpdateTargetsTimerHandle);
	
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (AbilitySystemComponent && CurrentSpeedEffectHandle.IsValid())
	{
		AbilitySystemComponent->RemoveActiveGameplayEffect(CurrentSpeedEffectHandle);
		CurrentSpeedEffectHandle.Invalidate();
	}
	
	if (CachedCompanionComponent)
		CachedCompanionComponent->SetCompanionState(ECompanionState::Happy);

	// Stop the continuous looping aiming force feedback vibration
	if (SoundWaveData && SoundWaveData->ActiveAimForceFeedback && CachedPlayerController)
	{
		CachedPlayerController->ClientStopForceFeedback(SoundWaveData->ActiveAimForceFeedback, FName("AimActive"));
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_SoundWave::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
	if (CachedCameraEffectComponent)
		CachedCameraEffectComponent->StopAllEffects();
	
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}

void UGA_SoundWave::ApplySpeedMultiplierToSelf(const float Multiplier)
{
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	
	// Safely remove the exact previous effect using its unique handle
	if (CurrentSpeedEffectHandle.IsValid())
	{
		AbilitySystemComponent->RemoveActiveGameplayEffect(CurrentSpeedEffectHandle);
		CurrentSpeedEffectHandle.Invalidate();
	}
	
	const FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(
		SoundWaveData->SpeedEffectClass,
		1.0f,
		AbilitySystemComponent->MakeEffectContext()
	);

	if (!SpecHandle.IsValid())
		return;

	SpecHandle.Data->SetSetByCallerMagnitude(
		FGameplayTag::RequestGameplayTag(FName("Data.SpeedMultiplier")),
		Multiplier
	);

	// Apply the effect to change the values in the character class
	CurrentSpeedEffectHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
}

void UGA_SoundWave::ApplyEffectWithSetByCaller(const TSubclassOf<UGameplayEffect> EffectClass, const FGameplayTag& DataTag, const float Magnitude) const
{
	if (!EffectClass) 
		return;

	// Create the spec handle for the outgoing effect
	const FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(EffectClass, GetAbilityLevel());
    
	// Inject the duration/cooldown value using the provided SetByCaller tag
	if (SpecHandle.IsValid())
	{
		SpecHandle.Data.Get()->SetSetByCallerMagnitude(DataTag, Magnitude);
		(void)ApplyGameplayEffectSpecToOwner(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, SpecHandle);
	}
}

void UGA_SoundWave::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	const UGameplayEffect* CooldownGE = GetCooldownGameplayEffect();
    
	if (!CooldownGE || !SoundWaveData) 
		return;

	// Apply the cooldown effect with the SetByCaller magnitude
	ApplyEffectWithSetByCaller(CooldownGE->GetClass(), TAG_Data_SoundWaveCooldown, SoundWaveData->AbilityCooldown);
}

void UGA_SoundWave::SearchForTargets()
{
	if (CachedCameraEffectComponent)
		CachedCameraEffectComponent->PlayEffectPreset(SoundWaveData->AimEffectPreset);
    
	AvailableTargets.Empty();
	CurrentTargetIndex = 0;
    
	AvailableTargets = GetValidTargetsOnScreen();
    
	if (AvailableTargets.IsEmpty())
		return;
       
	const AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (AvatarActor)
	{
		AvailableTargets.Sort([AvatarActor](const AActor& A, const AActor& B)
		{
		   const float DistA = FVector::DistSquared(AvatarActor->GetActorLocation(), A.GetActorLocation());
		   const float DistB = FVector::DistSquared(AvatarActor->GetActorLocation(), B.GetActorLocation());
		   return DistA < DistB;
		});
	}
    
	if (AvailableTargets.IsValidIndex(0))
	{
		ITargetable* TargetableInterface = Cast<ITargetable>(AvailableTargets[0]);
		if (TargetableInterface)
			TargetableInterface->SetTargeted();
	}
}

TArray<AActor*> UGA_SoundWave::GetValidTargetsOnScreen() const
{
	TArray<AActor*> ValidTargets;
    
	const AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor || !SoundWaveData || !CachedPlayerController)
		return ValidTargets;

	int32 ViewportSizeX, ViewportSizeY;
	CachedPlayerController->GetViewportSize(ViewportSizeX, ViewportSizeY);

	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(AvatarActor);

	const bool bHasOverlap = GetWorld()->OverlapMultiByChannel(
	   OverlapResults,
	   AvatarActor->GetActorLocation(),
	   FQuat::Identity,
	   ECC_Visibility,
	   FCollisionShape::MakeSphere(SoundWaveData->TargetingRadius),
	   QueryParams
	);

	if (!bHasOverlap)
		return ValidTargets;

	for (const FOverlapResult& Result : OverlapResults)
	{
		AActor* OverlapActor = Result.GetActor();
		if (OverlapActor && OverlapActor->Implements<UTargetable>())
		{
			const ITargetable* TargetableInterface = Cast<ITargetable>(OverlapActor);
			if (TargetableInterface && TargetableInterface->IsTargetable())
			{
				FVector2D ScreenPosition;
				if (GetScreenPosition(ScreenPosition, OverlapActor->GetActorLocation()))
				{
					const bool bIsOnScreen = (ScreenPosition.X >= 0.0f && ScreenPosition.X <= ViewportSizeX && ScreenPosition.Y >= 0.0f && ScreenPosition.Y <= ViewportSizeY);
					if (bIsOnScreen)
						ValidTargets.AddUnique(OverlapActor);
				}
			}
		}
	}

	return ValidTargets;
}

bool UGA_SoundWave::GetScreenPosition(FVector2D& OutScreenPosition, const FVector& WorldLocation) const
{
	if (!CachedPlayerController)
		return false;
	
	return UGameplayStatics::ProjectWorldToScreen(CachedPlayerController, WorldLocation,OutScreenPosition);
}

void UGA_SoundWave::RemoveChargeAndAimEffects() const
{
	if (!SoundWaveData)
		return;
	
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (AbilitySystemComponent)
	{
		if (SoundWaveData->ChargeEffectClass)
			AbilitySystemComponent->RemoveActiveGameplayEffectBySourceEffect(SoundWaveData->ChargeEffectClass, AbilitySystemComponent);
		
		if (SoundWaveData->AimEffectClass)
			AbilitySystemComponent->RemoveActiveGameplayEffectBySourceEffect(SoundWaveData->AimEffectClass, AbilitySystemComponent);
	}
}
#pragma endregion

#pragma region UFUNCTIONS
void UGA_SoundWave::OnSwitchTargetEventReceived(FGameplayEventData Payload)
{
	// Ensure the event contains valid location data
	const FGameplayAbilityTargetData_LocationInfo* LocationData = static_cast<const FGameplayAbilityTargetData_LocationInfo*>(Payload.TargetData.Get(0));
	if (!LocationData)
		return;

	const FVector Raw = LocationData->TargetLocation.LiteralTransform.GetLocation();
	const FVector2D InputDir2D = FVector2D(Raw.X, Raw.Y).GetSafeNormal();
	if (InputDir2D.IsNearlyZero())
		return;

	// Remove any invalid targets from the list and adjust the current target index accordingly
	for (int32 i = AvailableTargets.Num() - 1; i >= 0; i--)
	{
		// If a target is no longer valid, remove it from the list and adjust the current target index if necessary
		if (!IsValid(AvailableTargets[i]))
		{
			AvailableTargets.RemoveAt(i);
			
			if (i < CurrentTargetIndex)
				CurrentTargetIndex--;
			else if (i == CurrentTargetIndex)
				CurrentTargetIndex = FMath::Clamp(CurrentTargetIndex, 0, AvailableTargets.Num() - 1);
		}
	}
	if (AvailableTargets.IsEmpty())
		return;
	
	// Check if enough time has passed since the last target switch to prevent rapid switching
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastSwitchTime < SoundWaveData->TargetSwitchCooldown)
		return;

	// Project all targets onto the screen and pick the one most aligned with InputDir2D.
	// Using screen space ensures the selection feels correct regardless of camera angle or position
	FVector2D CurrentScreenPos;
	GetScreenPosition(CurrentScreenPos, AvailableTargets[CurrentTargetIndex]->GetActorLocation());
	
	int32 BestIndex = -1;
	float BestScore = SoundWaveData->TargetSwitchThreshold;
	
	for (int32 i = 0 ; i < AvailableTargets.Num(); i++) 
	{
		if (i == CurrentTargetIndex)
			continue; // Skip the current target 
		
		FVector2D CandidateScreenPos;
		if (!GetScreenPosition(CandidateScreenPos, AvailableTargets[i]->GetActorLocation()))
			continue; // Skip targets that can't be projected onto the screen

		// Dot product between input direction and screen-space direction to candidate
		const float Score = FVector2D::DotProduct(
			InputDir2D,
			(CandidateScreenPos - CurrentScreenPos).GetSafeNormal());
		
		if (Score > BestScore)
		{
			BestScore = Score;
			BestIndex = i;
		}
	}
	
	// If a valid target index was found, switch to that target
	if (BestIndex == -1)
		return;
	
	if (ITargetable* OldTarget = Cast<ITargetable>(AvailableTargets[CurrentTargetIndex]))
		OldTarget->SetUntargeted();

	CurrentTargetIndex = BestIndex;
	LastSwitchTime = GetWorld()->GetTimeSeconds();

	if (ITargetable* NewTarget = Cast<ITargetable>(AvailableTargets[CurrentTargetIndex]))
		NewTarget->SetTargeted();
}

void UGA_SoundWave::OnShootEventReceived(FGameplayEventData Payload)
{
	if (!IsActive())
		return;
	
	if (!SoundWaveData || !SoundWaveData->ProjectileClass)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}
	
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	const AActor* TargetActor = AvailableTargets.IsValidIndex(CurrentTargetIndex) ? AvailableTargets[CurrentTargetIndex] : nullptr;
	const bool bHasTarget = IsValid(TargetActor);
	
	RemoveChargeAndAimEffects(); // Remove charge and aim effects before shooting

	// Play strong, sharp force feedback vibration when the shot is fired
	if (SoundWaveData && SoundWaveData->ShootForceFeedback && CachedPlayerController)
	{
		FForceFeedbackParameters Params;
		Params.bLooping = false;
		Params.bIgnoreTimeDilation = true;
		CachedPlayerController->ClientPlayForceFeedback(SoundWaveData->ShootForceFeedback, Params);
	}
	
	if (CachedCameraEffectComponent)
	{
		CachedCameraEffectComponent->StopAllEffects();
		CachedCameraEffectComponent->PlayEffectPreset(SoundWaveData->ShootEffectPreset);
	}
	
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (AbilitySystemComponent)
		AbilitySystemComponent->ExecuteGameplayCue(TAG_GameplayCue_Companion_SoundWaveShoot);
	
	// Determine spawn origin (companion position if available, otherwise avatar)
	FVector StartLocation = AvatarActor->GetActorLocation();
	if (CachedCompanionComponent)
		StartLocation = CachedCompanionComponent->GetCurrentLocation();

	FVector TargetLocation;
	if (bHasTarget)
	{
		const UTargetSpot* TargetSpot = TargetActor->GetComponentByClass<UTargetSpot>();
		TargetLocation = TargetSpot ? TargetSpot->GetComponentLocation() : TargetActor->GetActorLocation();
	}
	else
	{
		TargetLocation = StartLocation + AvatarActor->GetActorForwardVector() * SoundWaveData->MaxHomingDistance;
	}
	
	// Calculate the direction vector from the companion to the target
	const FVector DirectionToTarget = (TargetLocation - StartLocation).GetSafeNormal();
	const FVector SpawnLocation = StartLocation + (DirectionToTarget * SoundWaveData->OffsetProjectileSpawn);
	
	FRotator ShootRotation = DirectionToTarget.Rotation();
	
	if (bHasTarget && SoundWaveData->bEnableHoming)
	{
		const float DistanceToTarget = FVector::Dist(StartLocation, TargetLocation);
       
		// Define the input range (Distance) and the output range (Pitch)
		const FVector2D DistanceRange(SoundWaveData->MinHomingDistance, SoundWaveData->MaxHomingDistance);
		const FVector2D PitchRange(0.0f, SoundWaveData->MaxHomingPitchOffset);
       
		// Map the current distance to the correct pitch value, clamped within our thresholds
		const float CalculatedPitch = FMath::GetMappedRangeValueClamped(DistanceRange, PitchRange, DistanceToTarget);
       
		ShootRotation.Pitch += CalculatedPitch;
	}
	
	const FTransform SpawnTransform(ShootRotation, SpawnLocation);
    
	AProjectile* SpawnedProjectile = GetWorld()->SpawnActorDeferred<AProjectile>(
		SoundWaveData->ProjectileClass,
		SpawnTransform,
		GetOwningActorFromActorInfo(),
		Cast<APawn>(AvatarActor),
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn
	);
    
	if (SpawnedProjectile)
	{
		if (bHasTarget && SoundWaveData->bEnableHoming)
		{
			UTargetSpot* TargetSpot = TargetActor->FindComponentByClass<UTargetSpot>();
			USceneComponent* HomingTargetComponent = TargetSpot ? Cast<USceneComponent>(TargetSpot) : TargetActor->GetRootComponent();
			if (HomingTargetComponent)
				SpawnedProjectile->SetHomingTarget(HomingTargetComponent, SoundWaveData->HomingAcceleration);
		}
       
		SpawnedProjectile->NoiseLoudness = SoundWaveData->ShootNoiseLoudness;
		SpawnedProjectile->NoiseRange = SoundWaveData->ShootNoiseRange;
		SpawnedProjectile->OriginLocation = SpawnLocation;
       
		SpawnedProjectile->FinishSpawning(SpawnTransform);
	}
	
	CommitAbilityCooldown(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false); // Apply cooldown
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_SoundWave::UpdateTargets()
{
	const TArray<AActor*> NewlyFoundTargets = GetValidTargetsOnScreen();

	const AActor* CurrentTarget = AvailableTargets.IsValidIndex(CurrentTargetIndex) ? AvailableTargets[CurrentTargetIndex] : nullptr;
	bool bCurrentTargetLost = false;

	// Remove targets that are no longer valid or went off-screen
	for (int32 i = AvailableTargets.Num() - 1; i >= 0; i--)
	{
		if (!NewlyFoundTargets.Contains(AvailableTargets[i]))
		{
			if (i == CurrentTargetIndex)
			{
				bCurrentTargetLost = true;
				ITargetable* OldTarget = Cast<ITargetable>(AvailableTargets[i]);
				if (OldTarget)
					OldTarget->SetUntargeted();
			}

			AvailableTargets.RemoveAt(i);

			if (i < CurrentTargetIndex)
				CurrentTargetIndex--;
		}
	}

	// Add newly entered targets
	for (AActor* NewActor : NewlyFoundTargets)
	{
		if (!AvailableTargets.Contains(NewActor))
			AvailableTargets.Add(NewActor);
	}

	if (AvailableTargets.IsEmpty())
	{
		CurrentTargetIndex = 0;
		return;
	}

	// If we lost the current target, pick the closest one from the updated list
	if (bCurrentTargetLost || CurrentTarget == nullptr)
	{
		const AActor* AvatarActor = GetAvatarActorFromActorInfo();
		if (AvatarActor)
		{
			AvailableTargets.Sort([AvatarActor](const AActor& A, const AActor& B)
			{
			   const float DistA = FVector::DistSquared(AvatarActor->GetActorLocation(), A.GetActorLocation());
			   const float DistB = FVector::DistSquared(AvatarActor->GetActorLocation(), B.GetActorLocation());
			   return DistA < DistB;
			});
		}

		CurrentTargetIndex = 0;
		ITargetable* NewTarget = Cast<ITargetable>(AvailableTargets[CurrentTargetIndex]);
		if (NewTarget)
			NewTarget->SetTargeted();
	}
}
#pragma endregion
