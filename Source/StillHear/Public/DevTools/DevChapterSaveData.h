#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DevChapterSaveData.generated.h"

/**
 * DEV TOOL — holds the serialized bytes of a captured USaveGameObject for one chapter.
 * These bytes are produced in-editor via UGameplayStatics::SaveGameToMemory (see
 * CaptureFromPlaySession) so they get packaged into shipping builds — loose .sav files
 * in Saved/SaveGames are NOT packaged.
 */
UCLASS(BlueprintType)
class STILLHEAR_API UDevChapterSaveData : public UDataAsset
{
	GENERATED_BODY()

public:
	// Serialized USaveGameObject bytes (as produced by UGameplayStatics::SaveGameToMemory).
	UPROPERTY(VisibleAnywhere, Category = "Chapter")
	TArray<uint8> SaveBytes;

	// Free-form note about where/when this snapshot was captured (e.g. checkpoint name).
	UPROPERTY(EditAnywhere, Category = "Chapter")
	FString Note;

#if WITH_EDITOR
	/**
	 * Editor capture button. Play in editor, cross the saving checkpoint you want this chapter to
	 * start at (that rebuilds the current save), then click this to snapshot it into SaveBytes.
	 * Remember to save the asset (Ctrl+S) afterwards.
	 */
	UFUNCTION(CallInEditor, Category = "Chapter|Capture")
	void CaptureFromPlaySession();
#endif
};
