#include "GameplayAbilitySystem/Abilities/GA_Parry.h"

#include "AbilitySystemComponent.h"
#include "Data/DataAssets/ParryData.h"
#include "Components/ShapeComponent.h"
#include "Components/SphereComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Character/StillHearMainCharacter.h"
#include "Subsystems/TimeManagementSubsystem.h"
#include "Character/StillHearPlayerController.h"
#include "TraceAndCollision/CustomCollision.h"
#include "GameplayAbilitySystem/Tags/GameplayTags.h"
#include "Camera/CameraEffects/CameraEffectsComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEffectRemoved.h"
#include "GameFramework/ForceFeedbackEffect.h"

#pragma region CONSTRUCTOR
UGA_Parry::UGA_Parry()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	// "Name of the ability" TAG
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(TAG_GameplayAbility_MainCharacter_Parry);
	SetAssetTags(AssetTags);
	
	// When the ability is activated, these tags are added to the owner
	ActivationOwnedTags.AddTag(TAG_GameplayAbility_MainCharacter_Parry_Active);
	ActivationOwnedTags.AddTag(TAG_Status_MainCharacter_ForceMoving);
}
#pragma endregion
	
#pragma region METHODS
void UGA_Parry::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!CheckCooldown(Handle, ActorInfo) || !ParryData)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// Play light force feedback vibration on button press
	if (ParryData->ParryPressedForceFeedback)
	{
		if (APlayerController* PC = ActorInfo->PlayerController.Get())
		{
			FForceFeedbackParameters Params;
			Params.bLooping = false;
			Params.bIgnoreTimeDilation = true;
			PC->ClientPlayForceFeedback(ParryData->ParryPressedForceFeedback, Params);
		}
	}

	bHasTriggeredSuccessfulParry = false;
	
	// Play Montage
	if (ParryData->ParryMontage)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			ParryData->ParryMontage,
			1.0f
		);
		MontageTask->ReadyForActivation();
	}

	if (const AStillHearMainCharacter* Character = Cast<AStillHearMainCharacter>(GetAvatarActorFromActorInfo()))
		ParrySphereComponent = Character->GetParrySphere();

	// Wait for the AnimNotify event to start the parry window
	UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		TAG_Event_Collision_Activate,
		nullptr,
		false,
		false
	);
	WaitEventTask->EventReceived.AddDynamic(this, &UGA_Parry::OnCollisionEventReceived);
	WaitEventTask->ReadyForActivation();
}

void UGA_Parry::OnCollisionEventReceived(FGameplayEventData Payload)
{
	// Setup Sphere Component
	if (ParrySphereComponent)
	{
		// Set Radius
		if (USphereComponent* Sphere = Cast<USphereComponent>(ParrySphereComponent))
			Sphere->SetSphereRadius(ParryData->ParryRadius);

		// Configure Collision Responses
		ParrySphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
		for (const ECollisionChannel Channel : ParryData->DetectionChannels)
		{
			ParrySphereComponent->SetCollisionResponseToChannel(Channel, ECR_Overlap);
		}

		// Enable Collision
		ParrySphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

		// Check for components already inside the sphere
		TArray<UPrimitiveComponent*> OverlappingComponents;
		ParrySphereComponent->GetOverlappingComponents(OverlappingComponents);
		for (UPrimitiveComponent* OtherComp : OverlappingComponents)
		{
			if (OtherComp && OtherComp->GetOwner() != GetAvatarActorFromActorInfo())
			{
				OnParrySphereOverlap(ParrySphereComponent, OtherComp->GetOwner(), OtherComp, 0, false, FHitResult());
			}
		}

		// Bind Overlap Event for actors entering later
		ParrySphereComponent->OnComponentBeginOverlap.AddDynamic(this, &UGA_Parry::OnParrySphereOverlap);

		// Add GameplayCue for the parry window VFX
		FGameplayCueParameters CueParams;
		CueParams.Instigator = GetAvatarActorFromActorInfo();
		CueParams.Location = ParrySphereComponent->GetComponentLocation();

		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			ASC->AddGameplayCue(TAG_GameplayCue_MainCharacter_Parry, CueParams);
		}
		
		// Start Parry Window Timer
		if (const UWorld* World = GetWorld())
			World->GetTimerManager().SetTimer(ParryWindowTimer, this, &UGA_Parry::OnParryWindowFinished, ParryData->ParryWindowDuration, false);
	}
}

void UGA_Parry::OnParrySphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	FVector ImpactPoint;
	if (bFromSweep)
	{
		ImpactPoint = SweepResult.ImpactPoint;
	}
	else if (OtherComp)
	{
		FVector OutPoint;
		OtherComp->GetClosestPointOnCollision(OverlappedComponent->GetComponentLocation(), OutPoint);
		ImpactPoint = OutPoint;
	}
	else
	{
		ImpactPoint = OtherActor->GetActorLocation();
	}

	ApplyGameplayEffectToTarget(OtherActor, ImpactPoint);
}

