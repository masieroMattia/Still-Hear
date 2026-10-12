#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DevToolsSubsystem.generated.h"

class UDataTable;
class UTexture2D;
class UDevChapterSaveData;
class FDevKeySequenceProcessor;

/** Lightweight, Blueprint-friendly view of one chapter for the level-skip widget. */
USTRUCT(BlueprintType)
struct FDevChapterEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "DevTools")
	FName RowName;

	UPROPERTY(BlueprintReadOnly, Category = "DevTools")
	FText DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "DevTools")
	TSoftObjectPtr<UTexture2D> Thumbnail;
};

/**
 * DEV TOOL — owns the global key-sequence listener and drives the hidden level-skip menu.
 * Registered as a GameInstance subsystem so it exists in the menu and in-game alike, in every
 * build configuration (the "hidden in shipping" guarantee comes from the secret sequence, not
 * from stripping the code).
 */
UCLASS()
class STILLHEAR_API UDevToolsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Called by the input pre-processor for every key down; advances/resets the sequence matcher. */
	void FeedKey(const FKey& Key);

	// ── Blueprint API for WBP_DevLevelSelect ──────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "DevTools")
	TArray<FDevChapterEntry> GetChapters() const;

	UFUNCTION(BlueprintCallable, Category = "DevTools")
	bool SkipToChapter(FName RowName);

	UFUNCTION(BlueprintCallable, Category = "DevTools")
	void CloseLevelSelect();

private:
	void OnUnlockSequenceEntered();
	void OpenLevelSelect();
	UDataTable* GetChapterTable() const;

	TSharedPtr<FDevKeySequenceProcessor> InputProcessor;

	// Sequence-matcher state
	int32 MatchIndex = 0;
	double LastKeyTime = 0.0;
};
