#pragma once

#include "CoreMinimal.h"
#include "SceneManager.h"
#include "GameFramework/Actor.h"
#include "Components/ArrowComponent.h"
#include "CheckpointBase.generated.h"

UCLASS(Abstract)
class STILLHEAR_API ACheckpointBase : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<ASceneManager> SceneManager;
	
	// Sets default values for this actor's properties
	ACheckpointBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	/** Called when the player enters the checkpoint going FORWARD. */
	virtual void OnCheckpointActivated();

	/** Called when the player exits the checkpoint going BACKWARD.
	 *  Reverses the level visibility (shows what was hidden, hides what was shown). */
	virtual void OnCheckpointReversed();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
private:
	UPROPERTY(EditAnywhere, Category="Checkpoint|Levels")
	TArray<FName> LevelsToShow;
	UPROPERTY(EditAnywhere, Category="Checkpoint|Levels")
	TArray<FName> LevelsToHide;
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UArrowComponent> PlayerNewStartPoint;
	UPROPERTY(EditAnywhere, Category="Checkpoint")
	bool bCallSaveOnActivate = false;

	// Priority of this checkpoint. Higher values = further in the game.
	// The game will only save if the player reaches a checkpoint with higher priority than the current one.
	UPROPERTY(EditAnywhere, Category="Checkpoint", meta=(ClampMin="0"))
	int32 CheckpointPriority = 0;

	// Row name in DT_CheckpointDisplay — drives the save slot label and background image
	UPROPERTY(EditAnywhere, Category="Checkpoint")
	FName CheckpointId;

	// True after the checkpoint has been traversed forward (levels changed).
	// Used to know whether an EndOverlap should reverse the visibility.
	bool bTraversedForward = false;
	
};
