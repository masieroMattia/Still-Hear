# Still Hear - Save System (UE 5.5)

This document  module and explains responsibilities, data flow, and how the parts interact.

## `SaveSubsystem.h`

### Purpose
Central manager for saving/loading game state and settings. It:
- Builds a `USaveGameObject` from currently loaded levels/actors.
- Saves it to a slot \(async\).
- Loads from a slot and applies to the current world.
- Saves/loads `USettingsSaveGame` separately.

### Key type
- `USaveSubsystem` \(`UGameInstanceSubsystem`\)

### Events \(Blueprint\-assignable\)
Gameplay saves:
- `OnSaveStarted`
- `OnSaveFinished(bool bSuccess)`

Settings saves:
- `OnSettingsSaveStarted`
- `OnSettingsSaveFinished(bool bSuccess)`

### Lifecycle
- `Initialize(...)`: intended for binding hooks to map loading and level streaming.
- `Deinitialize()`: unbind and cleanup.

### Stable key helpers
- `GetStableMapKey(const UWorld* World)`
- `GetStableLevelKey(const UWorld* World, const ULevel* Level)`

These provide `FName` keys used to store and later resolve `FLevelSaveData` entries.

### Save / load API
- `IsSaving()`: indicates whether a gameplay save is in progress.
- `RequestSaveSlotAsync(int32 SlotIndex)`: starts an async save request for a slot.
- `SaveToSlot(int32 SlotIndex, int32 UserIndex = 0)`: sync save function \(marked as not used currently\).
- `LoadFromSlot(int32 SlotIndex, int32 UserIndex = 0)`: loads a save from disk.
- `ApplyLoadedDataToCurrentlyLoadedLevels()`: manually applies the currently loaded save data to already loaded levels.
- `GetCurrentSlotSave()`: returns the active slot index \(1\-3\) or \-1.
- `DebugPrintCurrentSave()`: returns a debug string describing the current save.

### Settings API
- `SaveSettingsAsync()`: async save for settings.
- `LoadSettings()`: loads settings.
- `GetSettings()`: returns the current settings object.
- `GetSaveSettings()`: inline accessor returning the same settings pointer.

### Internals \(high level\)
Gameplay:
- `CurrentSave`: currently loaded save object.
- `PendingSaveObject`: kept alive during async save.
- `MakeSlotName(SlotIndex)`: builds `Save_SlotN`.
- Hooks:
  - `OnPostLoadMap(...)`
  - `OnLevelAddedToWorld(...)`
- Serialization helpers:
  - `SerializeObject(UObject*, TArray<uint8>&)`
  - `DeserializeObject(UObject*, const TArray<uint8>&)`
- Save building:
  - `BuildSaveObject(...)`: collects levels/actors into the save object.
  - `SaveLevelInto(...)`, `SaveActorInto(...)`
- Actor identity:
  - `TryGetActorGuid(...)`: extracts stable GUID \(typically from `USaveIdComponent`\).
- Async completion:
  - `HandleAsyncSaveComplete(...)`

Settings:
- `Settings`, `PendingSettingsSave`, `bSettingsSaveInProgress`
- `SettingsSlotName()`: returns `Settings`
- `HandleAsyncSettingsSaveComplete(...)`

### Expected save flow \(conceptual\)
1. `RequestSaveSlotAsync(slot)` starts, sets flags, broadcasts `OnSaveStarted`.
2. `BuildSaveObject` traverses levels and actors:
   - Identify actors via `USaveIdComponent`.
   - Optionally call `ISavable::OnPreSave()`.
   - Serialize with the configured archive into `FActorSaveData::Bytes`.
3. Engine async save completes; `HandleAsyncSaveComplete` broadcasts `OnSaveFinished(bSuccess)`.

### Expected load flow \(conceptual\)
1. `LoadFromSlot(slot)` loads `USaveGameObject`.
2. For each saved level key, find matching loaded level via stable key.
3. For each actor GUID, find actor in level/world, deserialize bytes, then optionally call `ISavable::OnPostLoad()`.


## `Savable.h`

### Purpose
Defines an Unreal Engine interface for objects that need custom logic **before saving** and **after loading**.

### Key types
- `USavable` \(`UInterface`\): the Unreal reflection wrapper that enables Blueprint exposure.
- `ISavable` \(`interface`\): the C\+\+ interface that provides the hooks.

### Public API
- `OnPreSave()`  
  Called right before serialization. Use it to prepare state that must be persisted.
- `OnPostLoad()`  
  Called right after deserialization. Use it to rebuild runtime\-only state \(UI refresh, dynamic materials, animation state, etc\.\).

