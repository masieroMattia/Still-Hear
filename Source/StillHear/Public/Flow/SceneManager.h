#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "SaveSystem/Savable.h"
#include "GameFramework/Actor.h"
#include "SaveSystem/SaveIdComponent.h"
#include "SceneManager.generated.h"

/** This class is responsible for managing the flow of the game, such as transitioning between scenes,
handling level streaming, and managing character spawn/reset.
Load all level streaming hided, read from save the current level streaming to load, spawn the character in the right position, and then
call the event to start the game (es. start music, anim, ecc.). When the player dies, call the event to reset the character and reload the current level streaming.
 */


// Context in which ShowStreamingLevels is invoked, so OnAllLevelsShown
// can dispatch the right follow-up logic.
UENUM()
enum class EShowLevelsContext : uint8
{
	/** First time the scene is built (BeginPlay / TryInitializeScene). */
	InitialLoad,
	/** A checkpoint changed level visibility at runtime – no spawn/reset needed. */
	RuntimeTransition,
	/** Player died and we are restoring the saved level layout. */
	Respawn,
	/** Loading a save game or starting a new game from the menu. */
	LoadGame
};



UCLASS()
class STILLHEAR_API ASceneManager : public AActor, public ISavable
{
	GENERATED_BODY()

private:
	// Variable to bypass savegame
	UPROPERTY(EditAnywhere, Category="SceneManager")
	bool bBypassSaveGame = false;

	// Whether the game should start in menu mode (for streaming levels and cameras)
	UPROPERTY(EditAnywhere, Category="SceneManager|Menu")
	bool bStartInMenuMode = true;

	// Default streaming levels to load if there is no SaveGame
	UPROPERTY(EditAnywhere, Category="SceneManager|Menu")
	TArray<FName> DefaultStartingLevels;

	// Tag of the default starting camera volume (placed in a streamed level)
	UPROPERTY(EditAnywhere, Category="SceneManager|Menu")
	FName DefaultMenuCameraTag = FName("DefaultMenuCamera");

	// Data table (FCheckpointDisplayData) mapping checkpoint IDs to display names and background images
	UPROPERTY(EditAnywhere, Category="SceneManager|Menu")
	TObjectPtr<UDataTable> CheckpointDataTable;

	// Row name used when a save has no checkpoint yet (new game / initial main menu)
	UPROPERTY(EditAnywhere, Category="SceneManager|Menu")
	FName DefaultCheckpointId = FName("Default");

	// The root flow graph asset to start when the scene is loaded and shown
	UPROPERTY(EditAnywhere, Category="SceneManager|Flow")
	TObjectPtr<class UFlowAsset> RootFlowAsset;

	// The saved instance name of the root flow graph to restore it on load
	UPROPERTY(SaveGame)
	FString SavedRootFlowInstanceName;
	
	// Array of streaming levels to load at the beginning of the game, set in the editor
	UPROPERTY(EditAnywhere, Category="SceneManager")
    TArray<FName> LevelsStreamingToLoad;
	
	// Array of streaming levels to show at the beginning of the game, set in the editor, if save is empty
	UPROPERTY(EditAnywhere, Category="SceneManager",SaveGame)
	TArray<FName> StreamingLevelsLoadedFromSave;
	
	// Array of streaming levels to show at the beginning of the game, from save
	UPROPERTY(EditAnywhere, Category="SceneManager")
	TArray<FName> StartingStreamingLevelsToShow;
	
	// Player character class to spawn, set in the editor
	UPROPERTY(EditAnywhere, Category="SceneManager")
	TSubclassOf<ACharacter> PlayerCharacterClass;
	
	// Reference to the spawned player character
	UPROPERTY(VisibleAnywhere, Category="SceneManager")
	TObjectPtr<ACharacter> PlayerCharacter;
	
	// Player start actor to determine the spawn location of the character, set in the editor
	UPROPERTY(EditAnywhere, Category="SceneManager")
	TObjectPtr<AActor> PlayerStart;
	
	// Player start location to save and load
	UPROPERTY(SaveGame)
	FVector PlayerStartLocation;
	UPROPERTY(SaveGame)
	FRotator PlayerStartRotation;

	// Current checkpoint priority, saved with the game
	UPROPERTY(SaveGame)
	int32 CurrentCheckpointPriority = -1;

	// ── Runtime state (NOT saved) ──────────────────────────────────
	// Tracks the levels that are currently visible at runtime.
	// This can differ from the saved state when non-saving checkpoints
	// change visibility without persisting.
	UPROPERTY()
	TArray<FName> RuntimeVisibleLevels;
	
	// Componet to enable save
	UPROPERTY(VisibleAnywhere, Category="SceneManager")
	TObjectPtr<USaveIdComponent> SaveIdComponent;

	// ── Internal state: level LOADING (LoadStreamLevel → hidden in memory) ──
	int32 PendingLevelsToLoad = 0;
	bool bLevelsReady = false;
	bool bSaveLoaded = false;
	bool bSceneInitialized = false;

