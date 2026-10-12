#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Camera/CameraEffects/CameraEffectTypes.h"
#include "CameraEffectsPresetData.generated.h"

UCLASS(BlueprintType)
class STILLHEAR_API UCameraEffectsPresetData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	FCameraEffectPreset Preset;
};