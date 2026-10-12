#pragma once

#include "CoreMinimal.h"
#include "MainCharacterAbilityType.generated.h"

UENUM(BlueprintType)
enum class EMainCharacterAbilityType : uint8
{
	Jump,
	Parry,
	Resonance,
	Sprint,
	Crouch,
	LowVault,
	Climb,
	SoundWave,
	Interaction
};
