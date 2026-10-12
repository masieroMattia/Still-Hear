#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "CheckpointDisplayData.generated.h"

/**
 * One row per checkpoint
 * Used to drive save slot labels, background images, and any other checkpoint-specific UI
 */
USTRUCT(BlueprintType)
struct STILLHEAR_API FCheckpointDisplayData : public FTableRowBase
{
	GENERATED_BODY()

	// Display name shown in the save slot and main menu
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	FText DisplayName;

	// Background / thumbnail image for this checkpoint
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	TSoftObjectPtr<UTexture2D> BackgroundImage;
};
