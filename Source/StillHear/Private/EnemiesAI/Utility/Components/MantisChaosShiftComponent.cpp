#include "EnemiesAI/Utility/Components/MantisChaosShiftComponent.h"

#include "TimerManager.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Interactions/Actors/ChaosResonanceObj.h"
#include "EnemiesAI/Pawns/Mantis/AIMantisCharacter.h"
#include "EnemiesAI/Controllers/Mantis/AIMantisController.h"

UMantisChaosShiftComponent::UMantisChaosShiftComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMantisChaosShiftComponent::BeginPlay()
{
	Super::BeginPlay();

	TryBindToActiveChaosObj();

	// Clone spawns a tick after BeginPlay and can be replaced later (checkpoint/world reset);
	// each replacement has its own OnTriggeredBy, so keep rechecking and rebind on swaps
	if (UWorld* World = GetWorld())
		World->GetTimerManager().SetTimer(CloneBindTimerHandle, this, &UMantisChaosShiftComponent::TryBindToActiveChaosObj, 0.5f, true);
}

void UMantisChaosShiftComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (const UWorld* World = GetWorld())
		World->GetTimerManager().ClearTimer(CloneBindTimerHandle);

	Super::EndPlay(EndPlayReason);
}

void UMantisChaosShiftComponent::TryBindToActiveChaosObj()
{
	if (!IsValid(ChaosObject))
		return;

	// The proxy forwards interactions (and OnTriggeredBy) to its clone once spawned
	AChaosResonanceObj* ActiveObj = IsValid(ChaosObject->ActiveClone) ? ChaosObject->ActiveClone.Get() : ChaosObject.Get();
	const bool bIsDormantProxy = (ActiveObj == ChaosObject) && !ActiveObj->Tags.Contains(FName("DynamicallySpawned"));
	if (bIsDormantProxy)
		ActiveObj = nullptr; // Clone not spawned yet, retry later

	if (ActiveObj == BoundActiveObj.Get())
		return;

	// Bound object got replaced — unbind before switching to the new one
	if (AChaosResonanceObj* OldObj = BoundActiveObj.Get())
		OldObj->OnTriggeredBy.RemoveDynamic(this, &UMantisChaosShiftComponent::HandleChaosObjBroken);

	BoundActiveObj = ActiveObj;

	if (ActiveObj)
		ActiveObj->OnTriggeredBy.AddUniqueDynamic(this, &UMantisChaosShiftComponent::HandleChaosObjBroken);
}

void UMantisChaosShiftComponent::HandleChaosObjBroken(AActor* Triggerer)
{
	if (!IsValid(AssignedMantis))
		return;

	const AActor* Destination = DestinationPoint.Get();
	if (!Destination)
		Destination = DestinationPoint.LoadSynchronous();

	AAIMantisController* MantisController = Cast<AAIMantisController>(AssignedMantis->GetController());
	if (!MantisController)
		return;

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(AssignedMantis);
	if (!ASC)
		return;

	if (AssignedMantis->IsDormant())
		AssignedMantis->Activate();

	// Trigger the Shift ability towards the assigned destination, landing with the destination's own facing
	FGameplayEventData Payload;
	Payload.EventTag = FGameplayTag::RequestGameplayTag(FName("GameplayAbility.EnemyAI.MantisShift.Nav"));
	Payload.Instigator = Triggerer;
	Payload.Target = AssignedMantis;

	FGameplayAbilityTargetData_LocationInfo* LocationInfo = new FGameplayAbilityTargetData_LocationInfo();
	LocationInfo->TargetLocation.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;
	LocationInfo->TargetLocation.LiteralTransform = Destination->GetActorTransform();
	Payload.TargetData.Add(LocationInfo);

	// Second entry: who the Mantis should fully reset state and start Hunting once it lands (fall back to the player)
	AActor* HuntTarget = IsValid(Triggerer) ? Triggerer : Cast<AActor>(UGameplayStatics::GetPlayerPawn(this, 0));
	FGameplayAbilityTargetData_ActorArray* HuntTargetData = new FGameplayAbilityTargetData_ActorArray();
	HuntTargetData->TargetActorArray.Add(HuntTarget);
	Payload.TargetData.Add(HuntTargetData);

	ASC->HandleGameplayEvent(Payload.EventTag, &Payload);
}
