#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySystem/Cues/Generic/GC_SoundAndVFXActor.h"
#include "GC_ParryWindow.generated.h"

class UParryData;

UCLASS()
class STILLHEAR_API AGC_ParryWindow : public AGC_SoundAndVFXActor
{
	GENERATED_BODY()

#pragma region CONSTANTS
private:
	static const FName RadiusParamName;
	static const FName DurationParamName;
#pragma endregion

#pragma region UPROPERTIES
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Settings")
	TObjectPtr<UParryData> ParryData;
#pragma endregion
	
#pragma region CONSTRUCTOR
	AGC_ParryWindow();
#pragma endregion
	
#pragma region METHODS
	virtual bool OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
#pragma endregion
};