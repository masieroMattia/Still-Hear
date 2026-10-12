#pragma once

#include "CoreMinimal.h"
#include "CheckpointBase.h"
#include "Components/BoxComponent.h"
#include "CheckpointTrigger.generated.h"

UCLASS()
class STILLHEAR_API ACheckpointTrigger : public ACheckpointBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ACheckpointTrigger();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> TriggerBox;

	// Stores the character's location when entering the trigger, used to detect traversal direction on exit
	FVector EntryLocation;

	/** If true, the trigger will disable itself after being traversed. */
	UPROPERTY(EditAnywhere, Category = "Checkpoint")
	bool bTriggerOnce = false;
	
	UFUNCTION()
	void OnTriggerBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnTriggerEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);
	
};