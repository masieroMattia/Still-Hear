#pragma once

#include "CoreMinimal.h"
#include "SaveSystem/Savable.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "MantisActivationTrigger.generated.h"

class AAIMantisCharacter;
class UIndicatorTriggerComponent;

UCLASS()
class STILLHEAR_API AMantisActivationTrigger : public AActor, public ISavable
{
	GENERATED_BODY()

#pragma region UPROPERTIES
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class USaveIdComponent> SaveIdComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> TriggerBox;

	// Trigger component to activate / deactivate IndicatorComponents on any target actor
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UIndicatorTriggerComponent> IndicatorTriggerComp;

	// Dormant Mantis enemies to activate when the player enters this trigger
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Activation")
	TArray<TObjectPtr<AAIMantisCharacter>> MantisList;

	// If true, the trigger disables itself after the first activation
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Activation")
	bool bTriggerOnce = true;
#pragma endregion

#pragma region VARIABLES
	// Checkpoint Snapshot
	UPROPERTY(SaveGame)
	bool bHasMantisCheckpointSnapshot = false;
	UPROPERTY(SaveGame)
	bool bCheckpointTriggerBoxEnabled = true;
#pragma endregion
	
#pragma region CONSTRUCTOR
public:
	AMantisActivationTrigger();
#pragma endregion

#pragma region UFUNCTIONS
private:
#pragma endregion

#pragma region METHODS
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void Reset();
	
	void SaveCheckpointState();
	void ClearCheckpointState();

	virtual void OnPostLoad_Implementation() override;
	
	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
#pragma endregion
};
