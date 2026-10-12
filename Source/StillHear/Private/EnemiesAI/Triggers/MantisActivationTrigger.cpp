#include "EnemiesAI/Triggers/MantisActivationTrigger.h"

#include "StillHearGameInstance.h"
#include "GameFramework/Character.h"
#include "SaveSystem/SaveIdComponent.h"
#include "TraceAndCollision/CustomCollision.h"
#include "UI/Indicator/IndicatorTriggerComponent.h"
#include "EnemiesAI/Pawns/Mantis/AIMantisCharacter.h"

#pragma region CONSTRUCTOR
AMantisActivationTrigger::AMantisActivationTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	SaveIdComponent = CreateDefaultSubobject<USaveIdComponent>(TEXT("SaveIdComponent"));

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	SetRootComponent(TriggerBox);
	TriggerBox->SetGenerateOverlapEvents(true);
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECustomCollision::Player, ECR_Overlap);

	// Indicator trigger component
	IndicatorTriggerComp = CreateDefaultSubobject<UIndicatorTriggerComponent>(TEXT("IndicatorTriggerComp"));
	IndicatorTriggerComp->SetupAttachment(TriggerBox);
	IndicatorTriggerComp->bManagedExternally = true; // hides TargetActors in the Details panel
}
#pragma endregion

#pragma region METHODS
void AMantisActivationTrigger::BeginPlay()
{
	Super::BeginPlay();
	TriggerBox->OnComponentBeginOverlap.AddUniqueDynamic(this, &AMantisActivationTrigger::OnTriggerBeginOverlap);

	// Populate IndicatorTriggerComp from MantisList
	if (IndicatorTriggerComp)
	{
		for (const TObjectPtr<AAIMantisCharacter>& Mantis : MantisList)
		{
			if (IsValid(Mantis))
				IndicatorTriggerComp->TargetActors.AddUnique(Mantis.Get());
		}
	}

	if (const UWorld* World = GetWorld())
	{
		if (UStillHearGameInstance* GI = World->GetGameInstance<UStillHearGameInstance>())
		{
			GI->OnRequestWorldReset.AddUObject(this, &AMantisActivationTrigger::Reset);
			GI->OnCheckpointSnapshot.AddUObject(this, &AMantisActivationTrigger::SaveCheckpointState);
			GI->OnClearCheckpointState.AddUObject(this, &AMantisActivationTrigger::ClearCheckpointState);
		}
	}
}

void AMantisActivationTrigger::EndPlay(const EEndPlayReason::Type EndPlayReason)
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

void AMantisActivationTrigger::Reset()
{
	if (!IsValid(this) || !GetWorld())
		return;

	const bool bFromSnapshot = bHasMantisCheckpointSnapshot;
	const bool bTargetEnabled = bFromSnapshot ? bCheckpointTriggerBoxEnabled : true;

	TriggerBox->SetCollisionEnabled(bTargetEnabled ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
}

void AMantisActivationTrigger::SaveCheckpointState()
{
	bHasMantisCheckpointSnapshot = true;
	bCheckpointTriggerBoxEnabled = (TriggerBox->GetCollisionEnabled() != ECollisionEnabled::NoCollision);
}

void AMantisActivationTrigger::ClearCheckpointState()
{
	bHasMantisCheckpointSnapshot = false;
}

void AMantisActivationTrigger::OnPostLoad_Implementation()
{
	Reset();
}
#pragma endregion

#pragma region UFUNCTIONS
void AMantisActivationTrigger::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!Cast<ACharacter>(OtherActor))
		return;

	for (TObjectPtr Mantis : MantisList)
	{
		if (IsValid(Mantis) && Mantis->IsDormant())
			Mantis->Activate();
	}

	if (bTriggerOnce)
		TriggerBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
#pragma endregion
