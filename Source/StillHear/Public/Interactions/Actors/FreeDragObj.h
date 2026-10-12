#pragma once

#include "CoreMinimal.h"
#include "InteractableObj.h"
#include "FreeDragObj.generated.h"

UCLASS()
class STILLHEAR_API AFreeDragObj : public AInteractableObj
{
	GENERATED_BODY()
		
#pragma region UPROPERTIES
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interactable Settings|Effect")
	TSubclassOf<class UGameplayEffect> DragEffectClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ToolTip = "Multiplier applied to the character's speed when dragging the object", ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"), Category = "Interactable Settings|Drag")
	float DragMultiplier = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ToolTip = "How far the object can be from the interactor before the grip brokes", ClampMin = "0.0", UIMin = "0.0"), Category = "Interactable Settings|Drag")
	float BreakGripTolerance = 50.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ToolTip = "How fast the object follows the player horizontally", ClampMin = "0.0", UIMin = "0.0"), Category = "Interactable Settings|Drag")
	float FollowInterpSpeed = 5.0f; 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ToolTip = "Minimum horizontal distance to maintain between the object and the player", ClampMin = "0.0", UIMin = "0.0"), Category = "Interactable Settings|Drag")
	float MinDragDistance = 100.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ToolTip = "How high the object floats above its rest position when dragged", ClampMin = "0.0", UIMin = "0.0"), Category = "Interactable Settings|Float")
	float FloatHeight = 80.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ToolTip = "Whether the object bobs up and down while floating"), Category = "Interactable Settings|Float|Bobbing")
	bool bEnableBobbing = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ToolTip = "Amplitude of the up-down bobbing oscillation", ClampMin = "0.0", UIMin = "0.0", EditCondition = "bEnableBobbing", EditConditionHides), Category = "Interactable Settings|Float|Bobbing")
	float BobAmplitude = 10.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ToolTip = "Frequency of the bobbing oscillation (cycles per second)", ClampMin = "0.0", UIMin = "0.0", EditCondition = "bEnableBobbing", EditConditionHides), Category = "Interactable Settings|Float|Bobbing")
	float BobFrequency = 2.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ToolTip = "Whether the object spins continuously while floating"), Category = "Interactable Settings|Float|Spinning")
	bool bEnableSpinning = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ToolTip = "Degrees per second the object spins (Yaw) while floating", EditCondition = "bEnableSpinning", EditConditionHides), Category = "Interactable Settings|Float|Spinning")
	float SpinSpeed = 90.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ToolTip = "How fast the object rises to the target float height", ClampMin = "0.0", UIMin = "0.0"), Category = "Interactable Settings|Float")
	float FloatInterpSpeed = 3.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ToolTip = "Max distance to trace downward for ground detection while dragging", ClampMin = "0.0", UIMin = "0.0"), Category = "Interactable Settings|Float")
	float GroundTraceDistance = 1000.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ToolTip = "How close to the ground the object must be to be considered landed", ClampMin = "0.0", UIMin = "0.0"), Category = "Interactable Settings|Float")
	float LandingDistanceThreshold = 5.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ToolTip = "Max velocity to be considered at rest when landing", ClampMin = "0.0", UIMin = "0.0"), Category = "Interactable Settings|Float")
	float LandingVelocityThreshold = 10.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ToolTip = "How fast the object straightens itself while being dragged", ClampMin = "0.0", UIMin = "0.0"), Category = "Interactable Settings|Float")
	float UprightInterpSpeed = 1.5f;
	
#pragma endregion
	
#pragma region VARIABLES
private: 
	UPROPERTY()
	TObjectPtr<ACharacter> CurrentInteractor;
	
	float GripDistance = 0.0f; // Distance from the interactor to the object, used to check break grip tolerance
	float DragDistance = 0.0f; // Horizontal distance to maintain in front of the player while dragging
	float FloatBaseZ = 0.0f; // The Z position the object should float at (rest position + FloatHeight)
	float BobTimer = 0.0f; // Accumulated time for the bobbing sine wave
	float UprightTargetYaw = 0.0f; // The original Yaw to preserve while straightening Roll and Pitch
	bool bIsDragging = false; // True while the object is being actively dragged
	bool bIsFalling = false; // True after the interaction ends, while the object is falling back to the ground
#pragma endregion
	
#pragma region CONSTRUCTOR
public:
	AFreeDragObj();
#pragma endregion
	
#pragma region METHODS
private:
	virtual void Tick(float DeltaSeconds) override;
	void HandleFalling();
	void CheckGripBreak();
	void CheckInteractorAbove();
	void UpdateFollowing(float DeltaSeconds);
	void UpdateBobbing(float DeltaSeconds);
	void UpdateUprighting(float DeltaSeconds);
	void UpdateSpinning(float DeltaSeconds);
	void ApplySpeedMultiplierToOwner(TObjectPtr<ACharacter> Interactor, float Multiplier) const;
	bool TraceGround(const FVector& Origin, float TraceDistance, FHitResult& OutHit) const; // Line trace downward from Origin; returns true if ground was hit
#pragma endregion
	
#pragma region INTERFACE METHODS
public:
	virtual void ExecuteStartInteraction(TObjectPtr<ACharacter> Interactor) override;
	virtual void EndInteraction(TObjectPtr<ACharacter> Interactor) override;
	
	virtual void Reset() override;
#pragma endregion
};