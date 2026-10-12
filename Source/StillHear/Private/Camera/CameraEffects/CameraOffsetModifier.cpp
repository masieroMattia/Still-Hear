#include "Camera/CameraEffects/CameraOffsetModifier.h"

bool UCameraOffsetModifier::ModifyCamera(const float DeltaTime, FMinimalViewInfo& InOutPOV)
{
	Super::ModifyCamera(DeltaTime, InOutPOV);
	InOutPOV.Location += OffsetToApply;
	return false;
}
