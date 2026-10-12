#pragma once

#include "CoreMinimal.h"
#include "NiagaraComponent.h"
#include "TargetMarkerNiagaraComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class STILLHEAR_API UTargetMarkerNiagaraComponent : public UNiagaraComponent
{
	GENERATED_BODY()
	
#pragma region CONSTANTS
	const FName COLOR = FName("User.Color");
	const FName	BASE_SIZE = FName("User.BaseSize");
	const FName MARKER_SIZE = FName("User.MarkerSize");
	const FName BASE_OFFSET = FName("User.BaseOffset");
	const FName MARKER_OFFSET = FName("User.MarkerOffset");
#pragma endregion
	
#pragma region UPROPERTIES
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Marker Niagara Settings")
	FLinearColor Color = FLinearColor::White;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0f, UIMin = 0.0f), Category = "Target Marker Niagara Settings")
	float BaseSize = 250.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0f, UIMin = 0.0f), Category = "Target Marker Niagara Settings")
	float MarkerSize = 150.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0f, UIMin = 0.0f), Category = "Target Marker Niagara Settings")
	float BaseZOffset = 50.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Marker Niagara Settings")
	float MarkerZOffset = 50.0f;
#pragma endregion
	
#pragma region CONSTRUCTOR
public:
	UTargetMarkerNiagaraComponent();
#pragma endregion
	
#pragma region METHODS
protected:
	virtual void BeginPlay() override;
	
private:
	// Pushes the basic properties to the Niagara System
	void UpdateNiagaraVariables();
	// Dynamically calculates the top of the actor and applies the offset
	void UpdateMarkerPosition();
#pragma endregion
	
};
