#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"

/**
 * Mapping of Trace and Collision channels used in the game.
 * This namespace provides a centralized place to define and manage custom collision channels,
 * making it easier to maintain and avoid hardcoding channel values throughout the codebase.
 */
namespace ECustomCollision
{
	static constexpr ECollisionChannel Player = ECC_GameTraceChannel1;
	static constexpr ECollisionChannel Companion = ECC_GameTraceChannel2;
	static constexpr ECollisionChannel Interactable = ECC_GameTraceChannel3;
	static constexpr ECollisionChannel Resonance = ECC_GameTraceChannel4;
	static constexpr ECollisionChannel Climb = ECC_GameTraceChannel5;
	static constexpr ECollisionChannel Worm = ECC_GameTraceChannel6;
	static constexpr ECollisionChannel Floor = ECC_GameTraceChannel7;
	static constexpr ECollisionChannel Mantis = ECC_GameTraceChannel8;
}