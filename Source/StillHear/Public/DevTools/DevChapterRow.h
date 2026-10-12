#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DevChapterRow.generated.h"

class UDevChapterSaveData;

/**
 * DEV TOOL — one row per selectable chapter in the level-skip menu.
 * Name and thumbnail are pulled from the existing checkpoint display DataTable via CheckpointId
 * (same source the real save-slot buttons use), so they aren't duplicated here.
 */
USTRUCT(BlueprintType)
struct STILLHEAR_API FDevChapterRow : public FTableRowBase
{
	GENERATED_BODY()

	// Checkpoint id — resolves the display name + image from the checkpoint display DataTable.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chapter")
	FName CheckpointId;

	// Lower values appear first in the list.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chapter")
	int32 SortOrder = 0;

	// Data asset holding the captured save bytes to load when this chapter is picked.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chapter")
	TSoftObjectPtr<UDevChapterSaveData> SaveData;
};
