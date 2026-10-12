#include "Data/DataAssets/FootStepData.h"

#include "Sound/SoundBase.h"

USoundBase* UFootStepData::GetSoundForSurface(const EPhysicalSurface SurfaceType) const
{
	if (const FFootstepEffects* FoundEffects = SurfaceEffectsMap.Find(SurfaceType))
	{
		return FoundEffects->Sound.Get();
	}

	return DefaultEffects.Sound.Get();
}

UNiagaraSystem* UFootStepData::GetVFXForSurface(const EPhysicalSurface SurfaceType) const
{
	if (const FFootstepEffects* FoundEffects = SurfaceEffectsMap.Find(SurfaceType))
	{
		return FoundEffects->VFX.Get();
	}

	return DefaultEffects.VFX.Get();
}
