#pragma once

#include "CoreMinimal.h"
#include "PCGSettings.h"
#include "Data/PCGPointData.h"
#include "PCGNode_CustomBase.generated.h"

UCLASS(Abstract, BlueprintType, ClassGroup = (PCG))
class STILLHEAR_API UPCGNode_CustomBaseSettings : public UPCGSettings
{
	GENERATED_BODY()
	
#pragma region UPROPERTIES
public:
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "Settings|Appearance")
	FLinearColor NodeTitleColor = FLinearColor(0.2f, 0.59f, 0.78f, 1.0f); 
#endif
#pragma endregion
	
#pragma region INFO
public:
#if WITH_EDITOR
	virtual EPCGSettingsType GetType() const override;
	virtual FLinearColor GetNodeTitleColor() const override;
#endif
#pragma endregion
};

class STILLHEAR_API FPCGNode_CustomBaseElement : public IPCGElement
{
protected:
	bool GetPointDataFromInput(FPCGContext* Context, const FName& PinName, const UPCGPointData*& OutData) const;
};