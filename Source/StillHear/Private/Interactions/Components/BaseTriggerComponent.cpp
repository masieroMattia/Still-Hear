#include "Interactions/Components/BaseTriggerComponent.h"

#include "StillHearGameInstance.h"
#include "TraceAndCollision/CustomCollision.h"

#pragma region CONSTRUCTOR
UBaseTriggerComponent::UBaseTriggerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// Set a default collision profile typical for triggers
	UPrimitiveComponent::SetCollisionProfileName(TEXT("Custom"));
	UPrimitiveComponent::SetCollisionResponseToAllChannels(ECR_Ignore);
	UPrimitiveComponent::SetCollisionResponseToChannel(ECustomCollision::Player, ECR_Overlap);
}
#pragma endregion

#pragma region METHODS
void UBaseTriggerComponent::BeginPlay()
{
	Super::BeginPlay();

	// Bind overlap events
	OnComponentBeginOverlap.AddUniqueDynamic(this, &UBaseTriggerComponent::HandleOverlapBegin);
	OnComponentEndOverlap.AddUniqueDynamic(this, &UBaseTriggerComponent::HandleOverlapEnd);

	// Bind to global reset event
	if (const UWorld* World = GetWorld())
	{
		if (UStillHearGameInstance* GI = World->GetGameInstance<UStillHearGameInstance>())
		{
			GI->OnRequestWorldReset.AddUObject(this, &UBaseTriggerComponent::Reset);
			GI->OnCheckpointSnapshot.AddUObject(this, &UBaseTriggerComponent::SaveCheckpointState);
			GI->OnClearCheckpointState.AddUObject(this, &UBaseTriggerComponent::ClearCheckpointState);
		}
	}
}

void UBaseTriggerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (const UWorld* World = GetWorld())
	{
		if (UStillHearGameInstance* GI = World->GetGameInstance<UStillHearGameInstance>())
		{
			GI->OnRequestWorldReset.RemoveAll(this);
			GI->OnCheckpointSnapshot.RemoveAll(this);
			GI->OnClearCheckpointState.RemoveAll(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void UBaseTriggerComponent::HandleOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || !IsCollisionChannelAllowed(OtherComp) || !HasTriggersRemaining())
		return;

	OnTriggerEnter(OtherActor, OtherComp);
}

void UBaseTriggerComponent::HandleOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor || !IsCollisionChannelAllowed(OtherComp) || !HasTriggersRemaining())
		return;

	OnTriggerExit(OtherActor, OtherComp);
}

bool UBaseTriggerComponent::IsCollisionChannelAllowed(const UPrimitiveComponent* TargetComp) const
{
	if (!TargetComp)
		return false;

	// If no specific collision channels are defined, allow everyone
	if (AllowedCollisionChannels.IsEmpty())
		return true;

	// Check if the component's object type matches any of the allowed channels
	return AllowedCollisionChannels.Contains(TargetComp->GetCollisionObjectType());
}

bool UBaseTriggerComponent::HasTriggersRemaining() const
{
	// 0 means unlimited triggers
	if (MaxTriggerCount == 0)
		return true;

	return CurrentTriggerCount < MaxTriggerCount;
}

void UBaseTriggerComponent::IncrementTriggerCount()
{
	++CurrentTriggerCount;
}
#pragma endregion
	
#pragma region INTERFACE METHODS
void UBaseTriggerComponent::Reset()
{
	CurrentTriggerCount = bHasTriggerCheckpointSnapshot ? CheckpointTriggerCount : 0;
}

void UBaseTriggerComponent::SaveCheckpointState()
{
	bHasTriggerCheckpointSnapshot = true;
	CheckpointTriggerCount = CurrentTriggerCount;
}

void UBaseTriggerComponent::ClearCheckpointState()
{
	bHasTriggerCheckpointSnapshot = false;
}

void UBaseTriggerComponent::OnPostLoad_Implementation()
{
	Reset();
}
#pragma endregion