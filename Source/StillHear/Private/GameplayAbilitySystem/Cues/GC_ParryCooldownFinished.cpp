#include "GameplayAbilitySystem/Cues/GC_ParryCooldownFinished.h"

USoundBase* AGC_ParryCooldownFinished::GetFinishCooldownSound() const
{
	return ParryData ? ParryData->FinishCooldownSound : nullptr;
}
