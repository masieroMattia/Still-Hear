#pragma once

#include "CoreMinimal.h"
#include "Data/DataAssets/SoundWaveData.h"
#include "Generic/GC_CooldownFinishedBase.h"
#include "GC_SoundWaveFinishCooldown.generated.h"

UCLASS()
class STILLHEAR_API AGC_SoundWaveFinishCooldown : public AGC_CooldownFinishedBase
{
	GENERATED_BODY()

#pragma region UPROPERTIES
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Data")
	TObjectPtr<USoundWaveData> SoundWaveData;
#pragma endregion

#pragma region METHODS
protected:
	virtual USoundBase* GetFinishCooldownSound() const override;
#pragma endregion
};
