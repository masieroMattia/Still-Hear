#pragma once

#include "CoreMinimal.h"
#include "PCGComponent.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "PCGSplineBase.generated.h"

UCLASS(Abstract)
class STILLHEAR_API APCGSplineBase : public AActor
{
	GENERATED_BODY()

#pragma region UPROPERTIES
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USplineComponent> Spline;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPCGComponent> PCGComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG Settings|Config")
	TArray<TObjectPtr<UStaticMesh>> Meshes;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG Settings|Config")
	float Distance = 100.0f;
#pragma endregion 
	
#pragma region CONSTRUCTOR
	APCGSplineBase();
#pragma endregion
};
