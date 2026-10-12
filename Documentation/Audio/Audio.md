
# StillHear — Audio System (UE 5.5)

  

This document describes the current audio architecture and how to extend/debug it.

## Goals

  

- Centralized, persistent audio control across level transitions (music + user volumes).

- Distance-based “world dynamics” driven by Companion ↔ Main character distance:

- World volume mapping across 4 states (Inside, 0..A, A..B, B..C)

- Optional “noise” that ramps up from B → C

- “Muffle” (low-pass) when the companion is inside the main character

- No external middleware.

  

---

  

## High-level Architecture

  

### 1) `UGameAudioSubsystem` (GameInstanceSubsystem)

**Persistent** across level transitions.

  

Responsibilities:

- Load audio references from **Developer Settings**.

- Ensure a **Runtime SoundMix** is active for the current game world.

- Apply **user volume sliders** using `SetSoundMixClassOverride` (runtime-only, no asset dirtying).

- Manage **music** via a persistent `UAudioComponent` (2D), including crossfade.

  

### 2) `UCompanionDistanceAudioComponent` (ActorComponent on PlayerController)

**Not persistent** (belongs to the current level’s PlayerController), updates using a **Timer** (no Tick).

  

Responsibilities:

- Read distance from `AStillHearPlayerController::GetCompanionDistance()`.

- Convert distance into target values:

- world volume target

- noise volume target

- muffle enabled/disabled

- Apply results:

- world dynamics volume via SoundMix override (through subsystem)

- noise via an internal `UAudioComponent` (volume multiplier)

- muffle via Submix effect chain override

  

---

  

## Unreal Concepts Used

  

### SoundClass

- Logical grouping (Master/SFX/Music/UI/Voice…)

- Hierarchical multiplication (child volumes are multiplied by parents).

- **Do not** change `SoundClass->Properties.Volume` at runtime (would dirty assets).

  

### SoundMix

- Runtime layer that overrides SoundClass volumes/pitch without touching assets.

- The subsystem uses a dedicated mix: `SMX_RunTimeSettings`.

  

### Submix

- Routing / processing bus.

- Used for “muffle” by applying a LowPass preset to the `SM_World` submix chain via override.

  

---

  

## Required Assets & Setup

  

### A) Sound Classes (User-facing sliders)

Actual hierarchy:

- `SC_Master`

- `SC_Music_User`

- `SC_Music`

- `SC_SFX_User`

- `SC_SFX`

- `SC_WorldDynimic`

- `SC_Noise`

- `SC_UI_User`

- `SC_UI`

- `SC_Voice`

- `SC_Ambience`

  

  

### B) Runtime SoundMix (user settings)

  

- `SMX_RunTimeSettings`

  

The subsystem sets it as:

- Base SoundMix + pushed modifier, on the active game world.

  

### C) World Submix + Muffle preset

  

- Submix: `SM_World`

- Preset: `LPF_Preset_On` (LowPass / Filter preset)

  

**Routing requirement:**

Sounds that must be muffled should route to `SM_World` (via asset settings or a consistent routing strategy).

  

### D) Distance tuning DataAsset

`UAudioDistanceTuningData` controls thresholds & behavior.

  

Core fields:

- A, B, C

- WorldMax, WorldMin

- NoiseSound, NoiseAtB, NoiseAtC

- SM_World, LPF_Preset_On, MuffleFadeTime

- UpdateInterval, interp speeds, epsilon

  

---

  

## Companion Distance Logic

  

The PlayerController exposes:

- `GetCompanionDistance()` returning:

- `-1` when companion is **inside**

- otherwise a positive distance

  

### States

  

Let `Dist` be the returned value.

  

1) **Inside** (`Dist < 0`)

- Muffle: **ON**

- World volume: `WorldMax`

- Noise: `0`

  

2) **0..A**

- Muffle: OFF

- World volume: `WorldMax`

- Noise: `0`

  

3) **A..B**

- Muffle: OFF

- World volume: lerp `WorldMax → WorldMin` as `Dist` approaches B

- Noise: `0`

  

4) **B..C**

- Muffle: OFF

- World volume: `WorldMin`

- Noise: lerp `NoiseAtB → NoiseAtC` as `Dist` approaches C

  

---

  

## `UGameAudioSubsystem` Details

  

### World binding (important)

SoundMix overrides must be applied to the correct **game world/audio device**.

  

The subsystem:

- subscribes to world creation via `FWorldDelegates::OnPostWorldCreation`

- stores the active game world (`CurrentWorld`)

- applies the RuntimeMix on **next tick** when a game world is ready

  

This timing matters: applying the mix too early can fail silently.

  

### User volume sliders (no asset dirtying)

The subsystem applies volumes via:

- `UGameplayStatics::SetSoundMixClassOverride(World, RuntimeMix, SoundClass, Volume, Pitch, FadeTime, bApplyToChildren)`

  

This affects runtime output only.

  

### Music

- Uses a persistent `UAudioComponent` spawned with `SpawnSound2D`.

- Keeps playing across level changes as long as the subsystem survives.

- Ensures the component belongs to the current world when needed.

  

---

  

## `UCompanionDistanceAudioComponent` Details

  

- Attached to **PlayerController** (owner).

- Uses a Timer to call `UpdateAudio()` every `UpdateInterval`.

  

Applies:

- World volume target → via subsystem SoundMix override.

- Noise volume target → internal `NoiseComp` volume multiplier and fade.

- Muffle toggle → Submix effect chain override (AudioMixer).

  

**Muffle**

Use:

- `UAudioMixerBlueprintLibrary::SetSubmixEffectChainOverride`

- `UAudioMixerBlueprintLibrary::ClearSubmixEffectChainOverride`

  

> Ensure the `AudioMixer` module is in the project `.Build.cs`.

  

---

  

## Debugging & Verification

  

### 1) Verify SoundMix is active

- The most reliable approach is logging in C++ when calling:

- `SetBaseSoundMix`, `PushSoundMixModifier`

- `SetSoundMixClassOverride`

  

### 2) Verify runtime overrides (do not trust asset values)

SoundClass assets stay at 1.0; runtime overrides won’t appear by inspecting the asset.

  

Use:

- **Audio Insights** (recommended) to inspect active mixes, classes and sources at runtime.

- Console:

- `au.Debug.SoundMixes 1`

- `au.Debug.Sounds 1`

- `au.DumpActiveSounds` (Output Log)

  

> Command availability can differ per build. Use console autocomplete with `au.`.
