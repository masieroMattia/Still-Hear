#pragma once

#include "CoreMinimal.h"
#include "SaveSystem/Savable.h"
#include "Interfaces/Restorable.h"
#include "Components/BoxComponent.h"
#include "BaseTriggerComponent.generated.h"

UCLASS(Abstract, ClassGroup=(Interactions))
class STILLHEAR_API UBaseTriggerComponent : public UBoxComponent, public IRestorable, public ISavable
{
	GENERATED_BODY()

#pragma region CONSTRUCTOR
public:
	UBaseTriggerComponent();
#pragma endregion

#pragma region UPROPERTIES
protected:
	// Collision channels that are allowed to activate this trigger. If empty, all channels are accepted
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	TArray<TEnumAsByte<ECollisionChannel>> AllowedCollisionChannels;

	// Maximum number of times this trigger can fire (0 = unlimited)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config", meta = (ClampMin = 0, UIMin = 0))
	int32 MaxTriggerCount = 0;
#pragma endregion

#pragma region VARIABLES
private:
	// Running counter of how many times the trigger has fired
	int32 CurrentTriggerCount = 0;

	// ── Checkpoint Snapshot ──
	UPROPERTY(SaveGame)
	bool bHasTriggerCheckpointSnapshot = false;
	UPROPERTY(SaveGame)
	int32 CheckpointTriggerCount = 0;
#pragma endregion

#pragma region METHODS
public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	// Called when a valid actor enters the trigger volume
	virtual void OnTriggerEnter(AActor* OtherActor, UPrimitiveComponent* OtherComp) {}

	// Called when a valid actor exits the trigger volume
	virtual void OnTriggerExit(AActor* OtherActor, UPrimitiveComponent* OtherComp) {}

	// Returns true if the overlapping component's collision channel is allowed
	bool IsCollisionChannelAllowed(const UPrimitiveComponent* TargetComp) const;

	// Returns true if the trigger has not yet reached its maximum activation count
	bool HasTriggersRemaining() const;

	// Increments the internal trigger counter — call from subclasses after a successful activation
	void IncrementTriggerCount();
	
#pragma region INTERFACE METHODS
public:
	virtual void Reset() override;
	virtual void SaveCheckpointState() override;
	virtual void ClearCheckpointState() override;
	virtual void OnPostLoad_Implementation() override;
#pragma endregion

private:
	UFUNCTION()
	void HandleOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandleOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
#pragma endregion
};