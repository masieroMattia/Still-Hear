#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "TargetSpot.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class STILLHEAR_API UTargetSpot : public USphereComponent
{
	GENERATED_BODY()

#pragma region UPROPERTIES
protected:
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "Spot Settings|Debug")
	bool bShowDebugLine = true;
	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.0f, UIMin = 0.0f, editCondition = "bShowDebugLine", editConditionHides), Category = "Spot Settings|Debug")
	float DebugLineLength = 500.0f;
	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.0f, UIMin = 0.0f, editCondition = "bShowDebugLine", editConditionHides), Category = "Spot Settings|Debug")
	float DebugLineThickness = 2.0f;
	UPROPERTY(EditAnywhere, meta = (editCondition = "bShowDebugLine", editConditionHides), Category = "Spot Settings|Debug")
	FColor DebugLineColor = FColor::Cyan;
#endif
#pragma endregion
	
#pragma region CONSTRUCTOR
	UTargetSpot();
#pragma endregion 
	
#pragma region METHODS
#if WITH_EDITOR
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
#endif
#pragma endregion
};