### Notes
- Marked as `BlueprintNativeEvent`, so you can implement it in either C\+\+ or Blueprint.
- Intended to be invoked by the subsystem during save/load traversal.

---

## `SaveTypes.h`

### Purpose
Provides the lightweight data structs used inside save containers to represent **per\-actor** and **per\-level** saved state.

### Key types
- `FActorSaveData`  
  Stores:
  - `Transform`: the actor transform at save time.
  - `Bytes`: raw serialized property bytes for the actor marked as `UPROPERTY(SaveGame)` 

- `FLevelSaveData`  
  Stores:
  - `Actors`: `TMap<FGuid, FActorSaveData>` mapping a stable actor GUID to its saved bytes and transform.

### Notes
- `FGuid` keys require a stable identity system \(provided by `USaveIdComponent`\).
- `Bytes` format depends on the archive used \(see `FSaveCoreArchive`\).

---

## `SaveGameObject.h`

### Purpose
Defines the main gameplay save container stored on disk for a slot. It aggregates all saved levels and actors.

### Key type
- `USaveGameObject` \(`USaveGame`\)

### Stored data
- `SaveVersion`: integer version for future migrations.
- `Levels`: `TMap<FName, FLevelSaveData>` where the key is a stable map/level identifier.
- `SlotName`: optional metadata about which slot produced the save.

### Notes
- Settings are intentionally not stored here; they live in `USettingsSaveGame`.
- `Levels` keys are expected to be generated via stable key helpers in the subsystem.

---

## `SettingsSaveGame.h`

### Purpose
Defines a separate save container dedicated to **user settings**, typically saved to a dedicated slot \(e\.g\. `Settings`\).

### Key type
- `USettingsSaveGame` \(`USaveGame`\)

### Stored settings \(audio\)
- `MasterVolume`
- `MusicVolume`
- `SfxVolume`
- `VoiceVolume`
- `AmbienceVolume`

### Notes
- This is a data\-only class; saving/loading is handled by the manager \(see `USaveSubsystem`\).
- Validation \(clamping ranges, applying to sound classes/mix\) should be performed by the system that consumes these values.

---


## `SaveIdComponent.h`

### Purpose
Provides a stable unique identifier \(GUID\) on actors so they can be reliably matched between save files and the current world.

### Key type
- `USaveIdComponent` \(`UActorComponent`\)

### Public API
- `GetSaveId()`: returns the component’s GUID.
- `GenerateID()` \(`CallInEditor`\): creates an ID in the editor.
- `RegenerateID()` \(`CallInEditor`\): replaces the ID \(breaks existing saves for that actor\).

### Editor behavior
- `PostEditImport()` \(`WITH_EDITOR`\): ensures IDs behave correctly when duplicating/pasting/importing actors.

### Internal behavior
- `EnsureId(bool bForceNew)`: generates/validates the GUID as needed.
- The component stores:
  - `SaveId`: the actual GUID.
  - `bNull`: marker used to detect initial creation vs loaded state.

### Notes / limitations
- Actors are only considered for save/load if they own this component \(as stated in comments\).
- The first time an actor is created, the ID may require manual generation via the editor button.
- If actor has the component but the current GUID is invalid (All 0) save system ignore it, so remenber to ceck ID.

---

## `FSaveCoreArchive.h`

### Purpose
Defines a custom serialization archive configured specifically for gameplay saves.

### Key type
- `FSaveCoreArchive` \(`FObjectAndNameAsStringProxyArchive`\)

### Behavior
- Sets `ArIsSaveGame = true`  
  Unreal serializes only fields marked with `UPROPERTY(SaveGame)`.
- Sets `ArNoDelta = true`  
  Forces serialization even when values match defaults \(useful to restore exact state and avoid delta issues\).

### Typical usage
Used by subsystem serialization helpers \(e\.g\. `SerializeObject` / `DeserializeObject`\) to convert a `UObject` into bytes and back, storing the bytes in `FActorSaveData::Bytes`.


# Additional Information

In the Editor, there are some useful **widgets**.

### EUW_SaveDebug
This is a debug window used to **read, save, load, and delete save games**.

You can find it in **SaveSystem/Debug**.  
To open it, right-click the asset and select **Run Editor Utility Widget**.

---

### WBP_Loading
This widget **displays a “saving” message** in the bottom-right corner of the screen.

It is an **example of how to hook UI into the SaveSystem**.  
If you need to create a new widget, **use this as a reference**.

You can find it in **SaveSystem/UI**.

---

### WBP_AudioSettings
This widget **shows how to modify settings (audio only, for now)**.

It is provided as an **example**.  
Feel free to modify or extend it as needed.

You can find it in **Audio/UI**.
