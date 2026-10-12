#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MantisChaosShiftComponent.generated.h"

class AAIMantisCharacter;
class AChaosResonanceObj;

/**
 * On break of the assigned Chaos object, forces the assigned Mantis into chase
 * and makes it Shift towards an assigned destination point
 */
UCLASS(ClassGroup=(EnemiesAI), meta=(BlueprintSpawnableComponent))
class STILLHEAR_API UMantisChaosShiftComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMantisChaosShiftComponent();

#pragma region UPROPERTIES
protected:
	// Chaos object that triggers this component when broken
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Config|MantisShift")
	TObjectPtr<AChaosResonanceObj> ChaosObject;

	// Mantis to send into chase and Shift
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Config|MantisShift")
	TObjectPtr<AAIMantisCharacter> AssignedMantis;

	// Where the Mantis Shifts to
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|MantisShift")
	TSoftObjectPtr<AActor> DestinationPoint;
#pragma endregion

#pragma region VARIABLES
private:
	// Periodically rebinds to the proxy's currently active clone
	FTimerHandle CloneBindTimerHandle;

	// Object we're currently bound to; clones get replaced at runtime so this can change
	TWeakObjectPtr<AChaosResonanceObj> BoundActiveObj;
#pragma endregion

#pragma region METHODS
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// Resolves the proxy's active clone (it forwards interactions to it) and (re)binds to it
	void TryBindToActiveChaosObj();

	UFUNCTION()
	void HandleChaosObjBroken(AActor* Triggerer);
#pragma endregion
};