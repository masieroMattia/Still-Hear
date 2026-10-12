#pragma once

#include "CoreMinimal.h"
#include "PCGSplineBase.h"
#include "Components/SplineMeshComponent.h"
#include "SplineSmoothConnectedMeshes.generated.h"


UCLASS()
class STILLHEAR_API ASplineSmoothConnectedMeshes : public APCGSplineBase
{
	GENERATED_BODY()
	
public:
	ASplineSmoothConnectedMeshes();

#pragma region UPROPERTIES
	// Defines which axis of your static mesh acts as the forward direction (usually X or Y)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG Settings|Smooth Spline")
	TEnumAsByte<ESplineMeshAxis::Type> ForwardAxis;

	// The up direction to prevent the spline mesh from twisting incorrectly
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG Settings|Smooth Spline")
	FVector UpDirection;
	
	// Toggle to enable or disable physical collisions on the generated meshes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG Settings|Smooth Spline")
	bool bEnableCollision = true;

	// Cross-section scale applied to each spline mesh segment
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG Settings|Smooth Spline")
	FVector2D MeshScale = FVector2D(1.0f, 1.0f);

	// Distance to extend each mesh beyond its segment to overlap with adjacent meshes and prevent gaps
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG Settings|Smooth Spline", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float OverlapAmount = 5.0f;
#pragma endregion
};
