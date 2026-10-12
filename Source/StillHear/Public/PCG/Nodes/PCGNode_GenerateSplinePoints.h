#pragma once

#include "CoreMinimal.h"
#include "PCGNode_CustomBase.h"
#include "PCGNode_GenerateSplinePoints.generated.h"

UCLASS(BlueprintType, ClassGroup = (PCG))
class STILLHEAR_API UPCGNode_GenerateSplinePointsSettings : public UPCGNode_CustomBaseSettings
{
	GENERATED_BODY()
	
#pragma region UPROPERTIES
public:
	UPROPERTY(VisibleAnywhere, Category = "Node Settings")
	FName MeshAttributeName = TEXT("MeshToSpawn");
#pragma endregion
	
#pragma region INFO
#if WITH_EDITOR
	virtual FText GetDefaultNodeTitle() const override;
#endif
#pragma endregion

#pragma region PINS
	virtual TArray<FPCGPinProperties> InputPinProperties() const override;
	virtual TArray<FPCGPinProperties> OutputPinProperties() const override;
#pragma endregion
	
#pragma region LOGIC CREATOR
protected:
	virtual FPCGElementPtr CreateElement() const override; // Creates the element that will execute the logic of this node
#pragma endregion
};
	
class FPCGNode_GenerateSplinePointsElement : public IPCGElement
{
#pragma region LOGIC EXECUTOR
protected:
	virtual bool ExecuteInternal(FPCGContext* Context) const override; // Executes the logic of this element. Returns true if execution was successful, false otherwise
#pragma endregion
};