#include "GameplayAbilitySystem/Cues/GC_SoundWaveFinishCooldown.h"

USoundBase* AGC_SoundWaveFinishCooldown::GetFinishCooldownSound() const
{
	return SoundWaveData ? SoundWaveData->FinishCooldownSound : nullptr;
}