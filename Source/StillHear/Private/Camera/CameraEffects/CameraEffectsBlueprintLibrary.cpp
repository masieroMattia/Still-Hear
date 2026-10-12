#include "Camera/CameraEffects/CameraEffectsBlueprintLibrary.h"

#include "Kismet/GameplayStatics.h"
#include "Camera/CameraEffects/CameraEffectsComponent.h"

UCameraEffectsComponent* UCameraEffectsBlueprintLibrary::GetCameraEffects(const UObject* WorldContextObject)
{
	const APlayerController* PlayerController = UGameplayStatics::GetPlayerController(WorldContextObject, 0);
	return PlayerController ? PlayerController->FindComponentByClass<UCameraEffectsComponent>() : nullptr;
}