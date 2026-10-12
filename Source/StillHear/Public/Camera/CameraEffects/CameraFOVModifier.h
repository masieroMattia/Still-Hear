#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraModifier.h"
#include "CameraFOVModifier.generated.h"

UCLASS()
class STILLHEAR_API UCameraFOVModifier : public UCameraModifier
{
	GENERATED_BODY()
	
public:
	UCameraFOVModifier();

	float FOVDeltaToApply;

	virtual bool ModifyCamera(float DeltaTime, struct FMinimalViewInfo& InOutPOV) override;
};
