#include "Camera/CameraEffects/CameraFOVModifier.h"

UCameraFOVModifier::UCameraFOVModifier()
{
	FOVDeltaToApply = 0.0f;
}

bool UCameraFOVModifier::ModifyCamera(const float DeltaTime, struct FMinimalViewInfo& InOutPOV)
{
	Super::ModifyCamera(DeltaTime, InOutPOV);
	
	InOutPOV.FOV += FOVDeltaToApply;
	
	// Clamp FOV to reasonable limits to avoid inverted or glitched rendering
	InOutPOV.FOV = FMath::Clamp(InOutPOV.FOV, 5.0f, 170.0f);
	
	return false;
}
