#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySystem/Cues/Generic/GC_SoundAndVFXBurst.h"
#include "GC_MantisShiftBurst.generated.h"

/**
 * Custom GameplayCueNotify_Burst for Mantis shift
 * Inherits from UGC_SoundAndVFXBurst and sets Niagara User Parameters
 */
UCLASS()
class STILLHEAR_API UGC_MantisShiftBurst : public UGC_SoundAndVFXBurst
{
	GENERATED_BODY()

public:
	UGC_MantisShiftBurst();

protected:
	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;
	virtual void SpawnBurstEffects(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;

#pragma region PROPERTIES
protected:
	// Name of the Niagara User Parameter for duration (float)
	UPROPERTY(EditDefaultsOnly, Category = "Settings|Mantis")
	FName DurationParamName = FName("User.Duration");

	// Name of the Niagara User Parameter for mode (bool or 0/1 float). 0 = Disappear, 1 = Reappear
	UPROPERTY(EditDefaultsOnly, Category = "Settings|Mantis")
	FName IsReappearingParamName = FName("User.IsReappearing");
#pragma endregion
};
