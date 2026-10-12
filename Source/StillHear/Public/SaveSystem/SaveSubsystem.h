#pragma once

#include "CoreMinimal.h"
#include "SaveGameObject.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SaveSubsystem.generated.h"

class USettingsSaveGame;
struct FLevelSaveData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSaveEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSaveEventBool, bool, bSuccess);

/**
 * This SubSystem manages saving and loading game state to/from disk.
 * It serializes levels and actors into a USaveGameObject instance, which is then saved to a slot.
 * It also handles loading saved data and applying it to the current game world.
 */
UCLASS()
class STILLHEAR_API USaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public: 
	// Events to notify UI or other systems
	UPROPERTY(BlueprintAssignable, Category="Save|UI")
	FOnSaveEvent OnSaveStarted;

	UPROPERTY(BlueprintAssignable, Category="Save|UI")
	FOnSaveEventBool OnSaveFinished;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
	int32 MaxSaveSlots = 3;
	
	// Subsystem overrides, hook into map loading/level streaming
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	// Helpers to get stable keys for levels/maps
	static FName GetStableMapKey(const UWorld* World);
	static FName GetStableLevelKey(const UWorld* World, const ULevel* Level);

	// Getter for save in progress
	UFUNCTION(BlueprintPure, Category="Save")
	bool IsSaving() const { return bSaveInProgress; }
	
	// Save/Load functions
	UFUNCTION(BlueprintCallable, Category="Save")
	bool RequestSaveSlotAsync(int32 SlotIndex);
	
	// This function is not used for now 
	UFUNCTION(BlueprintCallable, Category="Save")
	bool SaveToSlot(int32 const SlotIndex, int32 const UserIndex = 0);
	
	UFUNCTION(BlueprintCallable, Category="Save")
	bool LoadFromSlot(int32 const SlotIndex, int32 const UserIndex = 0, bool bRestoreAudio = true);
	
	// If you want to force the manual application 
	UFUNCTION(BlueprintCallable, Category="Save")
	void ApplyLoadedDataToCurrentlyLoadedLevels();
	
	// Getter for current save slot index (1-3), -1 if none
	UFUNCTION(BlueprintCallable, Category="Save")
	int GetCurrentSlotSave() const;

	// Getter for the save slot index with the latest save date (1-3), -1 if none
	UFUNCTION(BlueprintCallable, Category="Save")
	int32 GetLatestSaveSlotIndex() const;

	UFUNCTION(BlueprintPure, Category="Save")
	bool HasAnySaveGames() const;

	UFUNCTION(BlueprintCallable, Category="Save")
	bool StartNewGame(int32 SlotIndex = -1);

	UFUNCTION(BlueprintCallable, Category="Save")
	void SetCurrentCheckpoint(FName CheckpointId);

	// Returns the display data for a given checkpoint ID (looks up CheckpointDataTable)
	UFUNCTION(BlueprintCallable, Category="Save")
	bool GetCheckpointDisplayData(FName CheckpointId, FText& OutName, TSoftObjectPtr<UTexture2D>& OutImage) const;

	// Returns the display data for the checkpoint stored in the current save
	UFUNCTION(BlueprintCallable, Category="Save")
	bool GetCurrentSaveCheckpointDisplayData(FText& OutName, TSoftObjectPtr<UTexture2D>& OutImage) const;

	// Returns the display data for a specific slot's checkpoint (loads from disk — use sparingly)
	UFUNCTION(BlueprintCallable, Category="Save")
	bool GetSlotCheckpointDisplayData(int32 SlotIndex, FText& OutName, TSoftObjectPtr<UTexture2D>& OutImage) const;

	UFUNCTION(BlueprintCallable, Category="Save")
	bool LoadLatestSaveGame();
	
	// Helper for Debug
	UFUNCTION(BlueprintCallable)
	FString DebugPrintCurrentSave() const;

	UFUNCTION(BlueprintCallable, Category="Save")
	void SetCurrentSlotToNewGame(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category="Save")
	void ResetGameSession();

	// ── DEV TOOL (level skip) ──────────────────────────────────────────────
	// Writes the given serialized save bytes into the dev scratch slot, then loads it through the
	// normal full-reload Load Game pipeline. The scratch slot is outside [1, MaxSaveSlots] so the
	// player's real slots and menu are never affected.
	bool SkipToChapterFromBytes(const TArray<uint8>& SaveBytes);

#if WITH_EDITOR
	// Editor-only: serialize the current in-memory save to bytes, for capturing a chapter template.
	bool CaptureCurrentSaveToBytes(TArray<uint8>& OutBytes) const;