void UGA_Parry::ApplyGameplayEffectToTarget(AActor* Target, const FVector& ImpactLocation)
{
	if (!Target || !ParryData) 
	return;

	UAbilitySystemComponent* EnemyASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
	if (!EnemyASC || !EnemyASC->HasAnyMatchingGameplayTags(ParryData->RequiredEnemyTags))
		return;

	bool bEffectApplied = false;
	if (ParryData->StunEffectClass)
	{
		const FGameplayEffectSpecHandle StunSpec = EnemyASC->MakeOutgoingSpec(ParryData->StunEffectClass, 1.0f, EnemyASC->MakeEffectContext());
		if (StunSpec.IsValid())
		{
			if (EnemyASC->ApplyGameplayEffectSpecToSelf(*StunSpec.Data).WasSuccessfullyApplied())
				bEffectApplied = true;
		}
	}

	if (bEffectApplied)
	{
		TriggerParrySensoryEffects();

		// Spawn Success Burst Cue at the exact impact point
		FGameplayCueParameters CueParams;
		CueParams.Location = ImpactLocation;
		CueParams.Normal = (GetAvatarActorFromActorInfo()->GetActorLocation() - ImpactLocation).GetSafeNormal();

		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			ASC->ExecuteGameplayCue(TAG_GameplayCue_MainCharacter_ParryImpact, CueParams);
		}
	}
}

void UGA_Parry::TriggerParrySensoryEffects()
{
	if (bHasTriggeredSuccessfulParry || !ParryData)
		return;

	bHasTriggeredSuccessfulParry = true;

	if (ParryData->SloMoEffectCurve)
	{
		if (UTimeManagementSubsystem* TimeSubsystem = GetWorld()->GetSubsystem<UTimeManagementSubsystem>())
			TimeSubsystem->PlayTimeCurve(ParryData->SloMoEffectCurve);
	}

	if (AStillHearPlayerController* PC = Cast<AStillHearPlayerController>(GetActorInfo().PlayerController.Get()))
	{
		// Play strong force feedback vibration on successful parry
		if (ParryData->ParrySuccessForceFeedback)
		{
			FForceFeedbackParameters Params;
			Params.bLooping = false;
			Params.bIgnoreTimeDilation = true;
			PC->ClientPlayForceFeedback(ParryData->ParrySuccessForceFeedback, Params);
		}

		UCameraEffectsComponent* CameraEffectsComponent = PC->GetCameraEffectsComponent();
		if (CameraEffectsComponent)
			CameraEffectsComponent->PlayEffectPreset(ParryData->EffectPreset);
	}
}

void UGA_Parry::OnParryWindowFinished()
{
	ResetParrySphere();
	
	// Apply Invulnerability
	if (bHasTriggeredSuccessfulParry && ParryData->InvulnerabilityEffectClass)
	{
		const FGameplayEffectSpecHandle InvulnerabilitySpec = MakeOutgoingGameplayEffectSpec(ParryData->InvulnerabilityEffectClass, GetAbilityLevel());
		if (InvulnerabilitySpec.IsValid())
		{
			// Set Duration from DataAsset
			InvulnerabilitySpec.Data.Get()->SetDuration(ParryData->InvulnerabilityDuration, true);
			
			const FActiveGameplayEffectHandle ActiveHandle = ApplyGameplayEffectSpecToOwner(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, InvulnerabilitySpec);
			
			// Wait for removal to commit cooldown and end ability
			UAbilityTask_WaitGameplayEffectRemoved* WaitRemovedTask = UAbilityTask_WaitGameplayEffectRemoved::WaitForGameplayEffectRemoved(this, ActiveHandle);
			WaitRemovedTask->OnRemoved.AddDynamic(this, &UGA_Parry::OnInvulnerabilityEffectRemoved);
			WaitRemovedTask->ReadyForActivation();
			
			return;
		}
	}

	// If no invulnerability was applied, end immediately
	OnInvulnerabilityEffectRemoved(FGameplayEffectRemovalInfo());
}

void UGA_Parry::OnInvulnerabilityEffectRemoved(const FGameplayEffectRemovalInfo& RemovalInfo)
{
	CommitAbilityCooldown(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Parry::ResetParrySphere()
{
	if (ParrySphereComponent)
	{
		ParrySphereComponent->OnComponentBeginOverlap.RemoveDynamic(this, &UGA_Parry::OnParrySphereOverlap);
		ParrySphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			ASC->RemoveGameplayCue(TAG_GameplayCue_MainCharacter_Parry);
		}
	}
}

void UGA_Parry::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	ResetParrySphere();
	
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ParryWindowTimer);

		if (bWasCancelled)
		{
			if (UTimeManagementSubsystem* TimeSubsystem = World->GetSubsystem<UTimeManagementSubsystem>())
				TimeSubsystem->ResetTimeDilation();
		}
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Parry::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	const UGameplayEffect* CooldownGE = GetCooldownGameplayEffect();
    
	if (!CooldownGE || !ParryData) 
	{
		Super::ApplyCooldown(Handle, ActorInfo, ActivationInfo);
		return;
	}

	const FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(CooldownGE->GetClass(), GetAbilityLevel());
	if (SpecHandle.IsValid())
	{
		SpecHandle.Data.Get()->SetSetByCallerMagnitude(TAG_Data_MainCharacter_ParryCooldown, ParryData->CooldownDuration);
		(void)ApplyGameplayEffectSpecToOwner(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, SpecHandle);
	}
}
#pragma endregion
