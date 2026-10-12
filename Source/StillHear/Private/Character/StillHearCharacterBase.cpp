#include "Character/StillHearCharacterBase.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayAbilitySystem/Tags/GameplayTags.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayAbilitySystem/Attributes/BasicAttributeSet.h"
#include "GameplayAbilitySystem/Component/StillHearAbilitySystemComponent.h"

// Sets default values
AStillHearCharacterBase::AStillHearCharacterBase()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Add the Ability System Component
	AbilitySystemComponent = CreateDefaultSubobject<UStillHearAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	
	GetCharacterMovement()->bOrientRotationToMovement = true;
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
}

UAbilitySystemComponent* AStillHearCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

TArray<FGameplayAbilitySpecHandle> AStillHearCharacterBase::GrantAbilities(TArray<TSubclassOf<UGameplayAbility>> AbilitiesToGrant)
{
	if (!AbilitySystemComponent)
		return TArray<FGameplayAbilitySpecHandle>();

	TArray<FGameplayAbilitySpecHandle> AbilityHandles;
	
	for (const TSubclassOf Ability : AbilitiesToGrant)
	{
		FGameplayAbilitySpecHandle SpecHandle = AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(Ability, 1, -1, this));
		AbilityHandles.Add(SpecHandle);
	}

	SendAbilitiesChangedEvent();
	return AbilityHandles;
}

void AStillHearCharacterBase::GrantMissingStartingAbilities()
{
	if (!AbilitySystemComponent)
		return;

	// Build the subset of StartingAbilities that the ASC does NOT already have a spec for.
	// FindAbilitySpecFromClass returns nullptr when no granted spec of that class exists.
	TArray<TSubclassOf<UGameplayAbility>> MissingAbilities;
	for (const TSubclassOf<UGameplayAbility>& AbilityClass : StartingAbilities)
	{
		if (!AbilityClass)
			continue;

		if (AbilitySystemComponent->FindAbilitySpecFromClass(AbilityClass) == nullptr)
		{
			MissingAbilities.Add(AbilityClass);
		}
	}

	// Only touch the ASC (and fire the abilities-changed event) when there is actually
	// something to grant, so repeated possessions with intact abilities are a no-op.
	if (MissingAbilities.Num() > 0)
	{
		GrantAbilities(MissingAbilities);
	}
}

void AStillHearCharacterBase::RemoveAbilities(TArray<FGameplayAbilitySpecHandle> AbilityHandlesToRemove)
{
	if (!AbilitySystemComponent)
		return;

	for (FGameplayAbilitySpecHandle AbilityHandle : AbilityHandlesToRemove)
	{
		AbilitySystemComponent->ClearAbility(AbilityHandle);
	}

	SendAbilitiesChangedEvent();
}

void AStillHearCharacterBase::SendAbilitiesChangedEvent()
{
	FGameplayEventData EventData;
	EventData.EventTag = FGameplayTag::RequestGameplayTag(FName("Event.Abilities.Changed"));
	EventData.Instigator = this;
	EventData.Target = this;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, EventData.EventTag, EventData);
}

// Called when the game starts or when spawned
void AStillHearCharacterBase::BeginPlay()
{
	Super::BeginPlay();
}

void AStillHearCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (AbilitySystemComponent)
	{
		// Refresh the ability actor info against the (possibly rebuilt) avatar/owner.
		AbilitySystemComponent->InitAbilityActorInfo(this, this);

		// Idempotently (re)grant the StartingAbilities. A streaming visibility toggle destroys and
		// respawns the AIController, and that teardown clears the granted ability specs from the ASC.
		// The previous one-shot guard (bStartingAbilitiesGranted) then blocked re-granting, leaving the
		// pawn with zero abilities forever. Granting only the missing ones restores them on re-possess
		// without ever creating duplicates.
		GrantMissingStartingAbilities();
	}
}

void AStillHearCharacterBase::SendGameplayEventToSelf(const FGameplayTag EventTag)
{
	FGameplayEventData EventData;
	EventData.EventTag = EventTag;
	EventData.Instigator = this;
	EventData.Target = this;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		this,
		EventData.EventTag,
		EventData
	);
}

// Called every frame
void AStillHearCharacterBase::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AStillHearCharacterBase::OnMovementModeChanged(const EMovementMode PrevMovementMode, const uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	if (!AbilitySystemComponent)
		return;

	if (GetCharacterMovement()->MovementMode == MOVE_Falling)
	{
		AbilitySystemComponent->AddLooseGameplayTag(TAG_Status_Falling);
	}
	else if (PrevMovementMode == MOVE_Falling)
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(TAG_Status_Falling);
	}
}

void AStillHearCharacterBase::OnEndCrouch(const float HalfHeightAdjust, const float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	// Notify GAS that the character has physically uncrouched
	SendGameplayEventToSelf(TAG_Event_MainCharacter_EndCrouch);
}

