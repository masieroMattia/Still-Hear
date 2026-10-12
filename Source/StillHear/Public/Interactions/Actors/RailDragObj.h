#pragma once

#include "CoreMinimal.h"
#include "InteractableObj.h"
#include "Interactions/Utils/InteractableRail.h"
#include "RailDragObj.generated.h"

UCLASS()
class STILLHEAR_API ARailDragObj : public AInteractableObj
{
	GENERATED_BODY()

#pragma region UPROPERTIES
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Rail")
	TObjectPtr<AInteractableRail> RailActor; // Reference to the actor containing the spline component for the rail
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interactable Settings|Effect")
	TSubclassOf<class UGameplayEffect> DragEffectClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"), Category = "Settings|Config")
	float DragMultiplier = 0.5f; // Multiplier applied to the character's speed when dragging the object
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", UIMin = "0.0"), Category = "Settings|Config")
	float ObjectLerpSpeed = 10.0f; // Speed at which the object moves towards the target position on the rail, higher values will make the object snap to the target position faster
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Config")
	float HeightOffset = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Config")
	FRotator RotationOffset = FRotator::ZeroRotator;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", UIMin = "0.0"), Category = "Interactable Settings|Config")
	float BreakGripTolerance = 50.0f;
#pragma endregion
	
#pragma region VARIABLES
private:
	UPROPERTY()
	TObjectPtr<ACharacter> CurrentInteractor;
	
	FVector GrabOffset = FVector::ZeroVector; // Offset from the object's location to the interactor's location at the moment of grabbing
	float CurrentDistanceAlongRail = 0.0f; // Current distance along the rail, used to determine the object's position on the rail during dragging
		
	float MaxAllowedSquareDistance = 0.0f; // Distance from the interactor to the object in 3D space, used to determine when to break the interaction if the interactor moves too far from the object
#pragma endregion
	
#pragma region CONSTRUCTOR
public:
	ARailDragObj();
#pragma endregion

#pragma region METHODS
private:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaSeconds) override;
	void ApplySpeedMultiplierToOwner(TObjectPtr<ACharacter> Interactor, float Multiplier) const;
#pragma endregion
	
#pragma region INTERFACE METHODS
	virtual void ExecuteStartInteraction(TObjectPtr<ACharacter> Interactor) override;
	virtual void EndInteraction(TObjectPtr<ACharacter> Interactor) override;
#pragma endregion
};