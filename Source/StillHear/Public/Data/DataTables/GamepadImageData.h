#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GamepadImageData.generated.h"

/**
 * Data Table row for information about the correct controller image to display in the settings.
 */
USTRUCT(BlueprintType)
struct FGamepadImageData : public FTableRowBase
{
	GENERATED_BODY()

	// The name of the gamepad preset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gamepad Settings Data")
	FText PresetName;
	
	// The input mapping context associated with the images
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gamepad Settings Data")
	TSoftObjectPtr<class UInputMappingContext> InputMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gamepad Settings Data")
	TSoftObjectPtr<class UTexture2D> XboxImageEng;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gamepad Settings Data")
	TSoftObjectPtr<class UTexture2D> XboxImageIta;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gamepad Settings Data")
	TSoftObjectPtr<class UTexture2D> PlaystationImageEng;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gamepad Settings Data")
	TSoftObjectPtr<class UTexture2D> PlaystationImageIta;
};
