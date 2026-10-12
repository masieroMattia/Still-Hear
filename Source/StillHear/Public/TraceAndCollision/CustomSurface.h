#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"

/**
 * Mapping of Physical Surface Types channels used in the game.
 * This namespace provides a centralized place to define and manage custom physical surface types,
 * making it easier to maintain and avoid hardcoding values throughout the codebase.
 */
namespace ECustomSurface
{
	static constexpr EPhysicalSurface Grass = SurfaceType1;
	static constexpr EPhysicalSurface Soil = SurfaceType2;
}