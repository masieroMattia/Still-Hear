#include "Flow/CheckpointTrigger.h"

#include "Character/StillHearMainCharacter.h"

// Sets default values
ACheckpointTrigger::ACheckpointTrigger()
{
	PrimaryActorTick.bCanEverTick = false;
	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(RootComponent);
	TriggerBox->SetGenerateOverlapEvents(true);
}

// Called when the game starts or when spawned
void ACheckpointTrigger::BeginPlay()
{
	Super::BeginPlay();
	
	TriggerBox->OnComponentBeginOverlap.AddUniqueDynamic(this, &ACheckpointTrigger::OnTriggerBeginOverlap);
	TriggerBox->OnComponentEndOverlap.AddUniqueDynamic(this, &ACheckpointTrigger::OnTriggerEndOverlap);
}

// Called every frame
void ACheckpointTrigger::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACheckpointTrigger::OnTriggerBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (Cast<AStillHearMainCharacter>(OtherActor))
	{
		EntryLocation = OtherActor->GetActorLocation();
		OnCheckpointActivated();
	}
}

void ACheckpointTrigger::OnTriggerEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (!Cast<AStillHearMainCharacter>(OtherActor))
	{
		return;
	}

	// Determine direction: compare how the character moved through the trigger.
	// The forward direction is from the entry side toward the PlayerNewStartPoint arrow.
	// If the character exited on the same side it entered (went in and came back out),
	// it means it reversed direction.
	const FVector ExitLocation = OtherActor->GetActorLocation();
	const FVector TriggerCenter = GetActorLocation();

	// The forward direction of this checkpoint (using the trigger box rotation)
	const FVector ForwardDir = TriggerBox->GetForwardVector();

	// Entry was "behind" the trigger if the dot product is negative
	const float EntryDot = FVector::DotProduct(EntryLocation - TriggerCenter, ForwardDir);
	const float ExitDot  = FVector::DotProduct(ExitLocation - TriggerCenter, ForwardDir);

	// The player went FORWARD if they entered from behind and exited in front (or were already in front)
	// The player went BACKWARD if they entered from the front and exited behind
	const bool bExitedBackward = (ExitDot < EntryDot);

	if (bExitedBackward)
	{
		OnCheckpointReversed();
	}
	
	if (bTriggerOnce)
	{
		TriggerBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}