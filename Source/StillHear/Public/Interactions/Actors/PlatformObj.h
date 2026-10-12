#pragma once

#include "CoreMinimal.h"
#include "DrivableObj.h"
#include "PlatformObj.generated.h"

/**
 * Triggers linked interactables when a valid activator
 * (specific actor or any actor on a matching collision channel) overlaps
 * for a configurable duration
 *
 * Two modes:
 * - Persistent: the activator must remain on the plate, leaving undoes the trigger
 * - OneShot: once triggered, the activation is permanent
 */

UENUM(BlueprintType)
enum class EPressurePlateMode : uint8
{
	Persistent,
	OneShot
};

UCLASS()
class STILLHEAR_API APlatformObj : public ADrivableObj
{
	GENERATED_BODY()

#pragma region UPROPERTIES
protected:
	// Specific actors required to activate the plate (leave empty to allow any overlapping actor)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Config")
	TArray<TObjectPtr<AActor>> RequiredActors;

	// Seconds the activator must remain on the plate before triggering (0 = instant)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Config", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float PlateActivationTime = 0.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Config")
	EPressurePlateMode PlateMode = EPressurePlateMode::OneShot;
#pragma endregion

#pragma region VARIABLES
protected:
	// True while a valid activator is sitting on the plate
	bool bPlateOccupied = false;

	// True after the plate has successfully triggered its target
	bool bPlateTriggered = false;

	FTimerHandle PlateActivationTimerHandle;

	// The actor currently occupying the plate
	UPROPERTY()
	TObjectPtr<AActor> CurrentPlateActivator;

	// ── Checkpoint Snapshot ──
	UPROPERTY(SaveGame)
	bool bHasPlatformCheckpointSnapshot = false;
	UPROPERTY(SaveGame)
	bool bCheckpointPlateTriggered = false;
#pragma endregion

#pragma region OVERRIDE METHODS
public:
	virtual void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;
	virtual void Reset() override;
	virtual void SaveCheckpointState() override;
	virtual void ClearCheckpointState() override;
#pragma endregion

#pragma region METHODS
private:
	// Check whether the overlapping actor/component qualifies as a plate activator
	bool IsValidPlateActivator(AActor* Actor, UPrimitiveComponent* OtherComp) const;

	void ActivatePlate();
	void DeactivatePlate();
	void OnPlateActivationTimerExpired();
#pragma endregion
};
