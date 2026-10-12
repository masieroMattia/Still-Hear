# Scene Manager, Spawn & Checkpoint System

> **Project:** Still Hear  
> **Module:** `Source/StillHear/Public/Flow/` & `Source/StillHear/Private/Flow/`  
> **Last updated:** February 2026

---

## Table of Contents

1. [General Overview](#1-general-overview)
2. [Class Architecture](#2-class-architecture)
3. [ASceneManager — The Scene Director](#3-ascenemanager--the-scene-director)
   - 3.1 [Responsibilities](#31-responsibilities)
   - 3.2 [Boot Flow (BeginPlay)](#32-boot-flow-beginplay)
   - 3.3 [Editor-Configurable Properties](#33-editor-configurable-properties)
   - 3.4 [Level Streaming](#34-level-streaming)
   - 3.5 [Character Spawning](#35-character-spawning)
   - 3.6 [Death and Respawn](#36-death-and-respawn)
   - 3.7 [Save System Integration](#37-save-system-integration)
4. [Checkpoint System](#4-checkpoint-system)
   - 4.1 [ACheckpointBase — Abstract Class](#41-acheckpointbase--abstract-class)
   - 4.2 [ACheckpointTrigger — Box Trigger](#42-acheckpointtrigger--box-trigger)
   - 4.3 [Priority and Direction](#43-priority-and-direction)
   - 4.4 [Saving vs. Non-Saving](#44-saving-vs-non-saving)
5. [Designer Guide — Placing Checkpoints in the Level](#5-designer-guide--placing-checkpoints-in-the-level)
6. [Programmer Guide — Extending the System](#6-programmer-guide--extending-the-system)
7. [Flow Diagrams](#7-flow-diagrams)
8. [FAQ and Troubleshooting](#8-faq-and-troubleshooting)

---

## 1. General Overview

The **Scene Manager / Checkpoint** system handles three macro-features:

| Feature | Owner | Description |
|---|---|---|
| **Scene Management** | `ASceneManager` | Loads, shows and hides streaming sub-levels. |
| **Spawn / Respawn** | `ASceneManager` | Spawns the character at the correct position (from save or PlayerStart) and handles respawn after death. |
| **Checkpoints** | `ACheckpointBase` / `ACheckpointTrigger` | Triggers placed in the world that update the respawn point, level visibility and (optionally) persist progress to disk. |

All these elements work together with the **Save System** (`ISavable` interface, `USaveIdComponent`, `USaveSubsystem`) to persist the player's state across sessions.

---

## 2. Class Architecture

```
AActor
 ├── ASceneManager          (ISavable)     ← 1 instance per persistent level
 └── ACheckpointBase        (Abstract)     ← base class for all checkpoints
      └── ACheckpointTrigger               ← checkpoint with BoxComponent trigger
```

**Key relationships:**

- Every `ACheckpointBase` holds a **direct reference** to its `ASceneManager` (`SceneManager` property, set in the Editor).
- `ASceneManager` implements `ISavable` and owns a `USaveIdComponent` for automatic serialization of `SaveGame` properties.
- `ASceneManager` creates and owns the `PlayerCharacter` (manual spawn, not through GameMode).

---

## 3. ASceneManager — The Scene Director

**Files:** `Public/Flow/SceneManager.h` | `Private/Flow/SceneManager.cpp`

### 3.1 Responsibilities

1. **Load all streaming sub-levels** at startup (hidden).
2. **Read the save data** to determine which levels to show and where to spawn the character.
3. **Spawn the character** and attach it to the PlayerController.
4. **Handle death**: show the death screen, then the loading screen, then respawn.
5. **Expose delegates** for the UI (loading screen, death screen).
6. **Coordinate checkpoints** by providing the `UpdateRuntimeVisibility` and `UpdateSavedState` APIs.

### 3.2 Boot Flow (BeginPlay)

```
BeginPlay()
   │
   ├── Broadcast: OnLoadingScreenShow           ← UI shows loading screen
   │
   ├── For each level in LevelsStreamingToLoad:
   │       LoadStreamLevel(level, invisible)
   │       └── callback → OnLevelLoaded()
   │
   │   (in parallel, SaveSubsystem deserializes SaveGame properties)
   │       └── callback → OnPostLoad_Implementation()
   │
   └── When BOTH are complete:
           TryInitializeScene()
              │
              ├── If save data exists:
              │     ShowStreamingLevels(StreamingLevelsLoadedFromSave)
              │     SpawnCharacter() at saved PlayerStartLocation/Rotation
              │
              └── If NO save data:
                    ShowStreamingLevels(StartingStreamingLevelsToShow)
                    SpawnCharacter() at the PlayerStart actor's position
```

> **Note:** The two events (level loading + save loading) can complete in any order. `TryInitializeScene()` is called by both but only acts when **both** flags `bLevelsReady` and `bSaveLoaded` are `true`.

### 3.3 Editor-Configurable Properties

| Property | Type | Description |
|---|---|---|
| `bBypassSaveGame` | `bool` | **Debug only.** If `true`, ignores the save system and uses default values. Useful for testing scenes without saves. |
| `LevelsStreamingToLoad` | `TArray<FName>` | **All** sub-levels to load into memory at startup. They are loaded but **not** shown. |
| `StartingStreamingLevelsToShow` | `TArray<FName>` | Sub-levels to make visible at startup **if no save exists**. Represents the "initial state" of the scene. |
| `PlayerCharacterClass` | `TSubclassOf<ACharacter>` | The character class to spawn. |
| `PlayerStart` | `AActor*` | The actor that marks the initial spawn position (used only when there is no save). |
| `DeathToScreenDelay` | `float` (sec) | Time between character death and the death screen appearing. Default: **2.0s**. |
| `ScreenToRespawnDelay` | `float` (sec) | Time between the death screen appearing and the respawn. Default: **3.0s**. |

> ⚠️ **StreamingLevelsLoadedFromSave** is a `SaveGame` property — **do not** edit it manually in the Editor. It is written automatically by the checkpoint/save system.

### 3.4 Level Streaming

The Scene Manager manages sub-level visibility through two channels:

| Function | What it does | Called by |
|---|---|---|
| `LoadLevelsStreaming()` | Loads sub-levels into memory (invisible) | SceneManager itself at BeginPlay |
| `UnloadLevelsStreaming()` | Removes sub-levels from memory | Not used in the standard flow |
| `ShowStreamingLevels()` | Makes already-loaded sub-levels visible | SceneManager, Checkpoint |
| `HideStreamingLevels()` | Hides sub-levels (they remain in memory) | SceneManager, Checkpoint |

**Key concept: Runtime Visible vs. Saved State**

The system maintains two separate arrays:

- **`RuntimeVisibleLevels`** — Which levels are currently visible. Changes every time the player crosses a checkpoint (even non-saving ones). **Not saved to disk.**
- **`StreamingLevelsLoadedFromSave`** — The saved "snapshot" of visible levels. Updated **only** by saving checkpoints. This is the state the game reverts to after death.

### 3.5 Character Spawning

`SpawnCharacter()` determines the spawn position with this priority:

1. **`PlayerStartLocation` / `PlayerStartRotation`** (from save) — if non-zero
2. **`PlayerStart` actor** — if assigned in the Editor
3. **World origin** (fallback with a warning in the log)

After spawning:
- The `PlayerController` possesses the character (`PC->Possess(...)`)
- The SceneManager registers to the character's delegates:
  - `OnInitializedFinished` → hides the loading screen
  - `OnDeath` → starts the death/respawn sequence

### 3.6 Death and Respawn

```
Character dies
   │
   ├── OnDeath broadcast by the character
   │
   ├── [wait DeathToScreenDelay seconds]
   │
   ├── ShowDeathScreen()
   │     └── Broadcast: OnDeathScreenShow
   │
   ├── [wait ScreenToRespawnDelay seconds]
   │
   └── RespawnCharacter()
         ├── Broadcast: OnLoadingScreenShow
         ├── Broadcast: OnDeathScreenHide
         ├── RestoreStreamingLevels()     ← restores SAVED levels
         ├── ResetCharacter()
         │     ├── Teleports to saved position
         │     └── Calls Revive() on the character
         └── Waits for OnInitializedFinished → hides the loading screen
```

**Important for Designers:** On death, the player returns to the state of the **last saving checkpoint**. Non-saving checkpoints are "forgotten".

### 3.7 Save System Integration

The SceneManager participates in the Save System through:

1. **`USaveIdComponent`** — Provides a stable GUID to identify the actor during serialization.
2. **`ISavable` interface** — The `OnPostLoad_Implementation()` callback is invoked after save loading, setting `bSaveLoaded = true`.
3. **`SaveGame` properties** — The following are serialized automatically:
   - `StreamingLevelsLoadedFromSave`
   - `PlayerStartLocation`
   - `PlayerStartRotation`
   - `CurrentCheckpointPriority`

The actual save to disk happens in `TrySaveGame()`, which calls `SaveSubsystem->RequestSaveSlotAsync(...)`.

---

## 4. Checkpoint System

### 4.1 ACheckpointBase — Abstract Class

**Files:** `Public/Flow/CheckpointBase.h` | `Private/Flow/CheckpointBase.cpp`

This is the base class for every checkpoint type. **It cannot be placed directly** (`UCLASS(Abstract)`).

**Configurable properties:**

| Property | Type | Description |
|---|---|---|
| `SceneManager` | `ASceneManager*` | Reference to the scene's SceneManager. **Required.** |
| `LevelsToShow` | `TArray<FName>` | Sub-levels to make **visible** when the checkpoint is activated (forward traversal). |
| `LevelsToHide` | `TArray<FName>` | Sub-levels to **hide** when the checkpoint is activated. |
| `bCallSaveOnActivate` | `bool` | If `true`, this checkpoint **saves progress** to disk (position, levels, priority). |
| `CheckpointPriority` | `int32` | Numeric value representing progression. **Higher values = further in the game.** The save only happens if the priority is greater than the last saved checkpoint. |

**Components:**

- **`PlayerNewStartPoint`** (`UArrowComponent`) — A green arrow visible only in the Editor. It marks where the character will respawn if they die after activating this checkpoint. Its **position and rotation** are saved.

**Behaviour:**

| Virtual method | When | What it does |
|---|---|---|
| `OnCheckpointActivated()` | The player enters the trigger going **forward** | Changes level visibility and (if saving) saves |
| `OnCheckpointReversed()` | The player exits the trigger going **backward** | Reverses visibility (shows what was hidden, hides what was shown) |

### 4.2 ACheckpointTrigger — Box Trigger

**Files:** `Public/Flow/CheckpointTrigger.h` | `Private/Flow/CheckpointTrigger.cpp`

Concrete implementation of `ACheckpointBase` that uses a `UBoxComponent` as a trigger.

**How it works:**

1. When the character **enters** the box → saves the entry location (`EntryLocation`) and calls `OnCheckpointActivated()`.
2. When the character **exits** the box → calculates the traversal direction:
   - Uses the **dot product** between the movement vector and the actor's `ForwardVector`.
   - If the player exited on the same side they entered from (backward), calls `OnCheckpointReversed()`.

> **For Designers:** The **actor's arrow** (X-axis Forward) indicates the "forward" direction. Place the checkpoint so that the arrow points in the player's progression direction.

### 4.3 Priority and Direction

The system uses **priorities** to determine whether a checkpoint can overwrite the save:

```
Checkpoint A (Priority 0) → Checkpoint B (Priority 1) → Checkpoint C (Priority 2)
```

- The player activates A → saves with Priority 0
- The player activates B → `CanActivateCheckpoint(1)` → `1 > 0` → ✅ saves
- The player dies, returns to B
- The player goes back and re-crosses A → `CanActivateCheckpoint(0)` → `0 > 1` → ❌ **does not save** (but level visibility still changes)

**Rule:** A checkpoint with `bCallSaveOnActivate = true` saves **only** if its priority is **strictly greater** than the current saved priority.

### 4.4 Saving vs. Non-Saving

| Scenario | `bCallSaveOnActivate` | Valid priority | Effect |
|---|---|---|---|
| Saving checkpoint, going forward | ✅ | ✅ | `UpdateSavedState()` → changes visibility + saves position/levels/priority to disk |
| Saving checkpoint, going backward | ✅ | ❌ | `UpdateRuntimeVisibility()` → changes only runtime visibility (no save) |
| Non-saving checkpoint | ❌ | N/A | `UpdateRuntimeVisibility()` → changes only runtime visibility |

**On death:** The system **always** restores the state of the last save (not the runtime state). Levels shown/hidden by non-saving checkpoints are "forgotten".

---

## 5. Designer Guide — Placing Checkpoints in the Level

### Step 1: Set Up the SceneManager

1. Place **one single `ASceneManager`** in the persistent level.
2. Configure:
   - **`LevelsStreamingToLoad`** — List **all** sub-levels that the player might traverse.
   - **`StartingStreamingLevelsToShow`** — Only the levels visible at the start of a new game.
   - **`PlayerCharacterClass`** — Select the character class (e.g. `BP_MainCharacter`).
   - **`PlayerStart`** — Drag an actor into the level that marks the initial spawn point.
3. Make sure the `SaveIdComponent` has a **generated ID** (click "Generate ID" in the Details panel if it is empty).

### Step 2: Place the Checkpoints

1. Place an `ACheckpointTrigger` at the desired location.
2. **Orient the actor** so that the Forward arrow points in the player's progression direction.
3. Scale the **TriggerBox** to cover the passage.
4. Configure:
   - **`SceneManager`** → select the level's SceneManager.
   - **`LevelsToShow`** → sub-levels to make visible when the player crosses.
   - **`LevelsToHide`** → sub-levels to hide when the player crosses.
   - **`bCallSaveOnActivate`** → `true` if you want this checkpoint to save progress.
   - **`CheckpointPriority`** → an increasing number along the player's path (e.g. 0, 1, 2, 3...).
5. Position the **green arrow `PlayerNewStartPoint`** exactly where you want the player to respawn if they die after this checkpoint. Make sure the rotation is correct.

### Step 3: Test

- Use `bBypassSaveGame = true` on the SceneManager to test without saved data.
- Verify that:
  - Levels show/hide correctly when crossing checkpoints.
  - Going backward, visibility reverts.
  - On death, the character returns to the correct point with the right levels.

### Quick Checklist

- [ ] SceneManager placed in the persistent level
- [ ] `SaveIdComponent` has a generated ID
- [ ] `LevelsStreamingToLoad` contains all sub-levels
- [ ] `StartingStreamingLevelsToShow` contains the initial levels
- [ ] `PlayerCharacterClass` assigned
- [ ] `PlayerStart` assigned
- [ ] Every checkpoint has `SceneManager` referenced
- [ ] Checkpoint priorities are increasing
- [ ] `PlayerNewStartPoint` positioned and rotated correctly for every saving checkpoint
- [ ] Names in `LevelsToShow` / `LevelsToHide` match sub-level names **exactly**

---

## 6. Programmer Guide — Extending the System

### Creating a New Checkpoint Type

If you need a checkpoint with a different trigger (e.g. sphere, line-trace based, etc.):

1. Create a new class derived from `ACheckpointBase`.
2. Implement the player detection logic.
3. Call `OnCheckpointActivated()` when the player "activates" the checkpoint.
4. Call `OnCheckpointReversed()` when the player goes backward.

```cpp
UCLASS()
class ACheckpointSphere : public ACheckpointBase
{
    GENERATED_BODY()
    
    UPROPERTY(VisibleAnywhere)
    USphereComponent* TriggerSphere;
    
    // ... custom overlap logic ...
    // Call OnCheckpointActivated() and OnCheckpointReversed()
};
```

### Main SceneManager APIs

```cpp
// Changes only runtime visibility (no save)
void UpdateRuntimeVisibility(
    const TArray<FName>& LevelsToShow, 
    const TArray<FName>& LevelsToHide
);

// Changes visibility + saves everything to disk
void UpdateSavedState(
    const TArray<FName>& LevelsToShow,
    const TArray<FName>& LevelsToHide,
    const FVector& Location,
    const FRotator& Rotation,
    int32 Priority
);

// Checks whether a checkpoint can save (strictly greater priority)
bool CanActivateCheckpoint(int32 Priority) const;

// Low-level streaming functions
void LoadLevelsStreaming(const TArray<FName>& Levels);
void UnloadLevelsStreaming(const TArray<FName>& Levels);
void ShowStreamingLevels(const TArray<FName>& Levels);
void HideStreamingLevels(const TArray<FName>& Levels);
```

### Available UI Delegates (BlueprintAssignable)

| Delegate | When it fires |
|---|---|
| `OnLoadingScreenShow` | At game start and before respawn |
| `OnLoadingScreenHide` | When the character has finished initializing |
| `OnDeathScreenShow` | After `DeathToScreenDelay` seconds from death |
| `OnDeathScreenHide` | Just before respawn (behind the loading screen) |

### SaveGame Properties (automatically serialized)

If you add new properties to the SceneManager that need to be persisted, mark them with `UPROPERTY(SaveGame)`:

```cpp
UPROPERTY(SaveGame)
int32 MyNewSavedProperty;
```

The `SaveSubsystem` will serialize them automatically thanks to the `SaveIdComponent`.

### Technical Notes

- The SceneManager does **not** use the `GameMode` for spawning — it creates the character directly with `SpawnActor` and calls `Possess` manually. This is intentional to have full control over the initialization sequence.
- The `RuntimeVisibleLevels` vs `StreamingLevelsLoadedFromSave` system allows "lightweight" (non-saving) checkpoints that change the scene during gameplay without polluting the save data.
- Checkpoint reversal works via dot product with the actor's forward vector — it is critical that checkpoints are oriented correctly.

---

## 7. Flow Diagrams

### Scene Boot

```
┌─────────────┐
│  BeginPlay  │
└──────┬──────┘
       │
       ├──────────────────────────────────────┐
       │                                      │
       ▼                                      ▼
┌──────────────────┐               ┌─────────────────────┐
│  Load Streaming  │               │  Save System loads   │
│  Levels (hidden) │               │  SaveGame properties │
└───────┬──────────┘               └──────────┬──────────┘
        │                                     │
        ▼                                     ▼
  bLevelsReady=true                    bSaveLoaded=true
        │                                     │
        └──────────┬──────────────────────────┘
                   ▼
         TryInitializeScene()
                   │
          ┌────────┴─────────┐
          ▼                  ▼
    [Save data]        [No save data]
          │                  │
          ▼                  ▼
  Show saved levels    Show default levels
  Spawn at saved pos   Spawn at PlayerStart
          │                  │
          └────────┬─────────┘
                   ▼
        Character Initialized
                   │
                   ▼
       OnLoadingScreenHide
```

### Death/Respawn Cycle

```
┌────────────────┐
│ Character Dies │
└───────┬────────┘
        │
        ▼
  [Wait DeathToScreenDelay]
        │
        ▼
  OnDeathScreenShow
        │
        ▼
  [Wait ScreenToRespawnDelay]
        │
        ▼
  OnLoadingScreenShow
  OnDeathScreenHide
        │
        ├── RestoreStreamingLevels()  ← reverts to SAVED state
        ├── ResetCharacter()          ← teleport + Revive()
        │
        ▼
  Character Re-Initialized
        │
        ▼
  OnLoadingScreenHide
```

### Checkpoint Traversal

```
         Player enters the trigger
                    │
                    ▼
           OnCheckpointActivated()
                    │
            ┌───────┴────────┐
            │                │
   bCallSaveOnActivate    bCallSaveOnActivate
       = true                = false
            │                │
            ▼                │
  CanActivateCheckpoint?     │
     ┌──────┴──────┐        │
     ▼             ▼        │
   [YES]         [NO]       │
     │             │        │
     ▼             ▼        ▼
UpdateSavedState  UpdateRuntimeVisibility
  (save + show)     (visibility only)
     │             │        │
     └─────────────┴────────┘
                    │
                    ▼
          Player exits the trigger
                    │
                    ▼
            Exit direction?
           ┌────────┴────────┐
           ▼                 ▼
       [Forward]         [Backward]
       (nothing)     OnCheckpointReversed()
                     Reverses visibility
```

---

## 8. FAQ and Troubleshooting

**Q: The character spawns at the world origin.**  
A: Verify that `PlayerStart` is assigned in the SceneManager and that there is no corrupted save with `PlayerStartLocation = (0,0,0)`.

**Q: Levels don't change when I cross a checkpoint.**  
A: Check that:
- The `SceneManager` reference in the checkpoint is assigned.
- The names in `LevelsToShow` / `LevelsToHide` match the sub-level names **exactly** (case-sensitive).
- The levels are present in the SceneManager's `LevelsStreamingToLoad`.

**Q: The checkpoint doesn't save even though `bCallSaveOnActivate = true`.**  
A: The checkpoint's priority must be **strictly greater** than `CurrentCheckpointPriority`. If you have already activated a checkpoint with equal or higher priority, the save is skipped. Check your `CheckpointPriority` values.

**Q: After death, the player returns to a different checkpoint than expected.**  
A: On death the system restores the state of the last **saving checkpoint**. If the player crossed non-saving checkpoints after the last save, those changes are lost. This is intended behaviour.

**Q: Going backward through a checkpoint, the levels don't revert.**  
A: The reverse only works if the player exited the trigger from the **same side** they entered. Verify that the checkpoint's orientation (Forward axis) is correct relative to the gameplay direction.

**Q: The loading screen never disappears.**  
A: The loading screen is hidden when the character fires `OnInitializedFinished`. Verify that your `BP_MainCharacter` correctly broadcasts this delegate at the end of its initialization.

**Q: I want to test a scene without a save.**  
A: Set `bBypassSaveGame = true` on the SceneManager. This way the system won't wait for the save to load and will use default values.

