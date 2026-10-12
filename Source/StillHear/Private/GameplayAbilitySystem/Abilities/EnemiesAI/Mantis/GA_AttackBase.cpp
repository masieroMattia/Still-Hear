#include "GameplayAbilitySystem/Abilities/EnemiesAI/Mantis/GA_AttackBase.h"

#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "Components/ShapeComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnemiesAI/Utility/BlackboardKeyNames.h"
#include "GameplayAbilitySystem/Tags/GameplayTags.h"
#include "EnemiesAI/Pawns/Mantis/AIMantisCharacter.h"
#include "EnemiesAI/Utility/DataAssets/Abilities/AttackBaseData.h"

#pragma region CONSTRUCTOR
UGA_AttackBase::UGA_AttackBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(TAG_GameplayAbility_EnemyAI_Attack);
	SetAssetTags(AssetTags);

	ActivationOwnedTags.AddTag(TAG_GameplayAbility_EnemyAI_Attack_Active);
	
	// Block activation while stunned or on cooldown
	ActivationBlockedTags.AddTag(TAG_Status_EnemyAI_Stunned);
	ActivationBlockedTags.AddTag(TAG_Status_EnemyAI_AttackCooldown);
}
#pragma endregion

#pragma region METHODS
void UGA_AttackBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	CachedMantis = Cast<AAIMantisCharacter>(GetAvatarActorFromActorInfo());
	if (!CachedMantis.IsValid())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}

	// Get the target from the blackboard
	const AAIController* AIController = Cast<AAIController>(CachedMantis->GetController());
	const UBlackboardComponent* Blackboard = AIController ? AIController->GetBlackboardComponent() : nullptr;
	if (Blackboard)
		CurrentTarget = Cast<AActor>(Blackboard->GetValueAsObject(BlackboardKeyNames::KeyNameTargetActor));
	
	if (!CurrentTarget.IsValid())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}

	bHasHitThisSwing = false;
	BindHitBox();
	RegisterStunListener();
}

void UGA_AttackBase::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	UnregisterStunListener();
	DisableHitBox();
	UnbindHitBox();
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_AttackBase::BindHitBox()
{
	if (UShapeComponent* HitBox = FindHitBox())
		HitBox->OnComponentBeginOverlap.AddDynamic(this, &UGA_AttackBase::OnHitBoxOverlap);
}

void UGA_AttackBase::UnbindHitBox()
{
	if (UShapeComponent* HitBox = FindHitBox())
		HitBox->OnComponentBeginOverlap.RemoveDynamic(this, &UGA_AttackBase::OnHitBoxOverlap);
}

void UGA_AttackBase::DisableHitBox() const
{
	if (UShapeComponent* HitBox = FindHitBox())
		HitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void UGA_AttackBase::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	const UGameplayEffect* CooldownGE = GetCooldownGameplayEffect();
	if (!CooldownGE)
		return;

	// Fallback duration if AttackData is missing
	float CooldownDuration = 3.0f;
	if (AttackData)
		CooldownDuration = AttackData->AttackCooldownDuration;

	if (CooldownDuration <= 0.0f)
		return;

	const FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(CooldownGE->GetClass(), GetAbilityLevel());
	if (SpecHandle.IsValid())
	{
		// Inject the custom duration value via SetByCaller
		SpecHandle.Data.Get()->SetSetByCallerMagnitude(TAG_Data_EnemyAI_AttackCooldown, CooldownDuration);
       
		// Apply the effect and ignore the unused return value
		(void)ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
	}
}

UShapeComponent* UGA_AttackBase::FindHitBox() const
{
	if (!CachedMantis.IsValid() || !AttackData)
		return nullptr;
	
	TArray<UActorComponent*> Components = CachedMantis->GetComponentsByTag(UShapeComponent::StaticClass(), AttackData->HitBoxTag.GetTagName());
	if (Components.Num() > 0)
		return Cast<UShapeComponent>(Components[0]);
	
	return nullptr;
}

void UGA_AttackBase::RegisterStunListener()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC)
		ASC->RegisterGameplayTagEvent(TAG_Status_EnemyAI_Stunned, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &UGA_AttackBase::OnStunTagAdded);
}

void UGA_AttackBase::UnregisterStunListener() const
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC)
		ASC->RegisterGameplayTagEvent(TAG_Status_EnemyAI_Stunned, EGameplayTagEventType::NewOrRemoved).RemoveAll(this);
}
#pragma endregion

#pragma region UFUNCTIONS
void UGA_AttackBase::OnHitBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Only process once per swing and ignore self
	if (bHasHitThisSwing || !OtherActor || OtherActor == GetAvatarActorFromActorInfo())
		return;

	// Only hit the current target (player)
	if (OtherActor != CurrentTarget.Get())
		return;
	
	bHasHitThisSwing = true;
	DisableHitBox();

	// Apply attack hit effect to the player
	if (AttackData && AttackData->AttackHitEffectClass)
	{
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
		if (TargetASC)
		{
			const FGameplayEffectSpecHandle HitSpec = TargetASC->MakeOutgoingSpec(AttackData->AttackHitEffectClass, 1.0f, TargetASC->MakeEffectContext());
			if (HitSpec.IsValid())
				TargetASC->ApplyGameplayEffectSpecToSelf(*HitSpec.Data);
		}
	}
}

void UGA_AttackBase::OnStunTagAdded(const FGameplayTag Tag, const int32 NewCount)
{
	// When the Stunned tag is applied (NewCount > 0), cancel the attack ability
	if (NewCount > 0)
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
}
#pragma endregion