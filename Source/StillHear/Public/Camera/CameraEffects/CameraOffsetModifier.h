#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraModifier.h"
#include "CameraOffsetModifier.generated.h"

UCLASS()
class STILLHEAR_API UCameraOffsetModifier : public UCameraModifier
{
	GENERATED_BODY()
	
#pragma region VARIABLES
public:
	FVector OffsetToApply = FVector::ZeroVector;
#pragma endregion
	
#pragma region METHODS
protected:
	virtual bool ModifyCamera(float DeltaTime, FMinimalViewInfo& InOutPOV) override;
#pragma endregion
};
