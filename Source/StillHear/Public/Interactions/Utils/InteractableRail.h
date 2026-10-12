#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "InteractableRail.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class STILLHEAR_API AInteractableRail : public AActor
{
	GENERATED_BODY()

#pragma region CONSTRUCTOR
public:
	AInteractableRail();
#pragma endregion

#pragma region UPROPERTIES
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USplineComponent> SplineComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", UIMin = "0.0", EditCondition = "bSnapToFloor", HideEditConditionToggle), Category = "Settings|Config")
	float FloorOffset = 10.0f; 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Config")
	bool bSnapToFloor = true;
#pragma endregion
	
#pragma region METHODS
public:
	USplineComponent* GetSpline() const;
private:
	virtual void OnConstruction(const FTransform& Transform) override;
#pragma endregion
};