	// ── Internal state: level SHOWING (SetShouldBeVisible → OnLevelShown) ──
	int32 TotalLevelsToShow = 0;
	int32 CurrentLevelsShown = 0;

	// ── Internal state: level HIDING (SetShouldBeVisible(false) → OnLevelHidden) ──
	int32 TotalLevelsToHide = 0;
	int32 CurrentLevelsHidden = 0;

	// Levels to restore after a pending hide completes (used by RestoreStreamingLevels)
	TArray<FName> PendingRestoreLevels;

	// Tracks which context triggered the current ShowStreamingLevels batch
	EShowLevelsContext ActiveShowContext = EShowLevelsContext::RuntimeTransition;

	// Tracks which context to use when showing levels after a pending hide completes
	EShowLevelsContext PendingShowContext = EShowLevelsContext::InitialLoad;

	// True while a level show/hide batch is mid-flight; LoadSlotAndTransition defers
	// itself until it clears so two transitions don't stomp the shared counters/arrays.
	bool bIsLevelTransitionActive = false;

	// Track if we are currently in menu mode
	bool bIsInMenuMode = true;

	class ACameraVolume* FindCameraVolumeAtLocation(const FVector& Location) const;
	void SetupMenuView(bool bHasSaveData);
	void StartFlowRoot();

	/** Callback for LoadStreamLevel completion (level loaded into memory, still hidden). */
	UFUNCTION()
	void OnLevelLoaded();

	/** Callback bound to ULevelStreaming::OnLevelShown (level became visible). */
	UFUNCTION()
	void OnLevelShown();

	/** Callback bound to ULevelStreaming::OnLevelHidden (level became invisible). */
	UFUNCTION()
	void OnLevelHidden();

	/** Called internally when a single level is confirmed shown; bumps counter and checks completion. */
	void IncrementShownCount();

	/** Called internally when a single level is confirmed hidden; bumps counter and checks completion. */
	void IncrementHiddenCount();

	/** Called when every level in the current ShowStreamingLevels batch is visible. */
	void OnAllLevelsShown();

	/** Called when every level in the current hide batch is invisible. Proceeds with pending show. */
	void OnAllLevelsHidden();

	/** Called when every level in LevelsStreamingToLoad has finished loading into memory. */
	void OnAllLevelsReady();

	void TryInitializeScene();

	UFUNCTION()
	void OnCharacterInitialized();

	UFUNCTION()
	void OnCharacterDeath();

	void ShowDeathScreen();
	void RestoreStreamingLevels();
	
	public:
	
	// Sets default values for this actor's properties
	ASceneManager();

	UDataTable* GetCheckpointDataTable() const { return CheckpointDataTable; }
	FName GetDefaultCheckpointId() const { return DefaultCheckpointId; }
	
	/**
	 * Updates ONLY the runtime-visible levels (visual transition).
	 * Does NOT modify any saved state. Used by non-saving checkpoints
	 * and by saving checkpoints that cannot activate (going backwards).
	 */
	void UpdateRuntimeVisibility(const TArray<FName>& LevelsToShow, const TArray<FName>& LevelsToHide);

	/**
	 * Updates the saved state (levels, position, priority) and persists to disk.
	 * Call ONLY when a saving checkpoint is activated with valid priority.
	 * Also updates runtime visibility via UpdateRuntimeVisibility.
	 */
	void UpdateSavedState(const TArray<FName>& LevelsToShow, const TArray<FName>& LevelsToHide,
						  const FVector& Location, const FRotator& Rotation, int32 Priority);

	// Returns true if the given checkpoint priority is higher than the current one (i.e., the player is progressing forward)
	bool CanActivateCheckpoint(int32 Priority) const;
    
	protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	public:
	void LoadLevelsStreaming(const TArray<FName>& Levels);
	void UnloadLevelsStreaming(const TArray<FName>& Levels);
	void ShowStreamingLevels(const TArray<FName>& Levels, EShowLevelsContext Context = EShowLevelsContext::RuntimeTransition);
	void HideStreamingLevels(const TArray<FName>& Levels);
	void SpawnCharacter();
	void ResetCharacter();
	void ResetActors() const;
	void TrySaveGame();

	
	virtual void OnPostLoad_Implementation() override;

	void TransitionToGameplay();
	bool GetStartInMenuMode() const { return bStartInMenuMode; }
	bool IsInMenuMode() const { return bIsInMenuMode; }

	UFUNCTION(BlueprintCallable, Category = "Game")
	void ReturnToMainMenu();

	// bAllowFullReload: true triggers a pristine-world reload first (like New Game); the
	// post-reload hand-off calls back in with bAllowFullReload=false to do the actual load.
	UFUNCTION(BlueprintCallable, Category="SceneManager")
	bool LoadSlotAndTransition(int32 SlotIndex, bool bAllowFullReload = true);

	void RefreshMenuBackground();
	void ResetMenuToDefault();
};
