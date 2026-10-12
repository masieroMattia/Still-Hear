#include "GameplayAbilitySystem/Cues/GC_SoundWaveAim.h"

#pragma region METHODS
bool AGC_SoundWaveAim::OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	if (!SoundWaveData || !MyTarget)
		return false;
    
	if (SoundWaveData->AimSound)
		SFX = SoundWaveData->AimSound;
    
	return Super::OnActive_Implementation(MyTarget, Parameters);
}

bool AGC_SoundWaveAim::OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	return Super::OnRemove_Implementation(MyTarget, Parameters);
}
#pragma endregion