#endif

	// Collectibles (global — stored in Settings, not in save slots)
	UFUNCTION(BlueprintCallable, Category="Save|Collectibles")
	void CollectCollectible(FName RowName);

	UFUNCTION(BlueprintPure, Category="Save|Collectibles")
	bool IsCollectibleCollected(FName RowName) const;

	UFUNCTION(BlueprintPure, Category="Save|Collectibles")
	TSet<FName> GetCollectedCollectibles() const;

	UFUNCTION(BlueprintCallable, Category="Save|Collectibles")
	void ResetCollectibles();
	
	// Deletes the save file for the specified slot
	UFUNCTION(BlueprintCallable, Category="Save")
	bool DeleteSlot(int32 const SlotIndex, int32 const UserIndex = 0) const;

	// Ability Persistence
	UFUNCTION(BlueprintCallable, Category="Save|Abilities")
	void UnlockPermanentAbility(EMainCharacterAbilityType AbilityType);

	UFUNCTION(BlueprintCallable, Category="Save|Abilities")
	void RemovePermanentAbility(EMainCharacterAbilityType AbilityType);

	UFUNCTION(BlueprintPure, Category="Save|Abilities")
	bool IsAbilityPermanentlyUnlocked(EMainCharacterAbilityType AbilityType) const;

	UFUNCTION(BlueprintPure, Category="Save|Abilities")
	TSet<EMainCharacterAbilityType> GetPermanentlyUnlockedAbilities() const;


private:
	// Internal state variables
	UPROPERTY()
	USaveGameObject* CurrentSave = nullptr;
	UPROPERTY() // keep alive during async save
	USaveGameObject* PendingSaveObject = nullptr;
	
	FString CurrentSlot;
	int32 CurrentUserIndex = 0;
	int32 CurrentSlotIndex = -1;
	bool bSaveInProgress = false;

	// True once ApplyLoadedDataToCurrentlyLoadedLevels has run at least once this session
	// Only the very first run needs to defer to next tick — see ApplyLoadedDataToCurrentlyLoadedLevels
	bool bHasAppliedInitialLoad = false;
	FString PendingSlotName;
	int32 PendingUserIndex = 0;
	
	// Hook map and level streaming
	void OnPostLoadMap(UWorld* LoadedWorld);
	void OnLevelAddedToWorld(ULevel* InLevel, UWorld* InWorld);
	void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld);
	
	// Serialization helpers 
	static void SerializeObject(UObject* Object, TArray<uint8>& OutBytes);
	static void DeserializeObject(UObject* Object, const TArray<uint8>& InBytes);
	
	// Load/Apply helper
	
	void ApplyLevel(ULevel* Level, bool bDeferToNextTick = true);
	void ApplyLevelInternal(ULevel* Level);
	
	// Save helpers
	static bool TryGetActorGuid(const AActor* Actor, FGuid& OutGuid);
	UFUNCTION()
	void HandleFlowCheckpointSaveRequested();
	void BuildSaveObject(USaveGameObject* SaveObj); // serialized all levels/actors into SaveObj
	void HandleAsyncSaveComplete(const FString& SlotName, const int32 UserIndex, bool bSuccess);

	FORCEINLINE static FString MakeSlotName(int32 const SlotIndex)
	{
		return FString::Printf(TEXT("Save_Slot%d"), SlotIndex);
	}

	class ASceneManager* GetSceneManager() const;
	
	void SaveLevelInto(USaveGameObject* TargetSave, ULevel* Level);
	void SaveActorInto(AActor* Actor, FLevelSaveData& LevelData);
	
	// For Settings
public:
	UFUNCTION(BlueprintCallable, Category="Settings")
	bool SaveSettingsAsync();

	UFUNCTION(BlueprintCallable, Category="Settings")
	bool LoadSettings();

	// Re-applies the currently saved upscaler settings to the engine CVars. Call after map load or on game start
	UFUNCTION(BlueprintCallable, Category="Settings")
	void ApplyUpscalerFromSettings();

	// Re-applies the saved language to the engine localization system. Call after map load or on game start
	UFUNCTION(BlueprintCallable, Category="Settings")
	void ApplyLanguageFromSettings();

	UFUNCTION(BlueprintPure, Category="Settings")
	USettingsSaveGame* GetSettings() const { return Settings; }

	UPROPERTY(BlueprintAssignable, Category="Settings|UI")
	FOnSaveEvent OnSettingsSaveStarted;

	UPROPERTY(BlueprintAssignable, Category="Settings|UI")
	FOnSaveEventBool OnSettingsSaveFinished;

	UFUNCTION(BlueprintCallable)
	FORCEINLINE USettingsSaveGame* GetSaveSettings() { return Settings; }

private:
	UPROPERTY()
	USettingsSaveGame* Settings = nullptr;

	UPROPERTY()
	USettingsSaveGame* PendingSettingsSave = nullptr;

	bool bSettingsSaveInProgress = false;

	static FString SettingsSlotName() { return TEXT("Settings"); }

	void HandleAsyncSettingsSaveComplete(const FString& SlotName, const int32 UserIndex, bool bSuccess);
};
