#pragma once

#include "CoreMinimal.h"
#include "SoundDelegates.generated.h"

/*
 * Used to inform any system about audio levels change.
 * Handy in settings UI or anything that must respond to
 * audio levels change instantly.
 */
UDELEGATE()	//	Required when declaring delegates in a dedicated header file
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSoundLevelDelegate, USoundClass*, SoundClass, float, Level);
