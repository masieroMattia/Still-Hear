#pragma once

#include "CoreMinimal.h"
#include "PCGSplineBase.h"
#include "PCGSplineSequentialMeshes.generated.h"

UCLASS()
class STILLHEAR_API APCGSplineSequentialMeshes : public APCGSplineBase
{
	GENERATED_BODY()

#pragma region UPROPERTIES
public:
	UPROPERTY(EditAnywhere, Category = "PCG Settings|Config")
	bool bApplyOrientation = true;
	UPROPERTY(EditAnywhere, Category = "PCG Settings|Config")
	bool bShrinkToFitNextPoint = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG Settings|Config")
	FVector LocationOffset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG Settings|Config")
	FRotator RotationOffset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "!bApplyOrientation", EditConditionHides), Category = "PCG Settings|Config")
	FVector RandomRotation;
#pragma endregion 
};