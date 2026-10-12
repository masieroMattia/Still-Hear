#pragma once

#include "CoreMinimal.h"
#include "InteractableObj.h"
#include "Components/TimelineComponent.h"
#include "DrivableObj.generated.h"

USTRUCT(BlueprintType)
struct FDrivablePart
{
	GENERATED_BODY()

#pragma region UPROPERTIES
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Config")
	FComponentReference MeshComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Config")
	FTransform EndTransform = FTransform::Identity;

	// Cached at runtime
	FTransform StartTransform = FTransform::Identity;
	FTransform DeltaTransform = FTransform::Identity;
#pragma endregion
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractionFinished);

UCLASS()
class STILLHEAR_API ADrivableObj : public AInteractableObj
{
	GENERATED_BODY()
	
#pragma region UPROPERTIES
protected:
	// Empty scene component used as a folder to group all drivable mesh components in the Blueprint hierarchy
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> DrivablePartsRoot;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Config")
	TArray<FDrivablePart> DrivableParts;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Config")
	TObjectPtr<UCurveFloat> TransformCurve;
	// If true, the object can be interacted from the end transform to the start transform, otherwise it keeps add the same transform when interacting multiple times
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Config")
	bool bInteractBackwards = false;
	// If true, the object will stop moving when the interaction finishes, instead of finishing the movement and then stopping
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Config")
	bool bStopDriveOnEnd = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0f, UIMin = 0.0f, EditCondition = "bInteractBackwards", EditConditionHides), Category = "Interactable Settings|Config")
	float BackwardCurveRate;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Config")
	bool bTriggerObj = false;
	// Reference to a triggered object that will be triggered when the interaction finishes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bTriggerObj", EditConditionHides), Category = "Interactable Settings|Config")
	TArray<TObjectPtr<AInteractableObj>> LinkedTriggeredObjs;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Sound")
	TObjectPtr<USoundBase> DriveInteractionSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Sound")
	TObjectPtr<USoundBase> DriveBackInteractionSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Sound")
	TObjectPtr<USoundBase> FinishInteractionSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|VFX")
	TObjectPtr<UNiagaraSystem> DriveInteractionVFX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|VFX")
	TObjectPtr<UNiagaraSystem> DriveBackInteractionVFX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|VFX")
	TObjectPtr<UNiagaraSystem> FinishInteractionVFX;
	
	// Index into DrivableParts whose mesh the InteractNiagaraComponent will be attached to. -1 = stay on actor root.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|VFX", meta = (ClampMin = -1))
	int32 NiagaraVFXPartIndex = -1;
#pragma endregion

#pragma region EVENTS
	UPROPERTY(BlueprintAssignable, Category = "Interactable Settings|Events")
	FOnInteractionFinished OnInteractionFinished;
#pragma endregion
	
#pragma region VARIABLES
private:
	// Resolved at BeginPlay — parallel to DrivableParts
	UPROPERTY(Transient)
	TArray<TObjectPtr<UPrimitiveComponent>> ResolvedMeshes;
	UPROPERTY(Transient)
	TArray<FTransform> OriginalStartTransforms;
	UPROPERTY(Transient)
	TArray<FTransform> OriginalEndTransforms;
	UPROPERTY()
	TObjectPtr<UAudioComponent> SpawnedAudio;
	
	FTransform RootInitialTransform;
	FTimeline TransformTimeLine;
	
	// ── Checkpoint Snapshot ──
	UPROPERTY(SaveGame)
	bool bHasDrivableCheckpointSnapshot = false;
	UPROPERTY(SaveGame)
	bool bCheckpointOn = false;
	UPROPERTY(SaveGame)
	FTransform CheckpointRootTransform;
	UPROPERTY(SaveGame)
	TArray<FTransform> CheckpointPartsTransforms;
	
	bool bOn = false;
protected:
	UPROPERTY()
	TObjectPtr<ACharacter> CurrentInteractor;
#pragma endregion 
	
#pragma region CONSTRUCTOR
public:
	ADrivableObj();
#pragma endregion 

#pragma region METHODS
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
#pragma endregion
	
#pragma region OVERRIDE METHODS
public:
	virtual void ExecuteStartInteraction(TObjectPtr<ACharacter> Interactor = nullptr) override;
	virtual void EndInteraction(TObjectPtr<ACharacter> Interactor = nullptr) override;
	virtual void Reset() override;
	virtual void SaveCheckpointState() override;
	virtual void ClearCheckpointState() override;
#pragma endregion
	
#pragma region UFUNCTIONS
protected:
	UFUNCTION()
	void HandleTimelineProgress(float Value) const;
	UFUNCTION()
	virtual void HandleTimelineFinished();
#pragma endregion
};
