#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "Engine/DeveloperSettings.h"
#include "DevToolsSettings.generated.h"

class UDataTable;
class UCommonActivatableWidget;

/**
 * DEV TOOL — project-configurable settings for the hidden level-skip menu.
 * Lives in Project Settings ("Project > Dev Tools (Level Skip)") and is packaged into
 * shipping builds, so the unlock sequence and chapter table can be tuned without code.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Dev Tools (Level Skip)"))
class STILLHEAR_API UDevToolsSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override { return TEXT("Project"); }

	// The secret key sequence that opens the level-skip menu. Order matters (e.g. Up, Up, Down, Down, L, R).
	UPROPERTY(EditAnywhere, Config, Category = "Level Skip")
	TArray<FKey> UnlockSequence;

	// Max seconds allowed between consecutive keys before the in-progress sequence resets.
	UPROPERTY(EditAnywhere, Config, Category = "Level Skip", meta = (ClampMin = "0.1"))
	float SequenceResetSeconds = 1.5f;

	// DataTable of FDevChapterRow — the selectable chapters.
	UPROPERTY(EditAnywhere, Config, Category = "Level Skip")
	TSoftObjectPtr<UDataTable> ChapterTable;

	// Widget pushed onto the debug UI layer when the sequence is entered (WBP deriving from your ActivatableWidgetBase).
	UPROPERTY(EditAnywhere, Config, Category = "Level Skip")
	TSoftClassPtr<UCommonActivatableWidget> LevelSelectWidgetClass;

	// Save slot index used for skips. MUST be outside [1, USaveSubsystem::MaxSaveSlots] so it stays
	// invisible to every player-facing path (Continue / latest / New Game slot search all scan 1..Max).
	UPROPERTY(EditAnywhere, Config, Category = "Level Skip", meta = (ClampMin = "100"))
	int32 ScratchSlotIndex = 999;

	static const UDevToolsSettings* Get() { return GetDefault<UDevToolsSettings>(); }
};
