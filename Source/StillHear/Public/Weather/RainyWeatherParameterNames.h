#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"

namespace RainyWeatherParameterNames
{
	// Lightning Material
	inline const FName LightningPercentage = "LightningPercentage";
	inline const FName EmissiveMultiplier = "EmissiveMultiplier";
	inline const FName LightningTexture = "LightningTexture";
	inline const FName LightningColor = "LightningColor";

	// Rain VFX
	inline const FName SpawnRate = "SpawnRate";
	inline const FName BoxSize = "BoxSize";
	inline const FName Velocity = "Velocity";
	inline const FName SpriteSize = "SpriteSize";

	// Rain Material Parameter Collection
	inline const FName Rain = "Rain";
	inline const FName Wind = "Wind";
	inline const FName Wetness = "Wetness";

	// Volumetric Clouds Material Parameter Collection
	inline const FName StormClouds = "StormClouds";
	inline const FName Storm_LightningTexScale = "Storm_LightningTexScale";
	inline const FName LightningFlicker = "LightningFlicker";
	inline const FName DynamicLightingAnim = "DynamicLightingAnim";
	inline const FName ManualLightningAnim = "ManualLightningAnim";
	inline const FName SourcePower = "SourcePower";
	inline const FName FillScatter = "FillScatter";
	inline const FName FillScatterIntensity = "FillScatterIntensity";
	inline const FName SecondMipLevel = "SecondMipLevel";
	inline const FName LightningMaskBias = "LightningMaskBias";
	inline const FName LightningMaskStrength = "LightningMaskStrength";
	inline const FName CloudTextureWeight = "CloudTextureWeight";
	inline const FName Storm_LightningColor = "Storm_LightningColor";
	inline const FName Storm_AlbedoColor = "Storm_AlbedoColor";
};
