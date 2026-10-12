#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CameraEffectsBlueprintLibrary.generated.h"

class UCameraEffectsComponent;

UCLASS()
class STILLHEAR_API UCameraEffectsBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Camera Effects", meta = (WorldContext = "WorldContextObject"))
	static UCameraEffectsComponent* GetCameraEffects(const UObject* WorldContextObject);
};
