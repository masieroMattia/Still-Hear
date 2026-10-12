#pragma once

#include "CoreMinimal.h"
#include "Generic/GC_SoundAndVFXActor.h"
#include "Data/DataAssets/SoundWaveData.h"
#include "GC_SoundWaveAim.generated.h"

UCLASS()
class STILLHEAR_API AGC_SoundWaveAim : public AGC_SoundAndVFXActor
{
	GENERATED_BODY()
	
#pragma region UPROPERTIES
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Audio")
	TObjectPtr<USoundWaveData> SoundWaveData;
#pragma endregion

#pragma region METHODS
	virtual bool OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
	virtual bool OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
#pragma endregion
};
