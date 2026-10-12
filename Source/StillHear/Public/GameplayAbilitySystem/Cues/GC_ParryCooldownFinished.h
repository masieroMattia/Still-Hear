#pragma once

#include "CoreMinimal.h"
#include "Data/DataAssets/ParryData.h"
#include "Generic/GC_CooldownFinishedBase.h"
#include "GC_ParryCooldownFinished.generated.h"

UCLASS()
class STILLHEAR_API AGC_ParryCooldownFinished : public AGC_CooldownFinishedBase
{
	GENERATED_BODY()

#pragma region UPROPERTIES
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Data")
	TObjectPtr<UParryData> ParryData;
#pragma endregion

#pragma region METHODS
protected:
	virtual USoundBase* GetFinishCooldownSound() const override;
#pragma endregion
};
