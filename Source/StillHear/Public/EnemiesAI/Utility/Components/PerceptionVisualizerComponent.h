#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PerceptionVisualizerComponent.generated.h"

class UAIMantisInfo_DataAsset;
class AAIMantisCharacter;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class STILLHEAR_API UPerceptionVisualizerComponent : public UActorComponent
{
	GENERATED_BODY()

#pragma region UPROPERTIES
public:
	// Toggle to easily turn off the sight visualizer
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowSightDebug = true;

	// Toggle to easily turn off the line of sight visualizer
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowLineOfSightDebug = true;

	// Toggle to easily turn off the hearing visualizer
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowHearingDebug = true;
#pragma endregion
	
#pragma region VARIABLES
private:
	UPROPERTY()
	TObjectPtr<AAIMantisCharacter> MantisRef;
	UPROPERTY()
	TObjectPtr<UAIMantisInfo_DataAsset> DataAssetRef;
#pragma endregion
	
#pragma region CONSTRUCTOR
public:
	UPerceptionVisualizerComponent();
#pragma endregion
	
#pragma region METHODS
public:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void DrawVisionCones() const;
	void DrawLineOfSight() const;
	void DrawHearingCircles() const;
	void DrawConeSection(const FVector& Location, const FVector& Forward, float AngleDegrees, float Radius, const FColor& Color) const;
#pragma endregion
};
