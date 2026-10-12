
# StillHear — Code Technical Documentation

> Unreal Engine 5 project — C++ module `StillHear`.
> Dual-audience documentation:
> - 🔧 **Developer** — logic, flows, classes and functions to edit in C++.
> - 🎛️ **Designer** — variables exposed in the editor (Blueprint / Data Asset) and how to use them.
>
> When a class is *data-driven*, the documentation points to its Data Asset;
> otherwise it lists the exposed variables (`UPROPERTY`) directly.

---


## Table of Contents

### [Phase 0 — Overall Architecture](#phase-0)
- [0.1 What StillHear is](#01-what-stillhear-is-from-the-codes-perspective)
- [0.2 Module dependencies](#02-module-dependencies-stillhearbuildcs)
- [0.3 Folder organization](#03-folder-organization)
- [0.4 Recurring code conventions](#04-recurring-code-conventions)

### [1 — Game Core](#phase-1)
- [1.1 `AStillHearGameMode`](#11-astillheargamemode)
- [1.2 `AStillHearCharacterBase`](#12-astillhearcharacterbase)
- [1.3 `AStillHearPlayerController`](#13-astillhearplayercontroller)
- [1.4 `AStillHearMainCharacter`](#14-astillhearmaincharacter)

### [2 — Input](#phase-2)
- [2.1 Data assets & supporting types](#21-data-assets--supporting-types)
- [2.2 `UInputSubsystem` — lifecycle & catalog](#22-uinputsubsystem--lifecycle--catalog)
- [2.3 Rebinding model](#23-rebinding-model-developer)
- [2.4 Query helpers for the UI](#24-query-helpers-for-the-ui-developer)

### [3 — Gameplay Ability System (Core)](#phase-3)
- [3.1 `UStillHearAbilitySystemComponent` (ASC)](#31-ustillhearabilitysystemcomponent-asc)
- [3.2 Attribute Sets](#32-attribute-sets)
- [3.3 `UStillHearGameplayAbility` (base ability)](#33-ustillheargameplayability-base-ability)
- [3.4 `UGE_GrantTagsToActor`](#34-uge_granttagstoactor-generic-gameplay-effect)
- [3.5 Native Gameplay Tags](#35-native-gameplay-tags)
- [3.6 `UAT_NavigateTo`](#36-uat_navigateto-reusable-ability-task)
- [3.7 Generic Gameplay Cue base classes](#37-generic-gameplay-cue-base-classes)

### [4 — Player Abilities](#phase-4)
- [4.0 Common ability pattern](#40-common-ability-pattern-developer)
- [4.1 `UGA_Sprint`](#41-uga_sprint)
- [4.2 `UGA_Jump`](#42-uga_jump)
- [4.3 `UGA_Crouch` (data-driven)](#43-uga_crouch--data-driven-ucrouchdata)
- [4.4 `UGA_Climb`](#44-uga_climb)
- [4.5 `UGA_Parry` (data-driven)](#45-uga_parry--data-driven-uparrydata)
- [4.6 `UGA_Resonance` (data-driven)](#46-uga_resonance--data-driven-uresonancedata)
- [4.7 `UGA_Death`](#47-uga_death-state-ability)
- [4.8 `UGA_LowVault` / `UGA_LowGetOnTop` (dormant)](#48-uga_lowvault--uga_lowgetontop--currently-dormant)

### [5 — Interactions](#phase-5)
- [5.1 Interfaces](#51-interfaces)
- [5.2 Interaction abilities](#52-interaction-abilities)
- [5.3 `AInteractableObj`](#53-ainteractableobj-base-interactable-actor)
- [5.4 Interactable actor catalog](#54-interactable-actor-catalog)
- [5.5 Trigger components](#55-trigger-components-ubasetriggercomponent-family)
- [5.6 Helper components](#56-helper-components)

### [6 — Camera](#phase-6)
- [6.1 `ICameraVolumesInteractor`](#61-icameravolumesinteractor-publicinterfacescameravolumesinteractorh)
- [6.2 `ACameraVolume`](#62-acameravolume-abstract-base)
- [6.3 Spline-follow cameras](#63-spline-follow-cameras)
- [6.4 Camera effects layer](#64-camera-effects-layer)
- [6.5 `AStillHearCamera` (legacy)](#65-astillhearcamera--legacy--unused)

### [7 — Companion](#phase-7)
- [7.1 `UFloatingCompanionComponent`](#71-ufloatingcompanioncomponent)
- [7.2 `UGA_SoundWave` (data-driven)](#72-uga_soundwave--data-driven-usoundwavedata)
- [7.3 `UTargetMarkerNiagaraComponent`](#73-utargetmarkerniagaracomponent)
- [7.4 `UCompanionInteractionSpotComponent`](#74-ucompanioninteractionspotcomponent)

### [8 — Audio](#phase-8)
- [8.1 `UGameAudioSubsystem`](#81-ugameaudiosubsystem)
- [8.2 State-driven audio](#82-state-driven-audio)
- [8.3 Settings & config](#83-settings--config)
- [8.4 `WorldStereoBalanceSubmix` (DSP)](#84-worldstereobalancesubmix-custom-dsp)
- [8.5 `UAudioDistanceTuningData`](#85-uaudiodistancetuningdata-data-asset)
- [8.6 `FSoundLevelDelegate`](#86-fsoundleveldelegate-publicaudiosounddelegatesh)
- [8.7 Footsteps — `UFootStepData`](#87-footsteps--ufootstepdata-data-asset)
- [8.8 Audio Flow nodes](#88-audio-flow-nodes)

### [9 — Animation](#phase-9)
- [9.1 `UMainCharacterAnimInstance`](#91-umaincharacteraniminstance)
- [9.2 AnimNotifies — footsteps & floor](#92-animnotifies--footsteps--floor)
- [9.3 AnimNotifies — AI noise](#93-animnotifies--ai-noise-stealth-loop)
- [9.4 AnimNotifies — GAS & hitboxes](#94-animnotifies--gas--hitboxes)

### [10 — Trace & Collision](#phase-10)
- [10.1 `ECustomCollision`](#101-ecustomcollision-publictraceandcollisioncustomcollisionh)
- [10.2 `ECustomSurface`](#102-ecustomsurface-publictraceandcollisioncustomsurfaceh)
- [10.3 `EFloorTypeEnum`](#103-efloortypeenum-publictraceandcollisionfloortypeenumh)

### [11 — Enemies AI](#phase-11)
- [11.0 Architecture at a glance](#110-architecture-at-a-glance)
- [11.1 Shared base](#111-shared-base)
- [11.2 Perception & awareness (Mantis meter)](#112-perception--awareness-mantis-meter)
- [11.3 The Mantis](#113-the-mantis)
- [11.4 The Worm](#114-the-worm-ground-sound-eater)
- [11.5 Shared ability — `UGA_Stun`](#115-shared-ability--uga_stun)
- [11.6 Behavior Tree library](#116-behavior-tree-library)
- [11.7 EQS contexts](#117-eqs-contexts-eqscontext)
- [11.8 Patrol & triggers](#118-patrol--triggers)
- [11.9 Enemy animation](#119-enemy-animation)

### [12 — Save System](#phase-12)
- [12.1 Identity & serialization primitives](#121-identity--serialization-primitives)
- [12.2 Save containers](#122-save-containers)
- [12.3 `USaveSubsystem` (orchestrator)](#123-usavesubsystem-orchestrator)

### [13 — Flow (Level Direction & Checkpoints)](#phase-13)
- [13.1 `ASceneManager`](#131-ascenemanager)
- [13.2 Checkpoints](#132-checkpoints)
- [13.3 Custom Flow nodes](#133-custom-flow-nodes)

### [14 — PCG (Procedural Generation along Splines)](#phase-14)
- [14.1 Spline actors](#141-spline-actors)
- [14.2 Custom PCG nodes](#142-custom-pcg-nodes)

### [15 — VFX & Weather](#phase-15)
- [15.1 VFX](#151-vfx)
- [15.2 Weather](#152-weather)

### [16 — UI](#phase-16)
- [16.0 Layer model](#160-layer-model)
- [16.1 Core layout & routing](#161-core-layout--routing)
- [16.2 Reusable elements](#162-reusable-elements-uielements)
- [16.3 Menus & settings](#163-menus--settings)
- [16.4 Controls rebinding](#164-controls-rebinding-widgetscontrols)
- [16.5 Popups & HUD widgets](#165-popups--hud-widgets)
- [16.6 World-anchored indicators](#166-world-anchored-indicators-uiindicator)
- [16.7 Loading screen](#167-loading-screen-uislate)
- [16.8 World text — `ATypewriterTextActor`](#168-world-text--atypewritertextactor-uitextrenderer)

### [17 — Infrastructure & Shared Data](#phase-17)
- [17.1 `UStillHearGameInstance`](#171-ustillheargameinstance)
- [17.2 `UTimeManagementSubsystem`](#172-utimemanagementsubsystem)
- [17.3 `UPSOBlueprintLibrary`](#173-upsoblueprintlibrary)
- [17.4 Project Settings](#174-project-settings-udevelopersettings)
- [17.5 Tools — `ATrajectoryPreview`](#175-tools--atrajectorypreview-editor)
- [17.6 Shared Data catalog](#176-shared-data-catalog)
- [17.7 Module boilerplate](#177-module-boilerplate)

---
<a id="phase-0"></a>
## 0 — Overall Architecture

<a id="01-what-stillhear-is-from-the-codes-perspective"></a>
### 0.1 What StillHear is (from the code's perspective)
StillHear is a third-person action/stealth game with a strong **audio** component
(sound-eater enemies, noise events, sound propagation) and a floating **companion**
that accompanies the main character. The architecture is built on:

- **Gameplay Ability System (GAS)** — every character action (jump, sprint, crouch,
  parry, resonance, sound wave, interactions, death) is a *Gameplay Ability*.
- **Enhanced Input** — input mapped through `InputMappingContext` / `InputAction`.
- **Volume-based camera** — the camera is driven by `ACameraVolume` actors placed in
  the level, with priority and blending, rather than a camera attached to the character.
- **Flow** (plugin) — level direction, cutscenes and sequences.
- **PCG** — procedural content generation along splines.
- **Save System / Subsystems** — saving, audio, UI and time management implemented as
  `Subsystem`s of the `GameInstance` / `LocalPlayer`.

<a id="02-module-dependencies-stillhearbuildcs"></a>
### 0.2 Module dependencies (`StillHear.Build.cs`)
| Category | Modules |
|---|---|
| Core UE | `Core`, `CoreUObject`, `Engine`, `InputCore`, `ApplicationCore` |
| Input | `EnhancedInput`, `InputDevice`, `CommonInput` |
| GAS | `GameplayAbilities`, `GameplayTasks`, `GameplayTags` |
| Camera | `EngineCameras` |
| AI / Nav | `AIModule`, `NavigationSystem` |
| Audio / DSP | `AudioMixer`, `SignalProcessing` |
| Animation / Movement | `AnimGraphRuntime`, `MotionWarping` |
| Physics / Chaos | `GeometryCollectionEngine`, `ChaosCaching`, `ChaosSolverEngine`, `PhysicsCore` |
| UI | `CommonUI`, `UMG`, `Slate`, `SlateCore` |
| Direction / Sequences | `Flow`, `LevelSequence`, `MovieScene` |
| VFX / Rendering | `Niagara`, `RenderCore`, `RHI` |
| Procedural | `PCG` |
| Settings | `DeveloperSettings` |
| Vendor (private) | `DLSSBlueprint`, `StreamlineDLSSGBlueprint`, `StreamlineReflexBlueprint` |
| Editor-only | `UnrealEd`, `PropertyEditor`, `EditorScriptingUtilities`, `PCGEditor`, `LevelEditor`, `EditorStyle` |

<a id="03-folder-organization"></a>
### 0.3 Folder organization
The code follows the UE **Public/Private** convention:
- `Public/` → headers (`.h`) visible to other modules.
- `Private/` → implementations (`.cpp`) and internal headers.

Top-level subsystems (by descending file count):

| Subsystem | Role |
|---|---|
| `UI/` | HUD, menus, CommonUI widgets, indicators, gamepad/keyboard controls |
| `EnemiesAI/` | Enemies (Worm, Mantis): Pawn, Controller, Behavior Tree, EQS, NavLink, Patrol |
| `GameplayAbilitySystem/` | ASC, Attributes, Abilities, Effects, Cues, Tasks, Tags |
| `Interactions/` | Interactable objects, rails, push spots, target spots |
| `Data/` | Configuration Data Assets and Data Tables |
| `Flow/` | Direction/narrative nodes |
| `Audio/` | Balance submix, distance tuning, audio config |
| `Animation/` | AnimInstances and AnimNotifies |
| `Character/` | Character hierarchy + components (this phase) |
| `Camera/` | Camera types, effects, volumes |
| `PCG/` | Procedural generation along splines |
| `SaveSystem/` | Saving/loading, archives |
| `Input/` | Controller data, mapping contexts, device |
| `Weather/`, `VFX/`, `TraceAndCollision/` | World-support systems |
| `Subsystems/`, `FunctionLibrary/`, `Tools/`, `ProjectSettings/` | Infrastructure |

<a id="04-recurring-code-conventions"></a>
### 0.4 Recurring code conventions
- **Region pragmas**: headers are split into `#pragma region UPROPERTY / VARIABLES /
  CONSTRUCTOR / UFUNCTIONS / METHODS` for readability.
- **Communication via Gameplay Events**: the code rarely calls abilities directly;
  it usually sends an `FGameplayEventData` carrying a `GameplayTag`
  (`SendGameplayEventToSelf`) that activates/stops the matching ability.
- **Native tags**: tags are declared in C++ (`GameplayAbilitySystem/Tags/GameplayTags.h`)
  and referenced as `TAG_...` constants instead of by string where possible.

---
<a id="phase-1"></a>
## 1 — Game Core

Hierarchy of the main classes:

<a id="11-astillheargamemode"></a>
### 1.1 `AStillHearGameMode`
**Files:** `Public/StillHearGameMode.h`, `Private/StillHearGameMode.cpp`

Minimal GameMode: derives from `AGameModeBase` with no overrides. All configuration
(Default Pawn, Player Controller, HUD) happens in the derived **Blueprint** and in the
*World/Project Settings*.

---

<a id="12-astillhearcharacterbase"></a>
### 1.2 `AStillHearCharacterBase`
**Files:** `Public/Character/StillHearCharacterBase.h`, `Private/Character/StillHearCharacterBase.cpp`

Base class for **all** GAS-enabled characters (player and companion).
Implements `IAbilitySystemInterface`.

#### What it does
- Creates the `AbilitySystemComponent` (of type `UStillHearAbilitySystemComponent`) in
  the constructor.
- Sets the default movement: `bOrientRotationToMovement = true`, controller rotation
  disabled on pitch/yaw/roll (the character faces its movement direction, not the
  camera).
- In `PossessedBy` it initializes `AbilityActorInfo` and grants the `StartingAbilities`
  **only once** (the `bStartingAbilitiesGranted` flag prevents duplicates if an AI
  controller re-possesses the pawn — otherwise the `GameplayEvent`s would fire twice).
- Reacts to `MovementMode` changes: adds/removes the `Status.Falling` tag when it
  enters/leaves the falling state.
- On `OnEndCrouch` it sends the `Event.MainCharacter.EndCrouch` gameplay event to GAS.

#### Key API (developer)
| Function | Purpose |
|---|---|
| `GetAbilitySystemComponent()` | `IAbilitySystemInterface` implementation; returns the ASC. |
| `GrantAbilities(TArray<TSubclassOf<UGameplayAbility>>)` | Grants a set of abilities at level 1, returns their `FGameplayAbilitySpecHandle`s, then broadcasts `Event.Abilities.Changed`. |
| `RemoveAbilities(TArray<FGameplayAbilitySpecHandle>)` | Clears the given abilities, then broadcasts `Event.Abilities.Changed`. |
| `SendAbilitiesChangedEvent()` | Sends the `Event.Abilities.Changed` gameplay event to self (used by UI to refresh the available-ability list). |
| `SendGameplayEventToSelf(FGameplayTag)` | Helper that builds an `FGameplayEventData` (Instigator = Target = self) and dispatches it. The backbone of the event-driven communication pattern. |
| `HandleDeath(FGameplayTag, int32)` | Virtual hook, empty here; overridden by subclasses. |

#### Exposed variables
| Variable | Spec | Meaning |
|---|---|---|
| `AbilitySystemComponent` | `VisibleAnywhere, BlueprintReadOnly` | The ASC subobject. Read-only — wired in the constructor. |
| `BaseSpeed` | `EditAnywhere, BlueprintReadWrite`, *CharacterStatistics* | Base movement speed. **Note:** a code comment flags that this should move to the MainCharacter (AI reads its speed from a Data Asset), but it is kept here for now to avoid breaking the Mantis. |
| `StartingAbilities` | `EditAnywhere, BlueprintReadWrite`, *AbilitySystem* | List of abilities granted automatically on possession. |

> 🎛️ **Designer:** set `StartingAbilities` on the character Blueprint to give it
> default abilities at spawn. `BaseSpeed` is the baseline walk speed before GAS
> modifiers; for the player it is fed into the stat-initializer Gameplay Effect (see
> §1.4). For AI characters, prefer the AI Data Asset where available.

---

<a id="13-astillhearplayercontroller"></a>
### 1.3 `AStillHearPlayerController`
**Files:** `Public/Character/StillHearPlayerController.h`, `Private/Character/StillHearPlayerController.cpp`

The hub for **input, camera switching, climbing/vaulting, pause, menu and AI
affiliation**. Implements `IGenericTeamAgentInterface` (team id `10`, so AI perception
treats the player as an enemy team).

#### Responsibilities
1. **Input binding (Enhanced Input).** `SetupInputComponent` binds every `UInputAction`
   to its handler. Each handler is guarded by `InputEnabled()`, `IsMoveInputIgnored()`,
   a valid pawn, and a "not sitting" check, then calls `RegisterPlayerActivity()` to
   reset the idle-sit timer. Most handlers simply forward to a method on
   `AStillHearMainCharacter`.
2. **Camera-relative movement.** `HandleMoveTriggered` converts 2D stick input into
   world movement using the **right vector of the active camera volume**, not the
   character's facing. When the player moves into a new volume with a different
   orientation, `UpdateInputDirection` smoothly lerps the input basis over
   `InputAdjustingTime` so the control direction doesn't snap.
3. **Camera switching.** `ChangeCamera` calls `SetViewTargetWithBlend` using the
   blend parameters exposed on the camera volume (enter/exit time, function, exponent),
   with special cases for menu mode and snap requests. It also redirects the audio
   listener to the active camera (`SetAudioListenerOverride`).
4. **Climbing & vaulting (line/box traces).** A family of trace helpers
   (`ScanForWallInFrontOfPlayer`, `IsWallHighEnoughForVaulting`, `CheckWallHeightRange`,
   `CheckIfShouldVault`, `CheckVaultLandingPoint`) detect walls on the custom `Climb`
   collision channel and feed motion-warp targets to the character. `EdgeGrabTraceOnTimerEnded`
   runs on a repeating timer while jumping: two box traces locate a grabbable ledge, set
   IK hand targets via the `IIKTargetReceiver` interface, and trigger the Climb ability.
   *(Low-vault / get-on-top paths exist but are currently commented out.)*
5. **Pause & menu.** `SetPause` interrupts resonance/sound-wave, ducks the music via the
   audio subsystem, and broadcasts `OnPauseStateChanged`. `BeginPlay` optionally starts
   in main-menu mode (driven by `ASceneManager::GetStartInMenuMode` and pending
   new/load-game flags on the GameInstance), pushing the main-menu widget through the UI
   subsystem. `TransitionToPossession` hands off from menu to gameplay without a camera
   cut; `ReturnToMainMenu` tears the gameplay UI back down.
6. **Idle → Sit.** `RegisterPlayerActivity` (re)arms `IdleSitTimerHandle`; on timeout
   `OnIdleSitTimeout` makes the character sit down, unless in a menu, cinematic, or while
   an ability is active (in which case it reschedules).

#### Exposed variables
| Group / Variable | Spec | Meaning |
|---|---|---|
| `CameraEffectsComponent` | `EditDefaultsOnly`, *Components* | Camera-effects component, created in the constructor (FOV/offset modifiers — see Camera phase). |
| `MainMenuWidgetClass`, `MainMenuLayerTag` | `EditDefaultsOnly`, *UI* | Widget class pushed at startup and the UI layer tag it goes to. |
| `KeyboardMappingContext`, `GamepadMappingContext` | `EditDefaultsOnly`, *InputMapping* | Enhanced Input contexts added on `ApplyControlsSettings`. The gamepad context can be overridden by the saved settings. |
| `MoveAction`, `SprintAction`, `CrouchAction`, `JumpAction`, `ParryAction`, `ActivateResonanceAction`, `ResonanceInteractionAction`, `InteractionAction` | `EditDefaultsOnly`, *InputMapping\|MainCharacter* | Player `InputAction` assets. |
| `CompanionSoundWaveAction`, `CompanionSoundWaveInterruptAction`, `CompanionAimAction` | `EditDefaultsOnly`, *InputMapping\|Companion* | Companion `InputAction` assets. |
| `MinimumWallHeightToStartVaulting`, `MaximumWallHeightToVault`, `TagToCheckForClimbing`, `ShowLineTraces` | `EditDefaultsOnly`, *ClimbingSettings* | Primary climbing/vaulting tuning. `TagToCheckForClimbing` (default `"Climb"`) is the actor tag a wall must have to be climbable. `ShowLineTraces` draws debug traces. |
| `MinimumWallDepth`, `WallDistanceFromCharacter`, `WallDistanceFromCharacterLowVault`, `LineTraceVerticalHeight`, `VaultingWallDepth`, `MaximumStepHeightToEvaluateVault`, `VaultLandingOffset`, `EdgeGrabLeftHandOffset`, `EdgeGrabRightHandOffset`, `EdgeGrabFinalHeightOffset` | `EditDefaultsOnly`, *ClimbingSettings\|Advanced* | Fine-grained trace geometry: scan depths, vertical scan height, ledge step threshold, landing offset, and IK hand/height offsets for the edge grab. |
| `OnPauseStateChanged` | `BlueprintAssignable`, *Game* | Fires whenever the pause state changes. |

> 🎛️ **Designer:** these variables are **directly exposed**, not data-driven — tune them
> on the Player Controller Blueprint. The two most impactful for level design are
> `MinimumWallHeightToStartVaulting` / `MaximumWallHeightToVault` (the height window in
> which a wall is climbable) and `TagToCheckForClimbing` (only walls carrying this actor
> tag are climbable). Enable `ShowLineTraces` to visualize the climb traces in PIE while
> tuning. The default constructor values are a good starting point (e.g. min vault
> height `70`, max `95`).

> 🔧 **Developer:** the climbing system is **purely trace-driven on the controller** and
> communicates with the character only through motion-warp targets and IK targets — the
> actual movement is performed by the Climb Gameplay Ability + Motion Warping. Input
> handlers are deliberately thin; gameplay logic lives in the abilities.

---

<a id="14-astillhearmaincharacter"></a>
### 1.4 `AStillHearMainCharacter`
**Files:** `Public/Character/StillHearMainCharacter.h`, `Private/Character/StillHearMainCharacter.cpp`

The player character. Extends `AStillHearCharacterBase` and additionally implements
`ICameraVolumesInteractor` (camera-volume tracking) and `IAISightTargetInterface`
(custom multi-point visibility for enemy sight).

#### Subobjects (created in constructor)
| Component | Type | Notes |
|---|---|---|
| `AttributeSet` | `UMainCharacterAttributeSet` | Player attributes (speed, etc. — GAS phase). |
| `ResonanceManagerComponent` | `UResonanceManagerComponent` | Resonance VFX/logic, `bAutoActivate = false`. |
| `MotionWarping` | `UMotionWarpingComponent` | Used by climb/vault and other warped montages. |
| `CompanionComponent` | `UFloatingCompanionComponent` | Drives the floating companion (documented in the Companion phase). |
| `ParrySphereComponent` | `USphereComponent` | Parry trigger volume; collision disabled by default, tagged `ParrySphere`. |
| `BlobShadow` | `UBlobShadowComponent` | Fake blob shadow under the character. |

#### Ability surface (developer)
The character exposes thin methods that try to activate (or send stop-events to) the
corresponding ability classes. The mapping enum is `EMainCharacterAbilityType`
(`Jump, Parry, Resonance, Sprint, Crouch, LowVault, Climb, SoundWave, Interaction`) and
`GetAbilityClassByType` resolves an enum value to its assigned ability class.

| Method | Action |
|---|---|
| `Sprint()` / `StopSprinting()` | Activate `SprintAbilityClass` / send `Event.InputReleased.Sprint`. |
| `CrouchAbility()` / `StartCrouch()` / `ReleaseCrouch()` | Toggle/activate crouch / send `Event.InputReleased.Crouch`. |
| `JumpAbility()` | Activate `JumpAbilityClass` if `PersonalizedCanJump()` (coyote-time aware). |
| `Climb(ImpactPoint)` | Compute the motion-warp target and activate `ClimbAbilityClass`. |
| `Parry()` | Activate `ParryAbilityClass`. |
| `ActivateResonance()` / `DeactivateResonance()` / `ActivateResonanceInteraction()` | Activate resonance / send stop / send activate-resonance interaction event. |
| `TapInteraction()` | If already interacting, send `Event.StopInteraction`; otherwise activate `InteractionAbilityClass`. |
| `StartSoundWave()` / `ShootSoundWave()` / `InterruptSoundWave()` | Companion sound-wave lifecycle (activate / send shoot event / cancel by tag). |

#### Notable systems
- **Coyote time.** On `Falling()` (when not jumping) the character starts a
  `CoyoteTimerHandle` and allows a jump for `CoyoteTimeForJump` seconds after leaving a
  ledge. `CanJumpInternal_Implementation` and `PersonalizedCanJump` extend the engine's
  jump check with this grace window.
- **Landing.** `Landed()` measures fall distance, plays `LandForceFeedback` (rumble)
  above a threshold, kills the character via `DeathAbilityClass` if the fall exceeds
  `DeathHeightThreshold`, and **reports a hearing noise event** to AI (louder/`Vibration.Run`
  tag on `Soil` surfaces) — tying movement into the audio-stealth loop.
- **Floor detection.** `DetectFloorType()` line-traces down and robustly resolves the
  `EPhysicalSurface` (hit phys material → body instance override → material phys
  material), used by footsteps and the noise system.
- **Camera-volume tracking** (`ICameraVolumesInteractor`). The character keeps a
  `CameraVolumesList`; `AddCameraVolumeToList` / `RemoveCameraVolumeFromList` /
  `CheckList` pick the **highest-priority** valid volume and call the controller to blend
  to it. `CheckFirstCameraAtSpawn` snaps to the right volume at spawn/respawn.
- **AI visibility** (`CanBeSeenFrom`). Instead of a single trace, it tests five points
  (head, chest, center, hips, feet); the player is "seen" if *any* point is unobstructed
  — handling partial occlusion behind cover.
- **GAS tag reactions** (registered in `BeginPlay`): `HandleForceMoving`,
  `HandleDragging` (free/rail dragging toggles `bOrientRotationToMovement`),
  `HandleAttackHit` (removes the granting effect and triggers death), `HandleDeath`
  (ragdoll + disable input/movement, stop camera effects, broadcast `OnDeath`).
- **Death / Revive.** `HandleDeath` ragdolls and disables input. `Revive()` reverses
  everything: un-ragdoll, restore the cached mesh transform, re-enable collision/movement,
  uncrouch, reset jump and companion position, clear the death tag and lingering
  abilities, reset the AnimInstance flags, and re-run the spawn camera flow.
- **Idle sit.** `SitDown()` plays `SitDownMontage` and locks input until it blends out;
  `StandUp()` is triggered by the next player input. Can optionally start seated via
  `bStartSittingOnSpawn` (skipped when spawning a brand-new game).
- **Permanent abilities.** On `PossessedBy`, abilities permanently unlocked in the
  `USaveSubsystem` are granted; `RefreshPermanentAbilities()` re-syncs them.

#### Exposed variables — gameplay/stats
| Variable | Spec | Meaning |
|---|---|---|
| `SprintSpeed`, `CrouchedSpeed`, `CrouchedHeight` | `EditDefaultsOnly`, *CharacterStatistics* | Movement tuning. `CrouchedSpeed`/`CrouchedHeight` are pushed into the movement component in `InitializeStats`. |
| `DeathHeightThreshold` | `EditDefaultsOnly`, *CharacterStatistics* | Fall distance that triggers fall-death. |
| `CoyoteTimeForJump` | `EditDefaultsOnly`, *CharacterStatistics* | Grace period (seconds) to still jump after leaving a ledge. |
| `LeftHandSocketName`, `RightHandSocketName` | `EditDefaultsOnly`, *Sockets* | Hand socket names (default `LeftHand_Socket` / `RightHand_Socket`). |

#### Exposed variables — ability classes & effects (*private, EditDefaultsOnly*)
`ParryAbilityClass`, `ResonanceAbilityClass`, `InteractionAbilityClass`,
`SprintAbilityClass`, `CrouchAbilityClass`, `JumpAbilityClass`, `LowVaultAbilityClass`,
`ClimbAbilityClass`, `DeathAbilityClass`, `SoundWaveAbilityClass` — the concrete
`UGameplayAbility` subclasses to activate. `StatsInitializerGameplayEffectClass` is the
Gameplay Effect that seeds the attribute set.

> 🎛️ **Designer:** assign each `*AbilityClass` slot on the player Blueprint to the
> corresponding ability Blueprint. To unlock/lock an ability for the whole game,
> drive it through the Save System (permanent unlocks) rather than editing
> `StartingAbilities` directly.

#### Exposed variables — animation / sitting / audio / feedback
| Variable | Spec | Meaning |
|---|---|---|
| `DefaultBlendSpace` | `EditDefaultsOnly`, *Animation* | Default locomotion blend space (restored via `SetDefaultLocomotionBlendSpace`). |
| `bEnableIdleSit` | `EditDefaultsOnly`, *Animation\|Sitting* | Master toggle for the idle-sit behavior. |
| `SitDownMontage` | `EditDefaultsOnly` (EditCondition `bEnableIdleSit`) | Montage played when sitting down. |
| `IdleTimeBeforeSit` | `EditDefaultsOnly` (EditCondition, ClampMin 0.1) | Seconds of inactivity before sitting (default `20`). |
| `bStartSittingOnSpawn` | `EditDefaultsOnly`, *Animation\|Sitting* | Start the level seated; stands up on first input. |
| `FootstepConfig` | `EditDefaultsOnly`, *Audio* → `UFootStepData` | **Data-driven footstep configuration** (sounds per surface). Documented in detail in the Audio phase; read via `GetFootstepConfig()`. |
| `LandForceFeedback` | `EditDefaultsOnly`, *Feedback* | Rumble effect played on a notable landing. |

#### Exposed events
`OnDeath` and `OnInitializedFinished` (`BlueprintAssignable`) — broadcast on death and
once post-spawn initialization completes (used to gate UI/cinematics).

> 🎛️ **Designer — stats are NOT in a Data Asset here.** Player movement/stat values
> (`SprintSpeed`, `CrouchedSpeed`, `CrouchedHeight`, `DeathHeightThreshold`,
> `CoyoteTimeForJump`, `BaseSpeed`) are tuned directly on the **player Blueprint's
> Class Defaults**. The only data-driven part of this class is `FootstepConfig`
> (`UFootStepData`). At `BeginPlay`, `BaseSpeed` and a `SpeedMultiplier` of `1.0` are
> injected into the attribute set through `StatsInitializerGameplayEffectClass` using
> SetByCaller tags (`Data.BaseSpeed`, `Data.SpeedMultiplier`).

---
<a id="phase-2"></a>
## 2 — Input

StillHear uses **Enhanced Input** with **CommonUI/CommonInput** on top. The input layer
is split across two distinct responsibilities:

1. **Gameplay binding** — done by `AStillHearPlayerController` (see §1.3): it binds each
   `UInputAction` to a handler and forwards to the character. This is the *runtime*
   action layer.
2. **Settings / rebinding / glyphs** — done by `UInputSubsystem` (a
   `GameInstanceSubsystem`): it owns the catalog of mappings, lets the player remap keys,
   persists them, and resolves the on-screen key icon (glyph) for any action. This is the
   *configuration* layer, consumed mainly by the options UI.

The two layers share the same `UInputMappingContext` assets; the subsystem manipulates
those contexts (map/unmap keys) so that changes are reflected at gameplay time.

<a id="21-data-assets--supporting-types"></a>
### 2.1 Data assets & supporting types

#### `UMappingContextList` (Data Asset)
**File:** `Public/Input/MappingContextList.h`

A simple `UDataAsset` holding `TArray<UInputMappingContext*> MappingContexts` — the full
catalog of input mapping contexts in the game. The `UInputSubsystem` loads this asset
(referenced on the `UStillHearGameInstance`) and treats it as the single source of truth
for "all mappings that exist".

> 🎛️ **Designer:** create one `MappingContextList` data asset, add every gameplay
> `InputMappingContext` to its `MappingContexts` array, and assign it on the GameInstance.
> Any context not listed here is invisible to the rebinding system (won't appear in the
> options menu and can't be remapped).

#### `FBindingData` (struct)
**File:** `Public/Input/BindingData.h`

The unit of a single key binding, stored in `SettingsSaveGame`:

| Field | Meaning |
|---|---|
| `InputAction` | The action this binding refers to. |
| `CurrentBoundKey` | The key currently assigned (possibly remapped by the player). |
| `DefaultBoundKey` | The original key from the mapping context (used for "reset to default"). |
| `DeviceType` | `KeyboardMouse` or `Controller` (auto-detected from the key). |

Implements `operator==` for array lookups/removals.

#### Enums
- **`EInputDeviceType`** (`Public/Input/InputDeviceType.h`): `KeyboardMouse`, `Controller`.
- **`EControllerCategory`** (`Public/Input/ControllerCategory.h`): `PlayStation`, `Xbox` —
  selects which glyph set to use for gamepad icons.

<a id="22-uinputsubsystem--lifecycle--catalog"></a>
### 2.2 `UInputSubsystem` — lifecycle & catalog
**Files:** `Public/Input/InputSubsystem.h`, `Private/Input/InputSubsystem.cpp`

A `UGameInstanceSubsystem` that is the backbone of the input-settings layer.

#### Initialization (`Initialize`)
1. Initializes its dependency on `USaveSubsystem`.
2. Loads the `UMappingContextList` from the GameInstance (synchronous load). If missing,
   logs an on-screen error (editor only) and bails out.
3. Copies `MappingContextList->MappingContexts` into `AllMappingContexts`.
4. `CacheDefaultBindings()` — walks every mapping of every context and builds the
   `CurrentBindings` array of `FBindingData`, recording each key as both *default* and
   *current*, and tagging the device type from whether the key is a gamepad key.
5. If the player has saved bindings (`SettingsSaveGame::Bindings` non-empty), calls
   `ApplySavedBindings()` to reapply them on top of the default contexts.

#### Internal state
| Member | Role |
|---|---|
| `AllMappingContexts` | The catalog of all `UInputMappingContext`s. |
| `CurrentBindings` | The live list of `FBindingData` (current + default keys per action). |
| `RebindUtilityArray` | Temporary staging list used during a rebinding transaction. |
| `CurrentControllerType` | Which `EControllerCategory` glyph set to use for gamepad icons. |

<a id="23-rebinding-model-developer"></a>
### 2.3 Rebinding model (developer)

All remap operations follow the same **three-phase pattern** to keep the Enhanced Input
contexts coherent (you must not mutate a context while iterating its mappings, and all
removals must precede additions to avoid transient key collisions):

> **Phase 1 — capture:** iterate the contexts while still coherent and record what needs
> to change (context, action, from-key, to-key, **and the existing `Modifiers`**).
> **Phase 2 — remove:** `UnmapKey` all the old mappings.
> **Phase 3 — add:** `MapKey` the new mappings and **re-attach the captured modifiers**
> (critical: modifiers like Negate/Swizzle define movement direction and would be lost
> otherwise).

Methods implementing this:

| Method | Purpose |
|---|---|
| `ApplySavedBindings(Settings)` | On startup, replace default keys with the player's saved `CurrentBoundKey`s. |
| `ApplyDefaultBindings()` | Revert every remapped action back to its `DefaultBoundKey`, updating `CurrentBindings`. |
| `RebindKeys()` | Commit the staged `RebindUtilityArray` into the live contexts (then clears the staging array). |
| `AddToKeysToRebind` / `AddToKeysToRebindArray` / `ClearKeysToRebind` | Manage the staging list before committing. |

#### Persistence
| Method | Purpose |
|---|---|
| `SaveBindings()` | Copy `CurrentBindings` into `SettingsSaveGame::Bindings` and save async. |
| `DeleteBindings()` | Clear the saved bindings and save async. |
| `ResetSavedBindingsToDefault()` | `ApplyDefaultBindings()` + `SaveBindings()`. |

<a id="24-query-helpers-for-the-ui-developer"></a>
### 2.4 Query helpers for the UI (developer)

The options/HUD widgets use these to display the right key and glyph:

| Method | Returns |
|---|---|
| `GetCurrentKeyForAction(Action, DeviceType)` | The currently bound key for an action on a given device. |
| `GetDefaultKeyForAction(Action, CurrentKey)` | The default key for an action (used to show "reset" affordances). |
| `GetCurrentKeyForMoveDirection(Action, Direction)` | Resolves which key maps to a 1D movement direction (`Forward/Back/Left/Right`) of a 2D move action, by inspecting **Negate / Swizzle** modifiers — Forward = Swizzle only, Left = Negate only, Right = none, Backward = both. |
| `GetBrushFromKey(Key, DeviceType)` | The `FSlateBrush` glyph for a key; for gamepad it picks the PlayStation/Xbox data asset based on `CurrentControllerType`. |
| `IsInputActionSetToDefault(Action, Key)` | Whether a binding is still at its default. |
| `SetControllerInputType(ControllerName)` | Forwards to CommonUI's `UCommonInputSubsystem::SetGamepadInputType` to switch the active gamepad glyph family. |

`HasNegateModifier` / `HasSwizzleModifier` are private helpers that scan a mapping's
modifier list for `UInputModifierNegate` / `UInputModifierSwizzleAxis`.

<a id="phase-3"></a>
## 3 — Gameplay Ability System (Core)

StillHear builds essentially **all** character actions on GAS. This section documents the
*core, reusable* GAS infrastructure: the Ability System Component, the attribute sets, the
base ability class, the generic Gameplay Effect, the native tag catalog, the reusable
ability task, and the generic Gameplay Cue base classes. The **concrete** abilities
(Sprint, Jump, Climb, Parry, Resonance, Sound Wave, Interactions…) and their specific
cues/effects are documented in later phases (Abilities, Interactions, EnemiesAI).

Since GAS was introduced in the early stages of development, the architecture follows an open mindset with a possible UI heavy implementation and/or multiplayer. These things have not been used in further development but the base architecture remained, in case some changes will be needed.

<a id="31-ustillhearabilitysystemcomponent-asc"></a>
### 3.1 `UStillHearAbilitySystemComponent` (ASC)
**Files:** `Public/GameplayAbilitySystem/Component/StillHearAbilitySystemComponent.h`,
`Private/.../StillHearAbilitySystemComponent.cpp`

A thin subclass of `UAbilitySystemComponent`, created on every
`AStillHearCharacterBase` (§1.2). Its only added behavior is **detecting when the set of
granted abilities changes** and notifying listeners.

- Keeps a cached copy `LastActivatableAbility` of the granted ability specs.
- On `OnRep_ActivateAbilities` it compares the cache against the engine's live
  `ActivatableAbilities`. If the count differs (ability added/removed) — or, with equal
  counts, an entry differs — it calls `Character->SendAbilitiesChangedEvent()` (which
  broadcasts `Event.Abilities.Changed`) and refreshes the cache.

> 🔧 **Developer:** this is the hook the **UI** uses to refresh ability widgets when the
> player unlocks/loses an ability at runtime. If you grant abilities through a path that
> doesn't go through `GrantAbilities`/`OnRep`, remember to fire the changed event yourself.
> *(Note: the equal-count branch compares with `==` to detect a change, i.e. it flags
> "changed" when specs match index-by-index — see the source if you touch this logic.)*

<a id="32-attribute-sets"></a>
### 3.2 Attribute Sets

#### `UBasicAttributeSet`
**Files:** `Public/GameplayAbilitySystem/Attributes/BasicAttributeSet.h`,
`Private/.../BasicAttributeSet.cpp`

The shared attribute set for all characters. Declares three attributes via the
`ATTRIBUTE_ACCESSORS` macro (which generates the standard getter/setter/initter for each):

| Attribute | Default | Meaning |
|---|---|---|
| `BaseSpeed` | `600` | Base movement speed. |
| `SpeedMultiplier` | `1.0` | Multiplier applied on top of `BaseSpeed`. |
| `MaxParryAngle` | — | Angular tolerance for a successful parry. |

**Speed model (developer):** the final movement speed is `BaseSpeed * SpeedMultiplier`,
pushed into the movement component (`MaxWalkSpeed`) by
`UpdateCharacterSpeed`. This is wired through both GAS hooks:
- `PreAttributeChange` — recomputes speed *before* either attribute changes (so
  duration-based / infinite effects that modify the current value update speed live).
- `PostGameplayEffectExecute` — re-applies the value *after* an instant effect executes.

`PreAttributeChange` is also the documented place to **clamp** attributes if needed.

> 🎛️ **Designer:** to make something faster/slower (sprint, slow zones, status effects),
> author a Gameplay Effect that modifies `BaseSpeed` or `SpeedMultiplier` rather than
> touching the movement component directly — the attribute set keeps the character's
> `MaxWalkSpeed` in sync automatically. `MaxParryAngle` tunes how forgiving
> the parry aim is.

#### `UMainCharacterAttributeSet`
**Files:** `Public/.../MainCharacterAttributeSet.h`, `Private/.../MainCharacterAttributeSet.cpp`

Currently an **empty subclass** of `UBasicAttributeSet`. It exists as a dedicated type for
the player (so the player can own a distinct attribute-set class) and as a hook for
future player-only attributes.

<a id="33-ustillheargameplayability-base-ability"></a>
### 3.3 `UStillHearGameplayAbility` (base ability)
**Files:** `Public/GameplayAbilitySystem/Abilities/StillHearGameplayAbility.h`,
`Private/.../StillHearGameplayAbility.cpp`

The base class for every gameplay ability in the project. Constructor defaults:
- Adds `GameplayAbility.Active` to `ActivationOwnedTags` (so any active ability tags the
  owner as "doing something" — used widely, e.g. idle-sit suppression in §1.4).
- Adds `Status.Death` to `ActivationBlockedTags` (dead characters can't activate abilities).
- `InstancingPolicy = InstancedPerActor`.

#### UE 5.5 blocked-tags bug fix (developer — important)
It overrides `DoesAbilitySatisfyTagRequirements` to fix a regression introduced in UE 5.5:
when a tag put in **Block Abilities With Tag** is a *parent* tag (e.g. blocking
`GameplayAbility` to block `GameplayAbility.AbilityOne`), the engine fails to recognize the
child as blocked. The override restores the correct check: the original engine line used
`!ContainerA.HasAny(ContainerB)`, and here it is corrected to test
`!ContainerB.HasAny(ContainerA)` so parent/child blocking works as it did pre-5.5. The rest
of the function mirrors the engine's blocked/required-tag logic (populating
`OptionalRelevantTags` with the proper fail tags).

> 🔧 **Developer:** because of this, **block/required tag relationships rely on tag
> hierarchy**. You can block a whole family of abilities by listing their common parent tag
> in *Block Abilities With Tag*. Don't revert this override or the hierarchical blocking
> will silently break again on 5.5+.

<a id="34-uge_granttagstoactor-generic-gameplay-effect"></a>
### 3.4 `UGE_GrantTagsToActor` (generic Gameplay Effect)
**Files:** `Public/GameplayAbilitySystem/Effects/GE_GrantTagsToActor.h`,
`Private/.../GE_GrantTagsToActor.cpp`

A minimal `UGameplayEffect` whose only default is `DurationPolicy = Infinite`. It is the
generic "apply a persistent tag (or set of tags) to an actor until removed" effect — the
granted tags are configured per-instance in the derived Blueprint effect.

> 🎛️ **Designer:** derive a Blueprint from this effect, set its *Granted Tags* (e.g. a
> status like `Status.MainCharacter.Crouched`), and apply/remove it from an ability to
> toggle a persistent state. Because the duration is Infinite, you control its lifetime by
> removing the effect (by handle or by granted-tag query).

<a id="35-native-gameplay-tags"></a>
### 3.5 Native Gameplay Tags
**Files:** `Public/GameplayAbilitySystem/Tags/GameplayTags.h`,
`Private/.../GameplayTags.cpp`

All gameplay tags are declared **natively in C++** with `UE_DECLARE_GAMEPLAY_TAG_EXTERN`
(and defined in the `.cpp`), then referenced as `TAG_...` constants throughout the code.
This gives compile-time safety (no stray string typos) and IDE autocompletion.

The catalog is organized by domain (via `#pragma region`):

| Region | Contents (examples) |
|---|---|
| **SHARED** | Cross-cutting abilities/status/events: `GameplayAbility.Active`, `Status.Death`, `Status.Falling`, `Status.FreeDragging`/`RailDragging`, `Event.AttackHit`, `Event.StopInteraction`, `Event.Interaction.Success`, `Event.Resonance.CrystalBroken`. |
| **MAIN_CHARACTER** | Per-ability tags (each with a `…` and a `….Active` variant): Jump, Sprint, Crouch, Climb, Parry, Resonance, TapInteraction, LowVault, LowGetOnTop, RecallCompanion, SetCompanionFree. Plus Status (`Crouched`, `ParryCooldown`, `ForceMoving`, `Moving`, `PreventJumping`, `PreventDeath`), Events (`StopResonance`, `ActivateResonance`, `EndCrouch`), Cues (Parry / ParryImpact / ParryCooldownFinished / Death) and Data (`ParryCooldown`, `CrouchCooldown`). |
| **COMPANION** | `GameplayAbility.Companion.SoundWave`, Status (`Outside`, `Aiming`, `SoundWaveCooldown`), Events (`Recall`, `Spawn`, `Despawn`, `ShootSoundWave`, `SwitchSoundWaveTarget`), SoundWave cues, Data (`SoundWaveChargeDuration`, `SoundWaveCooldown`). |
| **ENEMIES** | Abilities (Attack, CloseAttack, MantisShift + Nav/Attack sub-phases, WormRoar, WormDolphinDive, Stun), Status (`Stunned`, `AttackCooldown`, and the awareness ladder `Unaware`→`Suspicious`→`Alerted`→`Hunting`), Events, Cues, Data. |
| **INPUTS** | `Event.InputReleased.Crouch`, `Event.InputReleased.Sprint`. |
| **COLLISIONS** | `Event.Collision.Activate`. |
| **INTERACTIONS** | `Interact.Tap`, `Interact.Deflect`, `Interact.Projectile`, `Interact.Hold.DragFree`, `Interact.Hold.DragRail`, `Interact.Resonance`. |
| **UI** | Layer tags (`Game`, `Menu`, `Window`, `Modal`), action tags (`Settings`). |
| **AUDIO** | States (`Audio.State.MainMenu`). |
| **UTILS** | Actor/world tags: `Climb`, `Counterweight`, `AttackHitBox`, `ParrySphere`, `MainCharacter`, `Enemy`, plus the `GroundDebris` cue. |

> 🔧 **Developer — adding a tag:** declare it in `GameplayTags.h` with
> `UE_DECLARE_GAMEPLAY_TAG_EXTERN` inside the relevant region, define it in
> `GameplayTags.cpp` with the matching `UE_DEFINE_GAMEPLAY_TAG(...)` and its dotted string,
> then use the `TAG_...` constant in code. Note the **convention**: an ability has both a
> base tag and an `…_Active` tag — the base identifies the ability, `…_Active` marks that
> it's currently running (used in `ActivationOwnedTags` and runtime `HasMatchingGameplayTag`
> checks). Enemy **awareness states** are also modeled as a tag ladder.

<a id="36-uat_navigateto-reusable-ability-task"></a>
### 3.6 `UAT_NavigateTo` (reusable Ability Task)
**Files:** `Public/GameplayAbilitySystem/Tasks/AT_NavigateTo.h`,
`Private/.../AT_NavigateTo.cpp`

A custom `UAbilityTask` that drives the avatar character toward a world location using
**standard movement input** (not the navmesh pathfinder), then optionally rotates it to
face a direction, broadcasting `OnTargetLocationReached` when done.

Factory: `NavigateTo(OwningAbility, TargetLocation, AcceptanceRadius, FacingDirection = Zero)`.

Behavior (ticking task):
- **Move phase:** adds movement input toward the (horizontal) target, scaling the input
  down within the last 100 units for a smooth arrival (`MovementScale` clamped 0.2–1.0).
  The target is considered reached when within `AcceptanceRadius`, **or** if the character
  stops moving after having started, **or** if it fails to start moving within a 0.15 s
  grace period (`bFailedToMove`) — so the task can't hang if the path is blocked.
- **Face phase:** once reached, interpolates the actor's yaw toward `FacingDirection`
  (or toward the original target if none was given) at the movement component's
  `RotationRate`, finishing when within 5°.

> 🔧 **Developer:** use this inside an ability when you need the character to "walk to a
> spot and turn" as part of an ability flow (e.g. moving to an interaction anchor). It is
> input-driven, so it respects the character's normal acceleration/speed and any active
> speed modifiers.

<a id="37-generic-gameplay-cue-base-classes"></a>
### 3.7 Generic Gameplay Cue base classes
Reusable cue bases that spawn **SFX + VFX** with consistent, designer-friendly attach
options. All three use the shared `EAttachPoint` enum (`Public/.../Cues/Generic/EAttachPoint.h`):
`ROOT`, `MESH` (attach to a socket via `…SocketName`), `COMPONENT` (attach to a component
found by `…ComponentTag`).

#### `UGC_SoundAndVFXBurst`
**File:** `Public/.../Cues/Generic/GC_SoundAndVFXBurst.h` (+ `.cpp`)
Extends `UGameplayCueNotify_Burst` — a **one-shot** cue. On `OnExecute` it spawns the
configured `VFX` (Niagara) and `SFX` once. `SpawnBurstEffects` is `virtual` for subclasses
that need custom logic. Use for instantaneous feedback (impacts, hits).

#### `AGC_SoundAndVFXActor`
**File:** `Public/.../Cues/Generic/GC_SoundAndVFXActor.h` (+ `.cpp`)
Extends `AGameplayCueNotify_Actor` — a **persistent** cue. On `OnActive` it spawns VFX/SFX
(optionally looping, `bLoop`) and keeps references (`SpawnedVFX`, `SpawnedSFX`); on
`OnRemove` it tears them down. Use for state-bound feedback that must start and later stop
(charging, ongoing auras).

#### `AGC_CooldownFinishedBase` (abstract)
**File:** `Public/.../Cues/Generic/GC_CooldownFinishedBase.h` (+ `.cpp`)
Extends `AGameplayCueNotify_Actor` but **inverts the lifecycle**: it stays silent on
`OnActive` (when the cooldown effect is applied) and plays SFX/VFX on `OnRemove` (when the
cooldown's Gameplay Effect expires) — i.e. "cooldown ready" feedback. Subclasses override
`GetFinishCooldownSound()` to supply the sound (typically from a data asset).

Shared exposed variables (all three):
| Variable | Meaning |
|---|---|
| `SpawnedOnCharacter` | Whether the cue attaches to the target character (enables the attach-point options). |
| `VFX` / `SFX` | The Niagara system and sound to play. |
| `VFXAttachPoint` / `SFXAttachPoint` | `ROOT` / `MESH` / `COMPONENT` (see `EAttachPoint`). |
| `VFXSocketName` / `SFXSocketName` | Socket to attach to when attach point is `MESH`. |
| `VFXComponentTag` / `SFXComponentTag` | Component tag to find when attach point is `COMPONENT`. |
| `bLoop` *(Actor variant)* | Whether the SFX loops until removed. |

> 🎛️ **Designer:** pick the right base when authoring a cue Blueprint —
> **Burst** for one-shot effects, **Actor** for start/stop effects, **CooldownFinishedBase**
> for "ability is ready again" feedback. The `EditCondition` metadata hides the
> socket/component fields unless they apply, so the cue's details panel only shows the
> options relevant to the chosen attach point. These bases are what the concrete cues
> (Parry, Stun, MantisShift, SoundWave, Death, …) build on.

<a id="phase-4"></a>
## 4 — Player Abilities

All player actions are `UStillHearGameplayAbility` subclasses (§3.3) named `GA_*`. They are
assigned to the player on its Blueprint (the `*AbilityClass` slots in §1.4) and activated
from the controller via the character's thin wrapper methods.

<a id="40-common-ability-pattern-developer"></a>
### 4.0 Common ability pattern (developer)
Most player abilities follow the same skeleton, worth understanding once:

1. **Constructor** sets the identity tag as an **Asset Tag** (`SetAssetTags`, e.g.
   `GameplayAbility.MainCharacter.Sprint`), adds the `…_Active` tag to
   `ActivationOwnedTags`, and declares `Block`/`CancelAbilitiesWithTag` to define
   mutual exclusions (recall the hierarchical tag blocking from §3.3).
2. **`ActivateAbility`** casts the avatar to `AStillHearMainCharacter`, performs the
   gameplay change (apply a GE, play a montage, toggle movement), and starts one or more
   **Ability Tasks** to wait for the end condition.
3. **End condition** is usually an `AbilityTask_WaitGameplayEvent` listening for an
   input-release event (e.g. `Event.InputReleased.Sprint`), a montage callback, or a
   movement-mode change.
4. **`EndAbility`** undoes the gameplay change (remove the GE, restore movement/collision).

Two recurring helpers:
- **Speed effects** use a Gameplay Effect with a **SetByCaller** magnitude
  (`Data.SpeedMultiplier` / `Data.BaseSpeed`) so one effect asset serves many values.
- **Cooldowns** are often overridden in `ApplyCooldown` to inject a **data-asset-driven
  duration** via SetByCaller, instead of baking the duration into the GE.

> 🎛️ **Designer:** the abilities split cleanly into *logic* (C++, fixed) and *tuning*
> (Blueprint defaults + Data Assets). The richer abilities (Crouch, Parry, Resonance) read
> their numbers from a dedicated **Data Asset** — edit those, not the C++.

<a id="41-uga_sprint"></a>
### 4.1 `UGA_Sprint`
**Files:** `Public/.../Abilities/GA_Sprint.h`, `Private/.../GA_Sprint.cpp`

Activates on sprint press; **blocks and cancels** Crouch. Computes a multiplier
`SprintSpeed / BaseSpeed` (both from the character, §1.4) and applies
`SprintSpeedMultiplierEffectClass` with that value as the `Data.SpeedMultiplier`
SetByCaller. Waits for `Event.InputReleased.Sprint`, then ends; `EndAbility` removes the
speed effect (restoring base speed via the attribute set, §3.2).

| Exposed (Blueprint defaults) | Meaning |
|---|---|
| `SprintSpeedMultiplierEffectClass` | The GE that scales `SpeedMultiplier`. |

> 🎛️ **Designer:** the sprint *speed* is **not** here — it's `SprintSpeed` on the player
> character (§1.4). This ability only needs its multiplier GE assigned.

<a id="42-uga_jump"></a>
### 4.2 `UGA_Jump`
**Files:** `Public/.../Abilities/GA_Jump.h`, `Private/.../GA_Jump.cpp`

Blocks **all** abilities while active (`BlockAbilitiesWithTag = GameplayAbility`). Re-checks
`PersonalizedCanJump()` (coyote time, §1.4), calls the character's `Jump()`, plays
`JumpForceFeedback`, applies `AirStatusEffectClass` (an in-air status tag), then waits with
`AbilityTask_WaitMovementModeChange` for the return to `MOVE_Walking` to end and remove the
air status.

| Exposed | Meaning |
|---|---|
| `AirStatusEffectClass` | GE granting the "in air" status while airborne. |
| `JumpForceFeedback` | Rumble on jump. |

<a id="43-uga_crouch--data-driven-ucrouchdata"></a>
### 4.3 `UGA_Crouch` — *data-driven* (`UCrouchData`)
**Files:** `Public/.../Abilities/GA_Crouch.h`, `Private/.../GA_Crouch.cpp`
**Data Asset:** `Public/Data/DataAssets/CrouchData.h`

Calls `Character->Crouch()`, applies the crouch **status GE** from the data asset (only if
the movement component allows crouching), and waits for `Event.InputReleased.Crouch`. On
release it commits the cooldown and ends (uncrouch + remove status GE). `ApplyCooldown` is
overridden to set the cooldown duration from the data asset via the
`Data.MainCharacter.CrouchCooldown` SetByCaller. Self-blocks while active or while the
`Status.MainCharacter.Crouched` tag is present.

**`UCrouchData` fields:**
| Field | Default | Meaning |
|---|---|---|
| `CrouchStatusEffect` | — | Infinite GE granting the crouched status (and any modifiers). |
| `CooldownDuration` | `0.3` | Seconds before crouch can re-trigger. |

> 🎛️ **Designer:** create a `CrouchData` asset, assign the status effect and cooldown,
> and reference it on the Crouch ability Blueprint. The crouched movement *speed* and
> *height* are on the character (`CrouchedSpeed` / `CrouchedHeight`, §1.4).

<a id="44-uga_climb"></a>
### 4.4 `UGA_Climb`
**Files:** `Public/.../Abilities/GA_Climb.h`, `Private/.../GA_Climb.cpp`

Triggered by the controller's edge-grab detection (§1.3). Blocks other MainCharacter
abilities, cancels Jump. On activate: plays `ClimbForceFeedback`, stops movement, switches
to `MOVE_Flying`, makes the capsule ignore world-static, disables foot IK
(`SetAnimationClimbing(true)`), sets a **Motion Warp** target named `"Climb"` to the
character's `MotionWarpTarget`, and plays `ClimbMontage` (warped). Montage
completed/interrupted/cancelled all route to `EndAbility`, which restores collision,
`MOVE_Walking`, foot IK, and removes the warp target.

| Exposed | Meaning |
|---|---|
| `ClimbMontage` | The warped climb animation. |
| `ClimbForceFeedback` | Rumble on climb start. |

> 🔧 **Developer:** the landing position is computed *outside* the ability (controller
> traces → `SetMotionWarpTarget`); the ability just warps the montage to it. Foot IK is
> suppressed during the climb to avoid the legs snapping to the wall.

<a id="45-uga_parry--data-driven-uparrydata"></a>
### 4.5 `UGA_Parry` — *data-driven* (`UParryData`)
**Files:** `Public/.../Abilities/GA_Parry.h`, `Private/.../GA_Parry.cpp`
**Data Asset:** `Public/Data/DataAssets/ParryData.h`

The most involved player ability. Adds `Status.MainCharacter.ForceMoving` while active.
Flow:
1. On activate: checks cooldown + data asset, plays the press rumble and the parry montage,
   caches the character's **parry sphere** (§1.4), and waits for the
   `Event.Collision.Activate` gameplay event (fired by an **AnimNotify** at the right frame).
2. On that event: sizes the sphere to `ParryRadius`, sets overlap responses for the
   configured `DetectionChannels`, enables it, checks already-overlapping components, binds
   `OnComponentBeginOverlap`, adds the parry-window cue, and starts the
   `ParryWindowDuration` timer.
3. On overlap: for any actor whose ASC has the `RequiredEnemyTags`, applies
   `StunEffectClass`; on success triggers the **sensory effects** — slow-motion via the
   `TimeManagementSubsystem` (`SloMoEffectCurve`), success rumble, and a camera-effect
   preset — and spawns the impact cue at the contact point.
4. When the window ends: if a successful parry happened, applies invulnerability
   (`InvulnerabilityEffectClass`, duration from data) and waits for its removal before
   committing the cooldown and ending. `ApplyCooldown` uses `CooldownDuration` from data
   via the `Data.MainCharacter.ParryCooldown` SetByCaller.

**`UParryData` fields (selected):**
| Field | Default | Meaning |
|---|---|---|
| `ParryMontage` | — | Parry animation (carries the activate AnimNotify). |
| `EffectPreset` | — | `FCameraEffectPreset` played on success (see Camera phase). |
| `ParryRadius` | `200` | Detection sphere radius. |
| `ParryWindowDuration` | `0.5` | How long the parry window stays open. |
| `InvulnerabilityDuration` / `InvulnerabilityEffectClass` | `1.0` / — | I-frames granted after a successful parry. |
| `DetectionChannels` | — | Collision channels the sphere overlaps. |
| `RequiredEnemyTags` | — | Target ASC must match these to be parryable. |
| `CooldownDuration` | `3.0` | Parry cooldown. |
| `StunEffectClass` | — | GE applied to the parried enemy. |
| `SloMoEffectCurve` | — | Time-dilation curve on success. |
| `ParryPressedForceFeedback` / `ParrySuccessForceFeedback` | — | Light press rumble / strong success rumble. |
| `FinishCooldownSound` | — | Sound when the cooldown ends (used by the cooldown cue). |

> 🎛️ **Designer:** virtually every knob of the parry lives in `UParryData` — timing
> window, radius, i-frames, slow-mo curve, camera preset, rumbles, target-tag filter, and
> the stun effect applied to enemies. Tune it entirely from the data asset.

<a id="46-uga_resonance--data-driven-uresonancedata"></a>
### 4.6 `UGA_Resonance` — *data-driven* (`UResonanceData`)
**Files:** `Public/.../Abilities/GA_Resonance.h`, `Private/.../GA_Resonance.cpp`
**Data Asset:** `Public/Data/DataAssets/ResonanceData.h`

A rhythm/match ability that activates nearby **resonance interactables**. On activate it
caches the camera-effects component, finds valid resonance objects in radius
(`FindValidResonanceObjects`: overlaps the custom `Resonance` collision channel and keeps
actors implementing `IInteractable` whose tags match `ResonanceTags`), starts the
`UResonanceManagerComponent`, plays the entry camera preset and looping rumble, and waits
for two events:
- `Event.MainCharacter.ActivateResonance` (the "match input") → `AttemptMatch()` on the
  manager;
- `Event.MainCharacter.StopResonance` → cancel.

The manager broadcasts success/interrupt. On **success** it plays the success preset and
calls `StartInteraction(character)` on every found interactable; on **interrupt** it plays
the interrupt preset. `EndAbility` stops the manager, clears effects and stops the looping
rumble.

**`UResonanceData` fields (grouped):**
- **Targeting:** `ResonanceTags`, `ResonanceRadius (200)`, `ResonanceEffectClass`.
- **Match config:** `MaxHeight (50)`, `MatchThreshold (15)`, `SpeedMultiplier (2)`,
  `ResetSpeed (15)`, `ResonanceCurve`.
- **Camera presets:** `Entry / Phase2 / Success / Interrupt EffectPreset`.
- **VFX:** `Phase1VFX`, `Phase2VFX`, `InterruptVFX`.
- **Sounds:** entry / active / collide / phase2 / success / fail / interrupt.
- **Feedback:** `ActiveResonanceForceFeedback` (looping), `InteractResonanceForceFeedback`.

> 🎛️ **Designer:** the actual match minigame tuning (`MatchThreshold`, `SpeedMultiplier`,
> `ResetSpeed`, `ResonanceCurve`) and all the audio/visual feedback are in `UResonanceData`.
> The resonance **logic** lives in `UResonanceManagerComponent` (VFX phase). Which objects
> respond is controlled by `ResonanceTags` matched against the interactables' own tags.

<a id="47-uga_death-state-ability"></a>
### 4.7 `UGA_Death` (state ability)
**Files:** `Public/.../Abilities/State/GA_Death.h`, `Private/.../GA_Death.cpp`

`Cancel`s and `Block`s **all** abilities (`GameplayAbility` parent tag). On activate it
plays `DeathForceFeedback`, applies `DeathEffectClass` to the owner (which grants
`Status.Death` — the tag the character's `HandleDeath` reacts to, §1.4), and immediately
ends. Triggered from the character on lethal falls / attack hits.

| Exposed | Meaning |
|---|---|
| `DeathEffectClass` | GE granting the death status. |
| `DeathForceFeedback` | Heavy death rumble. |

<a id="48-uga_lowvault--uga_lowgetontop--currently-dormant"></a>
### 4.8 `UGA_LowVault` / `UGA_LowGetOnTop` — *currently dormant*
**Files:** `Public/.../Abilities/GA_LowVault.h` (+ `.cpp`), `Public/.../Abilities/GA_LowGetOnTop.h` (+ `.cpp`)

Montage-driven traversal abilities (each plays a montage and ends on
completed/interrupted/cancelled; `GA_LowVault` also listens for `Event.Collision.Activate`).
**Note:** their activation paths in the controller/character are currently **commented out**
(see §1.3 `HandleJumpStarted` and §1.4), so they are not reachable in the current build.
Documented here for completeness; re-enable the input paths to use them.

| Exposed | Meaning |
|---|---|
| `VaultMontage` / `LowGetOnTopMontage` | The traversal animation for each. |

> 🔧 **Developer:** these classes are intact but unwired. If you revive low-vault/get-on-top,
> restore the commented scan/activation logic in `AStillHearPlayerController::HandleJumpStarted`
> and the corresponding character methods.

<a id="phase-5"></a>
## 5 — Interactions

The interaction system lets the player (and other triggers) act on world objects: pushing,
dragging, driving along rails, collecting, deflecting projectiles, solving resonance
puzzles, etc. It is built from four layers:

1. **Interfaces** — the contracts an object can fulfill (`IInteractable`, `ITargetable`,
   plus the persistence pair `IRestorable` / `ISavable`).
2. **Interaction abilities** — player-side `GA_*Interaction` that locate an object, walk to
   it, and call its interface (data-driven via `UInteractionBaseData`).
3. **Interactable actors** — the `AInteractableObj` base and its many specializations.
4. **Trigger & helper components** — reusable `UBaseTriggerComponent` volumes and spot/shape
   helpers attached to interactables.

<a id="51-interfaces"></a>
### 5.1 Interfaces

#### `IInteractable` (`Public/Interfaces/Interactable.h`)
The core contract for anything the player can act on:
| Method | Purpose |
|---|---|
| `StartInteraction(Interactor)` / `EndInteraction(Interactor)` | Begin/end an interaction with a character. |
| `TriggerInteraction(Triggerer)` | Fire the interaction from a non-character source (e.g. a trigger volume or chain). |
| `GetNearestInteractionSpotLocation(From, OutLoc, OutDir)` | Return the best approach spot/direction so the ability can walk the character there; returns `false` if the object has no defined spot. |

#### `ITargetable` (`Public/Interfaces/Targetable.h`)
For objects that can be targeted by the companion's sound wave / deflection:
`SetTargeted` / `SetUntargeted`, `HitTarget(DeflectionsCount)` / `StopHitTarget`,
`IsTargetable()`.

#### `IRestorable` (`Public/Interfaces/Restorable.h`)
Checkpoint contract: `Reset()`, `SaveCheckpointState()`, `ClearCheckpointState()`. Lets an
object snapshot its state at a checkpoint and roll back on death/reload. Paired with
`ISavable` (Save System phase) for disk persistence.

<a id="52-interaction-abilities"></a>
### 5.2 Interaction abilities

#### `UGA_InteractionBase` (abstract) — *data-driven* (`UInteractionBaseData`)
**Files:** `Public/.../Abilities/Interaction/GA_InteractionBase.h`, `Private/.../GA_InteractionBase.cpp`

The shared logic for every interaction. On activate it:
1. Starts listening for `Event.StopInteraction`.
2. Overlaps a sphere of `InteractRadius` on the custom `Interactable` object channel and
   **sorts hits by distance** (closest first).
3. For each hit, keeps the first actor that implements `IInteractable` **and**
   `IGameplayTagAssetInterface` **and** matches `InteractionTags`, and whose Z difference is
   within `MaxZDifferenceToInteract`.
4. Asks the object for its nearest interaction spot; if none, approaches the object directly.
5. Spawns a `UAT_NavigateTo` task (§3.6) to walk the character to the spot, cancels Sprint,
   and applies the `MoveToEffectClass` movement effect during travel.
6. When the spot is reached (`OnTargetLocationReached`): removes the move effect, plays the
   interaction rumble, and calls the pure-virtual `OnInteractionStart()`.

`EndAbility` ends the navigate task and removes the move effect. Subclasses implement
`OnInteractionStart()` and `OnStopEventReceived()`.

**`UInteractionBaseData` fields:**
| Field | Default | Meaning |
|---|---|---|
| `InteractRadius` | `200` | Search radius for interactables. |
| `MoveToAcceptanceRadius` | `100` | Stop distance when approaching an object without a defined spot. |
| `MaxZDifferenceToInteract` | `50` | Max vertical gap to allow interaction. |
| `SpotAcceptanceRadius` | `5` | Stop distance when approaching a defined spot. |
| `InteractionTags` | — | Tags the target must match. |
| `MoveToEffectClass` | — | GE applied while walking to the object. |
| `InteractionForceFeedback` | — | Rumble when the interaction starts. |
| `bDrawDebug` | `false` | Draws the search sphere in editor. |

#### `UGA_TapInteraction` (`UTapInteractionData`)
Single-shot interaction. `OnInteractionStart` calls `StartInteraction` on the object;
`OnStopEventReceived` and `EndAbility` call `EndInteraction`. Self-blocks while active.

#### `UGA_HoldInteraction` (`UHoldInteractionData`)
Hold-to-interact. `OnInteractionStart` calls `StartInteraction`; the interaction persists
until `Event.StopInteraction`, at which point `OnStopEventReceived` calls `EndInteraction`
and ends. Blocks all MainCharacter abilities while held (used for drag interactions).

> 🎛️ **Designer:** `UTapInteractionData` and `UHoldInteractionData` are thin subclasses of
> `UInteractionBaseData` (only their *type* differs, so the right ability picks the right
> data class). Author one data asset per interaction flavor and assign it on the ability
> Blueprint. The match between an ability and an object is by **gameplay tags**
> (`InteractionTags` on the data vs. the object's own `InteractableTags`).

<a id="53-ainteractableobj-base-interactable-actor"></a>
### 5.3 `AInteractableObj` (base interactable actor)
**Files:** `Public/Interactions/Actors/InteractableObj.h`, `Private/.../InteractableObj.cpp`

The base class for most world interactables. Implements `IInteractable`,
`IGameplayTagAssetInterface`, `IRestorable` and `ISavable`. It bundles mesh, an overlap
sphere, proximity/interaction VFX & SFX, an on-screen indicator, a shake component, an
optional dissolve-on-end, and full checkpoint persistence.

**Lifecycle:** the overlap sphere drives proximity feedback (sound/VFX fade in/out); the
ability calls `StartInteraction`/`EndInteraction`; `TriggerInteraction` allows chained or
non-player activation; objects can be one-shot (`bDestroyOnEnd` with an optional dissolve)
and broadcast `OnInteractionStarted` / `OnInteractionStopped` / `OnTriggeredBy`.

**Checkpoint/persistence:** the `UPROPERTY(SaveGame)` "Checkpoint…" fields snapshot
location/rotation/consumed/interacting/hidden/tags at a checkpoint;
`Reset()` rolls back to that snapshot, `SaveCheckpointState`/`ClearCheckpointState` manage
it, and `OnPostLoad_Implementation` restores after a disk load (Save System phase).

**Selected exposed variables (on the actor):**
| Group / Variable | Meaning |
|---|---|
| *Overlap Config* `CollisionChannels` | Channels that count as a valid interactor. |
| *Tags* `InteractableTags` | The object's identity tags (matched by abilities/resonance). |
| *Reset* `bResetObj` | Whether the object participates in checkpoint reset. |
| *Config* `bDestroyOnEnd` | Destroy/consume the object after the interaction. |
| *Dissolve* `DissolveCurve`, `DissolveParameterName`, `DissolveTargets`, `bDestroyActorOnDissolveFinished` | Material-dissolve effect when destroyed (shown only if `bDestroyOnEnd`). |
| *Highlight* `HighlightMaterial` | Overlay material when the object is interactable. |
| *Sound* `StartInteractionSound`, `EndInteractionSound`, `ProximitySound`, `FadeIn/OutDuration` | Audio feedback. |
| *VFX* `StartInteractionVFX`, `EndInteractionVFX`, `ProximityVFX`, `LinkedVFXTravelers` | Niagara feedback and linked spline-travel VFX triggers. |
| *Events* `OnInteractionStarted/Stopped/TriggeredBy` | Blueprint hooks. |

> 🎛️ **Designer:** the base interactable is configured **directly on the actor instance**,
> not via a data asset. The most important fields are `InteractableTags` (must match the
> interaction ability's tags), `CollisionChannels`, and the destroy/dissolve + feedback
> sections. Drop a `SaveIdComponent` is automatic — the object persists across checkpoints
> if `bResetObj` is on.

<a id="54-interactable-actor-catalog"></a>
### 5.4 Interactable actor catalog
All of the following extend `AInteractableObj` (unless noted) and add specific behavior:

| Actor | Base | Role |
|---|---|---|
| `ADrivableObj` | `AInteractableObj` | Object that can be "driven"/moved by an interaction (base for moving props). |
| `ATargetableDrivableObj` | `ADrivableObj` + `ITargetable` | Drivable object that can also be targeted/hit by the companion. |
| `APlatformObj` | `ADrivableObj` | Moving platform. |
| `APushObj` | `AInteractableObj` | Pushable object (uses push spots / IK). |
| `AFreeDragObj` | `AInteractableObj` | Object the player drags freely (free dragging status). |
| `ARailDragObj` | `AInteractableObj` | Object dragged constrained to a rail/spline. |
| `ARailMoveObj` | `AInteractableObj` | Object that moves along a rail by player and/or trigger (modes: Both / PlayerOnly / TriggerOnly / None-sound-only). |
| `ASequenceObj` | `ATargetableDrivableObj` | Targetable object tied to a sequence/ordered step. |
| `AOverlapHitObj` | `ATargetableDrivableObj` | Targetable object that reacts to overlap/hit. |
| `ACollectibleObj` | `AInteractableObj` | Collectible; on collect can Play Cinematic / Show UI Widget / both, then destroy. |
| `AChaosResonanceObj` | `AChaosCachePlayer` + interactable/restorable/savable | Resonance-breakable object backed by a Chaos cache (Intact → Breaking → Consumed). |
| `AResonancePuzzleManager` | `AActor` | Metronome/pattern puzzle manager (configurable step cycle, bell listener `UPuzzleBellListener`). |
| `AProjectile` | `AActor` | Projectile that can be deflected (parry / target system). |

> 🎛️ **Designer:** pick the actor class that matches the mechanic (push, free-drag,
> rail-drag, rail-move, platform, collectible, resonance) and configure it on the instance.
> Targetable variants (`*Drivable`, `Sequence`, `OverlapHit`) interact with the companion's
> sound wave; resonance objects (`ChaosResonanceObj`, the puzzle manager) respond to the
> Resonance ability (§4.6).

<a id="55-trigger-components-ubasetriggercomponent-family"></a>
### 5.5 Trigger components (`UBaseTriggerComponent` family)
**File:** `Public/Interactions/Components/BaseTriggerComponent.h` (+ `.cpp`)

An abstract `UBoxComponent` trigger volume implementing `IRestorable` + `ISavable`. It
filters overlaps by `AllowedCollisionChannels`, limits firing with `MaxTriggerCount`
(0 = unlimited), tracks `CurrentTriggerCount` (checkpoint-persisted), and exposes virtual
`OnTriggerEnter` / `OnTriggerExit` for subclasses. Concrete triggers:

| Component | Behavior |
|---|---|
| `UInteractableTriggerComponent` | Fires an interactable on overlap (modes: On Enter / On Exit / On Enter and Exit). |
| `UApplyGameplayEffectTriggerComponent` | Applies/removes a Gameplay Effect on overlap (Apply+Remove / Apply only / Remove only). |
| `UTeleportTriggerComponent` | Teleports the entering actor. |
| `UCompanionLightTriggerComponent` | Trigger reacting to / driving the companion light. |
| `UBlueprintTriggerComponent` | Exposes Enter/Exit events to Blueprint for custom logic. |

<a id="56-helper-components"></a>
### 5.6 Helper components
| Component | Role |
|---|---|
| `UPushSpotComponent` (`Public/.../Components/PushSpotComponent.h`) | A scene component marking where the player stands to push, exposing left/right-hand socket transforms for hand IK. |
| `UTargetSpot` (`USphereComponent`) | A targetable hot-spot on an object (companion aim/hit target). |
| `UCompanionInteractionSpotComponent` | Defines how the companion behaves at a spot (Attach To Spot / Orbit Around / Follow Spline). |
| `UInteractableShakeComponent` | Procedural shake/vibration feedback for an interactable (configurable modes, core + duration settings, clean reset). |
| `UInteractableLinkTimerComponent` (`IRestorable` + `ISavable`) | Time-window link between interactions/hits (filters: Any / Interaction Only / Hit Only). |
| `AInteractableRail` (`Public/.../Utils/InteractableRail.h`) | A spline actor used as a rail for `ARailDragObj`/`ARailMoveObj`; can snap to floor (`bSnapToFloor`, `FloorOffset`). |

> 🔧 **Developer:** the trigger family centralizes overlap filtering, max-fire counting and
> checkpoint persistence in `UBaseTriggerComponent` — new trigger types should subclass it
> and implement `OnTriggerEnter`/`OnTriggerExit` rather than re-binding overlaps. Helper
> spots (`PushSpot`, `TargetSpot`, `CompanionInteractionSpot`) decouple the *where/how* of
> an interaction from the actor's own logic, so the same actor can expose multiple anchors.

<a id="phase-6"></a>
## 6 — Camera

StillHear uses a **volume-based camera** system: cameras live in the level as
`ACameraVolume` actors, and the player's presence inside their boxes decides which camera
is active. On top of that, a **camera-effects** layer (shake / FOV pulse / positional
offset) plays transient juice driven by gameplay (parry, resonance, etc.).

The runtime wiring was already covered from the consumer side in Phase 1: the character
tracks overlapping volumes (`ICameraVolumesInteractor`) and the controller blends between
them (`ChangeCamera`, §1.3). This section documents the camera classes themselves.

<a id="61-icameravolumesinteractor-publicinterfacescameravolumesinteractorh"></a>
### 6.1 `ICameraVolumesInteractor` (`Public/Interfaces/CameraVolumesInteractor.h`)
The contract the player implements so volumes can register themselves:
`AddCameraVolumeToList`, `RemoveCameraVolumeFromList`, `GetTargetPointLocation`
(all `BlueprintNativeEvent`). The character's implementation keeps the prioritized volume
list (§1.4).

<a id="62-acameravolume-abstract-base"></a>
### 6.2 `ACameraVolume` (abstract base)
**Files:** `Public/Camera/CameraVolume.h`, `Private/Camera/CameraVolume.cpp`

A camera placed in the world, with an inner `Volume` box (the trigger), an `OuterVolume`
box, a `CameraComponent`, and an `InputArrow` that defines the movement basis. Both boxes
overlap-only on all channels; `BeginOverlap`/`EndOverlap` add/remove this volume from the
player's list.

**Key responsibilities:**
- **Activation:** `Activate(Actor)` / `Deactivate()` mark the volume as the one currently
  driving the view (enables ticking only while active — a deliberate perf choice).
- **Target tracking:** `UpdateCamera(TargetPoint, DeltaTime)` is the virtual the subclasses
  override to position the camera each frame relative to the player's target point.
- **Snapping vs blending:** `RequestSnapToTarget()` makes the next update jump with no
  interpolation (used at spawn/respawn/cutscene-end); otherwise the controller blends using
  the volume's blend parameters.
- **Priority:** `GetPriority()` — when the player is in several volumes, the highest priority
  wins (§1.4 `CheckList`).
- **Input basis:** `GetRightDirection()` returns the `InputArrow`'s right vector, which the
  controller uses as the movement basis (§1.3). `bInputFollowsCamera` + `InputAdjustingTime`
  control how the input realigns when switching volumes.

**Exposed variables (designer):**
| Group / Variable | Meaning |
|---|---|
| *CameraBlend* `BlendFunction`, `BlendExp` | View-target blend curve and exponent. |
| *CameraBlend* `BlendTimeOnEnter` | Blend duration when entering this volume. |
| *CameraBlend* `bUseBlendParametersOnExit`, `BlendTimeOnExit` | Whether/how long to blend when leaving (the *exiting* volume's params can drive the transition). |
| *Configuration* `Priority` | Higher wins when multiple volumes overlap. |
| *CameraRotation* `bLookAtPlayer`, `RotationSpeed` | Optionally rotate the camera to face the player. |
| *Input* `bInputFollowsCamera`, `InputAdjustingTime` | Whether movement input realigns to the camera and how long the realign lerp takes. |

> 🎛️ **Designer:** place camera volumes to author the cinematic framing of an area. The two
> levers you'll use most are **`Priority`** (to layer a tighter camera inside a broader one)
> and the **blend** settings (enter/exit time + function) for smooth hand-offs. Point the
> `InputArrow` to set "which way is right" for the player's stick while in this volume; set
> `bInputFollowsCamera` if the controls should re-orient as the camera moves.

<a id="63-spline-follow-cameras"></a>
### 6.3 Spline-follow cameras

#### `AFollowSplineCamera`
**Files:** `Public/Camera/CameraTypes/FollowSplineCamera.h`, `Private/.../FollowSplineCamera.cpp`

A camera volume whose `CameraComponent` slides along a `Spline` as the player moves. Each
frame `UpdateCamera` finds the spline point closest to the player's target point (plus
`OffsetAlongSpline`), computes `DesiredCameraLocation` (+ `LocationOffset`), and
`ApplyPosition` interpolates the camera toward it over `TimeToReachTargetPoint` (or snaps if
a snap was requested). Use it for side-scrolling/rail-like sections where the camera should
track along an authored path.

| Variable | Meaning |
|---|---|
| `TimeToReachTargetPoint` | Smoothing time for the camera to catch up along the spline. |
| `OffsetAlongSpline` | Shifts the camera's spline position forward/back along the path. |
| `LocationOffset` | Constant world offset added to the spline point. |

#### `AFollowSplineMaintainDistanceCamera`
**Files:** `Public/.../FollowSplineMaintainDistanceCamera.h`, `Private/.../FollowSplineMaintainDistanceCamera.cpp`

Extends the spline camera to also hold a target **distance** from the player while clamping
height. Adds `DistanceFromActor`, `MaxHeight`/`MinHeight`, and `ShowDesiredLocationHeight`
(debug).

> 🎛️ **Designer:** choose the plain `FollowSplineCamera` when the camera should ride a fixed
> path; choose `FollowSplineMaintainDistanceCamera` when it should ride the path *but* keep a
> consistent distance/height from the player (e.g. a chase that breathes with the action).
> Draw the spline through the area; tune `TimeToReachTargetPoint` for how "laggy"/snappy the
> follow feels.

<a id="64-camera-effects-layer"></a>
### 6.4 Camera effects layer

A self-contained "camera juice" system: presets bundle three independently-optional layers —
**Shake**, **FOV pulse**, **positional offset** — each playable one-shot or sustained.

#### Types (`Public/Camera/CameraEffects/CameraEffectTypes.h`)
- **`FCameraShakeConfig`** — a `UCameraShakeBase` class + `Scale`, optional force feedback
  (loopable), and local-vs-world placement (with epicenter / inner-outer radius / falloff for
  world shakes).
- **`FCameraFOVConfig`** — `FOVDelta`, optional `PulseCurve` (else linear snap-and-return),
  and `bKeepFOV` (sustained until stopped).
- **`FCameraOffsetConfig`** — `KickDirection`, `SpringSpeed` (spring-back), `MaxDuration`,
  and `bKeepOffset` (sustained).
- **`FCameraEffectPreset`** — the bundle: `bPlayShake/FOV/Offset` toggles each sub-config.

#### `UCameraEffectsComponent`
**Files:** `Public/.../CameraEffectsComponent.h`, `Private/.../CameraEffectsComponent.cpp`

Lives on the **player controller** (`CameraEffectsComponent`, §1.3). It plays presets and
manages active pulses each tick. API:
| Method | Purpose |
|---|---|
| `PlayEffectPreset(Preset, CustomEpicenter)` | Play every enabled layer of a preset. |
| `PlayCameraShake(Config, CustomEpicenter)` | Shake only. |
| `PlayFOV(Config)` / `PlayOffset(Config)` | FOV-pulse / offset-pulse only (each interrupts a running one of its kind). |
| `StopLoopingShake()` / `StopAllEffects()` | Stop the looping shake / restore camera fully. |

It distinguishes **pulses** (transient, tracked in `ActiveFOVPulses` / `ActiveOffsetPulses`)
from **sustained** effects (`bKeepFOV`/`bKeepOffset`, persist until `StopAllEffects`). FOV and
offset are applied through dedicated **camera modifiers** rather than overwriting the camera
directly:
- **`UCameraFOVModifier`** — adds `FOVDeltaToApply` in `ModifyCamera`.
- **`UCameraOffsetModifier`** — adds `OffsetToApply` (world-space) in `ModifyCamera`.

Using modifiers keeps the base camera value intact, so multiple systems can stack safely.

#### Access helpers
- **`UCameraEffectsBlueprintLibrary::GetCameraEffects(WorldContext)`** — static getter to
  fetch the player's `UCameraEffectsComponent` from anywhere in Blueprint.
- **`UCameraEffectsPresetData`** — a `UDataAsset` wrapping one `FCameraEffectPreset`, so
  presets can be authored once as assets and reused.

> 🎛️ **Designer:** author reusable camera-juice as `FCameraEffectPreset` (inline on an
> ability's data asset — e.g. `UParryData::EffectPreset`, `UResonanceData::*EffectPreset` —
> or as a shared `UCameraEffectsPresetData` asset). Each preset independently enables shake,
> FOV pulse and offset, so one asset can do "small kick + slight FOV punch". Use `bKeepFOV` /
> `bKeepOffset` for states that should hold the effect until explicitly stopped (e.g. while
> resonance is active), and the plain pulses for instantaneous hits.

> 🔧 **Developer:** to trigger juice from gameplay, get the component off the player
> controller (`GetCameraEffectsComponent()`, or the BP library) and call `PlayEffectPreset`.
> FOV/offset go through `UCameraModifier`s registered on the player camera manager — don't
> set FOV/location on the camera directly or you'll fight the modifiers.

<a id="65-astillhearcamera--legacy--unused"></a>
### 6.5 `AStillHearCamera` — *legacy / unused*
**Files:** `Public/Camera/StillHearCamera.h`, `Private/Camera/StillHearCamera.cpp`

A standalone follow-camera actor (target focus point, floor distance/height, lead/lag
speeds, screen-bounds tolerance) that predates the volume system. It is **only referenced by
its own files** — the active camera flow is entirely volume-based — so treat it as legacy.
Documented for completeness; not part of the current pipeline.

<a id="phase-7"></a>
## 7 — Companion

The companion is the floating creature that accompanies the player. **It is implemented as
a component on the main character** — `UFloatingCompanionComponent` (the `CompanionComponent`
subobject, §1.4) — not as a separate pawn. It handles its own mesh, floating motion,
obstacle avoidance, emotional states and light, while its offensive **Sound Wave** ability
lives in GAS and is driven from the player input (§1.3).

> 🔧 **Developer:** an older standalone companion *character* class exists in the codebase
> but is **deprecated and unused**; all live companion behavior is the component documented
> here. Do not wire new logic to the legacy character.

<a id="71-ufloatingcompanioncomponent"></a>
### 7.1 `UFloatingCompanionComponent`
**Files:** `Public/Character/FloatingCompanionComponent.h`, `Private/Character/FloatingCompanionComponent.cpp`

A `UActorComponent` that spawns and drives the companion mesh relative to its owner. Each
tick it computes a target location (base offset + bob + orbit, crouch-aware), smooths toward
it, avoids obstacles, updates morph/visuals, and (optionally) polls the environment.

#### Emotional states
**`ECompanionState`:** `Idle`, `Scared`, `Happy`, `Angry`, `Interactable` (interactable
nearby), `Custom`. Each state maps to an **`FCompanionStateConfig`** (the `StateConfigs`
map) describing how the companion looks and moves in that state:

| `FCompanionStateConfig` field | Meaning |
|---|---|
| `MorphTargetName` / `MorphTargetValue` / `MorphBlendSpeed` | Facial/shape morph to drive (0–1) and blend speed. |
| `BobAmplitude` / `BobFrequency` | Vertical bobbing oscillation. |
| `OrbitAngleOffset` / `OrbitRadius` / `OrbitSpeed` | Orbit around the owner (0 speed = no orbit). |
| `Color` | State tint (applied via the dynamic material's emissive). |
| `MeshScale` | Per-state mesh scale. |

**API:** `SetCompanionState(State)` (blended transition), `GetCurrentState()`,
`SetCustomState(Config)` (force a runtime config), `GetCurrentLocation()`,
`ResetCompanionLocation()` (snap to owner — used by `Revive`, §1.4).

#### Floating, follow & obstacle avoidance
The companion follows a smoothed target (`FollowSpeed`) at `BaseOffset` from the owner,
dropping by `CrouchHeightOffset` when the owner crouches. When `bPreventClipping` is on, a
periodic sphere trace (`CollisionSphereRadius`, every `ObstacleCheckInterval`) solves
avoidance: it tries to deflect along the surface, then escapes vertically
(`VerticalEscapeHeight`), smoothing toward the avoidance target (`AvoidanceSmoothSpeed`). If
it stays stuck beyond `StuckDistanceThreshold` for `StuckTeleportTime` seconds, it teleports
to the player; if it lags beyond `MaxLagDistance`, a catch-up boost kicks in.

#### Environment polling
With `bAutoPollingEnabled` (toggle via `SetAutoPollingEnabled`), every `PollingInterval`
seconds it searches `EnemyCollisionChannels` within `EnemyDetectionRadius` for enemies
(`IsEnemyNearby`) and can react (e.g. switch to a scared/angry state).

#### Light
An optional `CompanionLight` (point light). `SetLightEnabled(bEnabled, Color, bUpdateColor,
Intensity)` toggles it; `bSyncLightColorWithState` ties its color to the state color, and
when lit the companion floats `LightOnForwardDistance` in front of the owner to light the path.

#### Interaction spots
`EngageInteractionSpot(Spot)` / `DisengageInteractionSpot()` / `ClearInteractionSpot()` make
the companion adopt a scripted behavior at a `UCompanionInteractionSpotComponent` (§7.4).

**Selected exposed variables (grouped):**
| Group | Variables |
|---|---|
| *Setup/Components* | `CompanionMeshAsset`, `CompanionMesh`, `CompanionLight` |
| *States* | `StateConfigs` (per-state `FCompanionStateConfig`) |
| *Float* | `BaseOffset (0,0,120)`, `FollowSpeed (8)`, `CrouchHeightOffset (-60)` |
| *Collision* | `bPreventClipping`, `CollisionSphereRadius`, `ObstacleCheckInterval`, `StuckDistanceThreshold`, `StuckTeleportTime`, `MaxLagDistance`, `VerticalEscapeHeight`, `AvoidanceSmoothSpeed` |
| *Polling* | `EnemyDetectionRadius (600)`, `PollingInterval (0.5)`, `EnemyCollisionChannels` |
| *Material* | `EmissiveParamName ("EmissiveColor")` |
| *Light* | `bSyncLightColorWithState`, `bIsLightEnabled`, `LightOnForwardDistance (200)` |

> 🎛️ **Designer:** the companion is configured **directly on the component** (on the player
> Blueprint). Author one `FCompanionStateConfig` per `ECompanionState` in `StateConfigs` to
> give each emotion its look (morph, color, scale) and motion (bob, orbit). Tune the follow
> feel with `BaseOffset` / `FollowSpeed`, and the avoidance robustness with the *Collision*
> group. Assign `CompanionMeshAsset`; the mesh is auto-created if left null.

<a id="72-uga_soundwave--data-driven-usoundwavedata"></a>
### 7.2 `UGA_SoundWave` — *data-driven* (`USoundWaveData`)
**Files:** `Public/.../Abilities/Companion/GA_SoundWave.h`, `Private/.../GA_SoundWave.cpp`
**Data Asset:** `Public/Data/DataAssets/SoundWaveData.h`

The companion's offensive ability: charge → aim/lock a target → shoot a (homing) projectile
that also **emits an AI noise event**. Adds `Status.Companion.Aiming` while active.

**Flow:**
1. **Activate (charge + aim):** caches the player controller, the companion component and the
   camera-effects component; applies the **charge** effect (duration set by caller from the
   charge sound length) and its cue; plays the charge camera preset; sets the companion to
   `Angry`; applies a **speed multiplier** to the player (slows them while aiming); and starts
   target search. A 0.1 s timer (`UpdateTargets`) keeps the on-screen target list fresh as the
   player/camera move. Two gameplay-event tasks wait for **switch-target** and **shoot**. A
   looping aim rumble plays.
2. **Targeting:** `GetValidTargetsOnScreen` overlaps `TargetingRadius`, keeps actors
   implementing `ITargetable` that are **on screen**, sorts by distance, and targets the
   nearest (`SetTargeted`). `UpdateTargets` adds/removes targets as they enter/leave screen,
   re-picking the closest if the current one is lost.
3. **Switch target:** on the switch event (a 2D stick direction from the controller, §1.3),
   it picks the target best aligned with that direction **in screen space** (dot-product vs
   `TargetSwitchThreshold`), throttled by `TargetSwitchCooldown`, and moves the target marker.
4. **Shoot:** spawns `ProjectileClass` from the companion's location toward the locked target
   (or straight ahead if none), with optional **homing** (`bEnableHoming`: pitch arc mapped
   between `MinHomingDistance`/`MaxHomingDistance`, homing toward the target's `UTargetSpot`
   or root with `HomingAcceleration`). The projectile carries `ShootNoiseLoudness` /
   `ShootNoiseRange` so it reports a hearing event to AI. Plays the shoot cue/preset/rumble,
   commits the cooldown, and ends.
5. **End/cancel:** removes charge/aim/speed effects, untargets, stops the aim rumble, sets the
   companion to `Happy`. `ApplyCooldown` uses `AbilityCooldown` via the
   `Data.SoundWaveCooldown` SetByCaller.

**`USoundWaveData` fields (grouped):**
- **Camera presets:** `Charge / Aim / Shoot EffectPreset` (`FCameraEffectPreset`, §6.4).
- **Projectile:** `ProjectileClass`, `OffsetProjectileSpawn (100)`.
- **Homing:** `bEnableHoming`, `MinHomingDistance (300)`, `MaxHomingDistance (1500)`,
  `MaxHomingPitchOffset (45)`, `HomingAcceleration (5000)`.
- **Targeting:** `TargetingRadius (500)`, `TargetSwitchThreshold (0.4)`,
  `TargetSwitchCooldown (0.3)`.
- **Effects:** `ChargeEffectClass`, `AimEffectClass`, `SpeedEffectClass`.
- **Tuning:** `AbilityCooldown (5)`, `SpeedMultiplier (0.5)` (player slow while aiming).
- **Sounds:** `Charge / Shoot / Aim / FinishCooldown`.
- **Feedback:** `ActiveAimForceFeedback` (looping), `ShootForceFeedback`.
- **AI Noise:** `ShootNoiseLoudness (1.0)`, `ShootNoiseRange (0 = perception default)`.

> 🎛️ **Designer:** the whole sound-wave feel is in `USoundWaveData` — projectile + homing
> arc, targeting radius/switch responsiveness, the aiming slow-down (`SpeedMultiplier`),
> cooldown, all camera presets, sounds, rumbles, and how loud the shot is to enemies
> (`ShootNoiseLoudness/Range`, which feeds the stealth loop). The projectile itself is
> `AProjectile` (§5.4).

<a id="73-utargetmarkerniagaracomponent"></a>
### 7.3 `UTargetMarkerNiagaraComponent`
**File:** `Public/VFX/TargetMarkerNiagaraComponent.h` (+ `.cpp`)

A `UNiagaraComponent` that renders the reticle/marker over a targetable. It pushes
designer-set user parameters into the Niagara system (`User.Color`, `User.BaseSize`,
`User.MarkerSize`, `User.BaseOffset`, `User.MarkerOffset`) and dynamically places the marker
at the top of the actor.

| Variable | Default | Meaning |
|---|---|---|
| `Color` | White | Marker tint. |
| `BaseSize` / `MarkerSize` | `250` / `150` | Base ring / inner marker size. |
| `BaseZOffset` / `MarkerZOffset` | `50` / `50` | Vertical offsets. |

> 🎛️ **Designer:** drop this on a targetable actor (or its `UTargetSpot`) to show the lock-on
> marker; tune size/offset/color here, and the visuals in the referenced Niagara system.

<a id="74-ucompanioninteractionspotcomponent"></a>
### 7.4 `UCompanionInteractionSpotComponent`
**File:** `Public/Interactions/Components/CompanionInteractionSpotComponent.h` (+ `.cpp`)

A scene component placed on an interactable that tells the companion how to behave there
(`EngageInteractionSpot`, §7.1). Behavior (`ECompanionBehavior`): **AttachToSpot**,
**OrbitAround**, or **FollowSpline**.

| Group / Variable | Meaning |
|---|---|
| `BehaviorType` | Attach / Orbit / Follow-spline. |
| `InteractionVFX`, `CompanionVFXSocket` | VFX spawned on engage and the companion socket to attach it. |
| `InteractionFollowSpeed` | Interp speed toward the spot. |
| *Orbit* `OrbitRadius`, `OrbitSpeed` | Orbit params (shown only for OrbitAround). |
| *Spline* `SplineNameTag`, `bPingPongSpline`, `SplineProgressCurve` | Spline to follow, ping-pong toggle, and a time→distance(0–1) progress curve (its max time = travel duration). |

> 🎛️ **Designer:** use these spots to choreograph the companion during scripted moments
> (sit on a spot, orbit an object, or travel a spline). `EditCondition` hides the
> orbit/spline fields unless the matching behavior is selected.

> 🔧 **Developer:** targeting relies on the `ITargetable` interface (§5.1) and `UTargetSpot`
> (§5.6) for the precise homing/marker anchor. The sound-wave projectile and the parry both
> apply effects to enemies via tags, keeping the companion offense consistent with the rest
> of the combat layer.

<a id="phase-8"></a>
## 8 — Audio

Audio is a pillar of StillHear (the game is literally about sound), and the audio code is
organized in layers:

1. **`UGameAudioSubsystem`** — the central runtime: music/ambience playback, per-category
   volumes, ducking, and a **state stack** that swaps the soundtrack as the game situation
   changes.
2. **State-driven audio** — any actor can request an audio state via `UAudioStateComponent`,
   aggregated by the `UStillHearAudioObserver` world subsystem and resolved against
   `UAudioStateConfig`.
3. **Settings & config** — `USoundDeveloperSettings` (Project Settings) plus the
   `UAudioGameConfig` data asset wire up the sound classes/mixes; `FSoundLevels` holds the
   user's volume sliders.
4. **DSP / world dynamics** — `WorldStereoBalanceSubmix` (custom submix effect) and
   `UAudioDistanceTuningData` (distance-based world volume, muffle and noise).
5. **Footsteps** — `UFootStepData` maps physical surfaces to footstep sound/VFX (driven by
   AnimNotifies, §9), feeding both feedback and the AI noise loop.
6. **Flow integration** — Flow nodes to play/stop music/ambience and set audio states from
   level scripting (§13).

<a id="81-ugameaudiosubsystem"></a>
### 8.1 `UGameAudioSubsystem`
**Files:** `Public/Audio/GameAudioSubsystem.h`, `Private/Audio/GameAudioSubsystem.cpp`

A `UGameInstanceSubsystem` that owns all non-spatial audio. It keeps persistent music and
ambience `UAudioComponent`s, the user's `FSoundLevels`, a runtime `USoundMix`, references to
each sound class (Master/Music/Ambience/SFX/UI/Voice/SFX_Gameplay), and the audio-state stack.

**Music & ambience:** `PlayMusic` / `CrossfadeMusic` / `StopMusic` and the matching
`*Ambience` calls (with fade in/out and volume). Internally `EnsureMusicComponent` /
`EnsureAmbienceComponent` keep one persistent component each.

**Volumes:** getters/setters per category (`Get/SetMasterVolume`, `Music`, `Ambience`, `SFX`,
`UI`, `Voice`), applied at runtime via `ApplyClassVolumeRuntime` on a runtime sound mix and
mirrored to the save settings (`SetVolumeFromSaveSettings`). Broadcasts `OnLevelChanged`
(`FSoundLevelDelegate`, §8.6) so settings UI updates live.

**Dynamics / ducking:** `SetWorldSFXDynamics` scales world SFX; `SetMusicDucking(Multiplier,
FadeTime)` ducks the music (used on pause, §1.3) with `PauseDuckMultiplier` / `DuckFadeTime`
(from Project Settings).

**Audio-state stack:** `PushAudioState(Tag)` / `PopAudioState(Tag)` / `ClearAudioStack()`
manage a priority stack of `FGameplayTag` states; `RefreshAudioState` evaluates the top state
and crossfades music/ambience accordingly. `GetCurrentAudioState`, `GetCombatStateTag`, and
the stack get/set helpers (used for save persistence) round it out. `OnWorldLoaded` keeps the
mix valid across level loads.

> 🔧 **Developer:** play music/ambience and switch moods through this subsystem rather than
> spawning raw audio components. The **state stack** is the main mechanism: push a state when
> entering combat/menu/area, pop it when leaving; the highest-priority state on the stack
> decides the track (see §8.2).

<a id="82-state-driven-audio"></a>
### 8.2 State-driven audio

#### `UAudioStateConfig` (Data Asset)
**File:** `Public/Audio/AudioStateConfig.h`

Maps gameplay tags to **`FAudioStateInfo`** entries. Each state defines its music/ambience
tracks and how it behaves:
| `FAudioStateInfo` field | Meaning |
|---|---|
| `StateTag` | The tag identifying this state (e.g. `Audio.State.MainMenu`, `…Combat`). |
| `Priority` | Higher priority overrides lower when multiple states are active. |
| `bOverrideMusic` / `bOverrideAmbience` | Whether this state changes music / ambience. |
| `MusicVolumeMultiplier` / `AmbienceVolumeMultiplier` | Per-state volume scale. |
| `bPersistAcrossLevels` | Survive level changes. |
| `bIsUISound` | Keep playing while paused. |
| `MusicTrack` / `AmbienceTrack` | The tracks (soft refs). |
| `FadeInTime` / `FadeOutTime` | Transition times. |

#### `UAudioStateComponent` + `UStillHearAudioObserver`
**Files:** `Public/Audio/AudioStateComponent.h`, `Public/Audio/StillHearAudioObserver.h`

A reusable component (`UAudioStateComponent`) that any actor can carry to **request** an audio
state (`AudioStateTag`), either when the player overlaps it (`bAutoDetectOverlap`) or manually
(`SetAudioStateActive`). All requests are collected by `UStillHearAudioObserver` (a
`UWorldSubsystem`), which tracks active requests per source and `RefreshGlobalAudioState` to
push/pop the right tags on the `UGameAudioSubsystem`. This decouples "where the music changes"
(level design) from the subsystem.

> 🎛️ **Designer:** to make an area change the music/ambience, drop a `UAudioStateComponent`
> on a trigger actor, set its `AudioStateTag`, and define that tag's tracks/priority in the
> `UAudioStateConfig`. Use `Priority` to layer states (combat over exploration), and
> `bPersistAcrossLevels` / `bIsUISound` for menu/global music.

<a id="83-settings--config"></a>
### 8.3 Settings & config

#### `USoundDeveloperSettings` (Project Settings → StillHear → Game Sound)
**File:** `Public/Audio/SoundDeveloperSettings.h`
A `UDeveloperSettings` (config `GameSounds`) holding the canonical references: each sound
class (Master/Music/Ambience/SFX/UI/Voice/SFX_Gameplay), the world submix `SM_World`, the
runtime settings mix, the `AudioStateConfig`, the `DefaultLevels`, and the ducking params
(`DuckFadeTime`, `PauseDuckMultiplier`). This is the project-wide source of truth the
subsystem loads at init.

#### `UAudioGameConfig` (Data Asset)
**File:** `Private/Audio/AudioGameConfig.h`
A data-asset bundle of sound classes (Master/Music/SFX/UI) and sound mixes (Gameplay / Menu /
RunTimeSettings) used by the subsystem.

#### `FSoundLevels`
The six 0–1 volume sliders (Master, Music, Ambience, SFX, UI, Voice). Stored in the save
settings and in `USoundDeveloperSettings::DefaultLevels`.

> 🎛️ **Designer:** set the global audio wiring once in *Project Settings → StillHear → Game
> Sound* (sound classes, world submix, default levels, pause ducking). Per-game tuning of
> classes/mixes can also live in the `UAudioGameConfig` asset.

<a id="84-worldstereobalancesubmix-custom-dsp"></a>
### 8.4 `WorldStereoBalanceSubmix` (custom DSP)
**Files:** `Public/Audio/WorldStereoBalanceSubmix.h`, `Private/Audio/WorldStereoBalanceSubmix.cpp`

A custom **submix effect** that re-balances the stereo field of whatever is routed through it.
- **`FWorldStereoBalanceSubmixSettings`** — `Pan` (-1 left … +1 right) and `Strength` (0–1).
- **`FWorldStereoBalanceSubmix`** — the audio-thread effect; in `OnProcessAudio` it applies
  smoothed equal-power L/R gains (`ComputeEqualPowerGains`) toward the target pan/strength.
- **`UWorldStereoBalanceSubmixPreset`** — the main-thread/editor preset, with `SetSettings`
  to push new values at runtime.

> 🎛️ **Designer/Audio:** route world audio through this submix to dynamically pan the mix
> (e.g. shift the world to one ear during a scripted moment). `Strength` controls how strong
> the rebalancing is; the effect smooths transitions so changes don't pop.

<a id="85-uaudiodistancetuningdata-data-asset"></a>
### 8.5 `UAudioDistanceTuningData` (Data Asset)
**File:** `Public/Audio/AudioDistanceTuningData.h`

Tunes **distance-based world audio dynamics**: as the relevant source/listener distance moves
through thresholds `A`/`B`/`C`, the world volume is mapped between `WorldMin`/`WorldMax`, a
`NoiseSound` fades in between B→C (`NoiseAtB`→`NoiseAtC`), and a low-pass "muffle" submix
preset (`LPF_Preset_On` on `SM_World`) can engage. It also carries the runtime routing
(`SMX_WorldDynamics`, `SC_WorldDynamics`) and smoothing params (`UpdateInterval`,
`WorldInterpSpeed`, `NoiseInterpSpeed`, `MuffleFadeTime`, `VolumeEpsilon`).

> 🎛️ **Designer/Audio:** use this asset to author "how the world sounds with distance" —
> the three distance bands, the min/max world volume, the noise bed that grows far away, and
> the muffle. Tune the smoothing so transitions feel natural.

<a id="86-fsoundleveldelegate-publicaudiosounddelegatesh"></a>
### 8.6 `FSoundLevelDelegate` (`Public/Audio/SoundDelegates.h`)
A dynamic multicast delegate `(USoundClass*, float)` broadcast when a level changes, so UI or
any responder can react instantly. The subsystem's `OnLevelChanged` uses it.

<a id="87-footsteps--ufootstepdata-data-asset"></a>
### 8.7 Footsteps — `UFootStepData` (Data Asset)
**File:** `Public/Data/DataAssets/FootStepData.h`

Maps each `EPhysicalSurface` to an **`FFootstepEffects`** (a `Sound` + a Niagara `VFX`), with
a `DefaultEffects` fallback. Helpers `GetSoundForSurface` / `GetVFXForSurface` resolve the
right effect for the surface under the foot. Assigned on the player as `FootstepConfig`
(§1.4); the surface is detected via `DetectFloorType()` (§1.4) and the effect is triggered by
the footstep AnimNotify (§9).

| Field | Meaning |
|---|---|
| `SurfaceEffectsMap` | `EPhysicalSurface` → `{ Sound, VFX }`. |
| `DefaultEffects` | Fallback when the surface isn't in the map. |

> 🎛️ **Designer:** add one entry per physical surface (grass, soil, stone…) with its
> footstep sound and particle. The surface comes from the floor's **Physical Material**, so
> make sure level materials carry the right surface type. Footsteps also feed the **stealth
> loop**: certain surfaces report louder noise to enemies (§1.4 `Landed`, §11).

<a id="88-audio-flow-nodes"></a>
### 8.8 Audio Flow nodes
The Flow graph can drive audio directly (details in §13): `UFlowNode_PlayMusic`,
`UFlowNode_PlayAmbience`, `UFlowNode_StopAudio`, `UFlowNode_SetAudioState`. They forward to the
`UGameAudioSubsystem` so cinematics/level scripting can change the soundtrack without code.

<a id="phase-9"></a>
## 9 — Animation

This section covers the **player** animation layer: the main character's `AnimInstance` and
the family of `AnimNotify` / `AnimNotifyState` classes that fire gameplay/audio/AI events from
animation timelines. Enemy AnimInstances (Worm, Mantis) are documented with the AI in §11; the
companion AnimInstance is part of the deprecated companion path and is excluded.

<a id="91-umaincharacteraniminstance"></a>
### 9.1 `UMainCharacterAnimInstance`
**Files:** `Public/Animation/AnimInstances/MainCharacterAnimInstance.h`, `Private/.../MainCharacterAnimInstance.cpp`

The player's AnimInstance. It feeds the AnimGraph with movement data each frame and implements
`IIKTargetReceiver` for hand IK (climbing edge-grab and dragging).

**Per-frame data (`NativeUpdateAnimation`):**
- `Velocity`, `GroundSpeed` (planar), `Direction` (via `CalculateDirection`), `bShouldMove`
  (`GroundSpeed > 3`).
- `bIsFalling`, `bIsCrouching` from the movement component.
- `bIsSprinting` / `bIsDragging` read from GAS tags (`…Sprint_Active`,
  `Status.FreeDragging` / `Status.RailDragging`).
- **IK alpha:** `IKTraceAlpha` interpolates toward 1 while dragging (blend in at
  `IKBlendInSpeed`), and toward 0/1 otherwise depending on `bIsClimbing` (blend out at
  `IKBlendOutSpeed`).

**State flags set by gameplay code** (read by the AnimGraph): `bIsClimbing` (§4.4),
`bIsSitting` (§1.4), plus `LocomotionBlendSpace` (swappable, §1.4).

**IK targets:** `UpdateIKTargets(Left, Right)` stores `LeftEffectorTransform` /
`RightEffectorTransform` — called by the controller's edge-grab trace (§1.3) and consumed by
the AnimGraph's Two-Bone IK at strength `IKTraceAlpha`.

| Exposed (selected) | Meaning |
|---|---|
| `Velocity`, `GroundSpeed`, `Direction`, `bShouldMove` | Locomotion inputs for the blend space. |
| `bIsFalling`, `bIsCrouching`, `bIsSprinting`, `bIsClimbing`, `bIsDragging`, `bIsSitting` | State booleans driving transitions. |
| `LocomotionBlendSpace` | The active locomotion blend space. |
| `IKTraceAlpha`, `IKBlendInSpeed (10)`, `IKBlendOutSpeed (15)` | Hand-IK weight and blend speeds. |
| `LeftEffectorTransform`, `RightEffectorTransform` | World IK targets for the hands. |

> 🔧 **Developer / animator:** the AnimInstance is deliberately thin — gameplay code sets the
> high-level flags and IK targets, the AnimBlueprint does the actual blending. The dragging/
> climbing IK is driven externally (controller traces → `UpdateIKTargets`) and weighted by
> `IKTraceAlpha`, so the same two-bone IK node serves both edge-grab and drag.

#### `IIKTargetReceiver` (`Public/Interfaces/IKTargetReceiver.h`)
A one-method interface (`UpdateIKTargets(Left, Right)`) that decouples the trace producer
(controller) from the IK consumer (AnimInstance), so the controller doesn't need to know the
concrete AnimInstance type.

<a id="92-animnotifies--footsteps--floor"></a>
### 9.2 AnimNotifies — footsteps & floor

#### `UAN_Footstep` — *data-driven*
**File:** `Public/Animation/AnimNotifies/AN_Footstep.h` (+ `.cpp`)
The current footstep notify. On the character's primary mesh it detects the surface
(`DetectFloorType`, §1.4), then plays the sound **and** Niagara VFX from the character's
`UFootStepData` (§8.7) at the foot socket (`SocketName`). This is the single source of footstep
feedback going forward.
| Variable | Meaning |
|---|---|
| `SocketName` | Foot socket for the play location (falls back to actor location). |

<a id="93-animnotifies--ai-noise-stealth-loop"></a>
### 9.3 AnimNotifies — AI noise (stealth loop)

#### `UAN_ReportNoiseEvent` (base)
**File:** `Public/.../AN_ReportNoiseEvent.h` (+ `.cpp`)
Reports a hearing stimulus to the AI perception system (`UAISense_Hearing::ReportNoiseEvent`)
at the owner's location, with a `Tag` so enemies can react to the *kind* of noise.
| Variable | Default | Meaning |
|---|---|---|
| `Loudness` | `1.0` | Noise loudness (0–1). |
| `NoiseRange` | `0` | Max hearing range (0 = perception default). |
| `Tag` | None | Identifies the noise type (e.g. `Walk`, `Run`, `Vibration.Run`). |

#### `UAN_ReportFoostepNoiseEvent` (derived)
**File:** `Public/.../AN_ReportFoostepNoiseEvent.h` (+ `.cpp`)
Specializes the base for footsteps: it ignores steps below `MinSpeedThreshold`, then sets the
`Tag` from the movement state — `Crouch` / `Run` / `Walk` — and **prefixes `Vibration.`** when
the surface is `Soil` (so soil transmits a stronger, distinct stimulus), before delegating to
the base `Notify`.
| Variable | Default | Meaning |
|---|---|---|
| `MinSpeedThreshold` | `55` | Below this 2D speed, no noise is reported. |

> 🎛️ **Designer:** this is the **animation-side half of the stealth loop**. Place
> `AN_ReportFoostepNoiseEvent` on the footfall frames of locomotion animations: louder/longer
> noises (run, soil "Vibration.*") draw enemies more. Crouching produces the quietest tag. The
> AI side (who hears what, and how it reacts per tag) is in §11. Tune `Loudness`/`NoiseRange`
> per notify and `MinSpeedThreshold` to silence tiny adjustments.

<a id="94-animnotifies--gas--hitboxes"></a>
### 9.4 AnimNotifies — GAS & hitboxes

#### `UAN_SendGameplayEventToSelf`
**File:** `Public/.../AN_SendGameplayEventToSefl.h` (+ `.cpp`)
Sends a gameplay event (`EventTag`) to the owner's ASC at a precise animation frame — the
animation-driven counterpart of the event pattern used everywhere (§3, §4). Used to time
ability phases to the montage (e.g. the parry window opening, §4.5).
| Variable | `EventTag` (the gameplay event to dispatch). |

#### `UAN_EnableHitBox` (one-shot) & `UANS_EnableHitBox` (windowed)
**Files:** `Public/.../AN_EnableHitBox.h`, `Public/.../ANS_EnableHitBox.h` (+ `.cpp`s)
Toggle a `UShapeComponent` hitbox found on the owner by its **component tag** (`HitBoxTag`):
- **`UAN_EnableHitBox`** — a single-point notify that enables *or* disables collision
  (`bEnableCollision`), and can optionally also send a gameplay event (`bSendGameplayEvent` +
  `EventTag`).
- **`UANS_EnableHitBox`** — a notify *state* that enables the hitbox for the whole window
  (`NotifyBegin`) and disables it at the end (`NotifyEnd`).

> 🎛️ **Designer:** tag the hitbox `UShapeComponent` on the actor and reference the same
> `HitBoxTag` here. Use the **state** version (`ANS_`) for attacks where the hitbox is active
> for a window, and the **one-shot** (`AN_`) for instantaneous enable/disable or when you also
> need to fire a gameplay event at that frame. These are generic — used by both the player
> (parry sphere activation) and enemies (attack hitboxes, §11).

<a id="phase-10"></a>
## 10 — Trace & Collision

Three small but pervasive header-only definitions that **centralize the project's custom
collision channels and physical surfaces**, so the rest of the code refers to them by name
instead of raw engine enum values. They appear throughout the previous phases (climbing,
interactions, resonance, footsteps, AI).

<a id="101-ecustomcollision-publictraceandcollisioncustomcollisionh"></a>
### 10.1 `ECustomCollision` (`Public/TraceAndCollision/CustomCollision.h`)
A namespace mapping the engine's generic game trace channels to readable names:

| Constant | Engine channel | Used by |
|---|---|---|
| `Player` | `ECC_GameTraceChannel1` | Player object/trace identity. |
| `Companion` | `ECC_GameTraceChannel2` | Companion. |
| `Interactable` | `ECC_GameTraceChannel3` | Interaction search overlaps (§5.2). |
| `Resonance` | `ECC_GameTraceChannel4` | Resonance object search (§4.6). |
| `Climb` | `ECC_GameTraceChannel5` | Wall/edge climbing traces (§1.3). |
| `Worm` | `ECC_GameTraceChannel6` | Worm enemy (§11). |
| `Floor` | `ECC_GameTraceChannel7` | Floor checks. |
| `Mantis` | `ECC_GameTraceChannel8` | Mantis enemy (§11). |

> 🔧 **Developer:** always reference channels via `ECustomCollision::Name`, never the raw
> `ECC_GameTraceChannelN`. The constants are the single source of truth — if a channel's
> purpose changes, it's renamed in one place.
>
> 🎛️ **Designer:** these names **must match** the channel display names configured in
> *Project Settings → Collision* (`DefaultEngine.ini`). The C++ side maps name→slot; the
> editor side maps slot→display name + default responses. Keep the two in sync, and set up
> object types / trace responses on meshes accordingly (e.g. a climbable wall must respond to
> the `Climb` channel and carry the `Climb` actor tag, §1.3).

<a id="102-ecustomsurface-publictraceandcollisioncustomsurfaceh"></a>
### 10.2 `ECustomSurface` (`Public/TraceAndCollision/CustomSurface.h`)
A namespace mapping physical-material surface types to names:

| Constant | Engine surface | Meaning |
|---|---|---|
| `Grass` | `SurfaceType1` | Grass floor. |
| `Soil` | `SurfaceType2` | Soil floor (transmits a louder "vibration" noise to AI, §1.4/§9.3). |

Used by `DetectFloorType()` (§1.4) and the footstep/noise notifies (§9) to branch on what the
character is standing on. Surfaces beyond these fall back to `SurfaceType_Default`.

> 🎛️ **Designer:** the surface comes from the floor's **Physical Material** (`Surface Type`
> field). Assign `SurfaceType1`/`SurfaceType2` (named *Grass*/*Soil* in *Project Settings →
> Physics → Physical Surface*) to the relevant materials so footsteps, footstep VFX, and AI
> noise pick the right behavior. `Soil` is special-cased to be "louder" for stealth.

<a id="103-efloortypeenum-publictraceandcollisionfloortypeenumh"></a>
### 10.3 `EFloorTypeEnum` (`Public/TraceAndCollision/FloorTypeEnum.h`)
A small `BlueprintType` enum (`Default`, `Grass`, `Soil`) — a Blueprint-friendly mirror of the
surface categories, available where a clean enum is preferable to a raw `EPhysicalSurface`
(e.g. exposing the floor type to designers/Blueprint logic).

> 🔧 **Developer:** `EFloorTypeEnum` is the Blueprint-exposed classification; `ECustomSurface`
> is the C++ mapping to the engine surface slots. Keep them aligned if you add a surface
> (e.g. add `Stone` to both, plus the physical material and the footstep/noise data).

<a id="phase-11"></a>
## 11 — Enemies AI

The largest subsystem (~100 files). It implements two stealth enemies — the **Mantis**
(a sight + hearing + touch hunter with an awareness/alert meter) and the **Worm** (a
hearing/vibration-only "ground sound eater") — on top of a shared base of pawn, controller,
data-asset, perception, Behavior Tree, EQS and NavLink infrastructure. Everything is
**data-driven**: designers tune enemies through `…Info_DataAsset` assets and never touch the
controllers.

<a id="110-architecture-at-a-glance"></a>
### 11.0 Architecture at a glance
```
AStillHearCharacterBase (GAS)
 └─ AStillHearAICharacterBase            (shared AI pawn: type, speed, waypoint, data asset)
     ├─ AAIMantisCharacter              (sight/touch hunter, dormancy, navmesh checks)
     └─ AAIWormCharacter                (segmented body colliders, look-at, touch)

AAIController
 └─ AStillHearAIControllerBase (Abstract) (perception, BT, status tag, team, blackboard)
     ├─ AAIMantisController             (multi-cone sight, awareness/alert, group propagation)
     └─ AAIWormController               (hearing/vibration only, alert cooldown)

UDataAsset
 └─ UAIInfo_DataAssetBase               (walk/run/rotation speed, stun duration)
     ├─ UAIWormInfo_DataAsset           (vibration hearing ranges, dive distances)
     └─ UAIPerceptionsMeterInfo_DataAsset (awareness/alert meter tuning)
         └─ UAIMantisInfo_DataAsset     (sight cones, hearing, BT timers, group awareness)
```

The **awareness ladder** is modeled both as a `E_AITag` enum and matching GAS status tags
(§3.5): `Unaware → Suspicious → Alerted → Hunting` (+ `Stunned`). The controller's
`CurrentStatusTag` is mirrored into the blackboard and broadcast via `OnStatusTagChanged`.

<a id="111-shared-base"></a>
### 11.1 Shared base

#### `AStillHearAICharacterBase`
**Files:** `Public/EnemiesAI/Pawns/Base/StillHearAICharacterBase.h` (+ `.cpp`)
The common AI pawn. Holds the `E_AIType`, the `AIInfo_DataAsset`, the current
`E_AISpeedType` (Walk/Run, applied via `ChangeSpeedType`), the patrol waypoints
(`StartingWaypoint`/`CurrentWaypoint`), an `IndicatorComponent` (HUD marker), and the
attack-feedback VFX anchor. It reacts to GAS tags via `HandleStun` / `HandleAttack`,
restores state with `ResetAfterDeath`, exposes `SetCollision`, and a
`NotifyHitByProjectile` hook (companion sound wave).

| Exposed | Meaning |
|---|---|
| `AIType` | Mantis / Worm. |
| `AIInfo_DataAsset` | The tuning data asset. |
| `StartingWaypoint` | Patrol start (per-instance). |

#### `AStillHearAIControllerBase` (abstract)
**Files:** `Public/EnemiesAI/Controllers/Base/StillHearAIControllerBase.h` (+ `.cpp`)
The common controller. Owns the `UAIPerceptionComponent` and `UBehaviorTree`, the
`CurrentStatusTag`, and team attitude (`GetTeamAttitudeTowards`). Key extension points:
- `UpdateCurrentStatusTag(E_AITag)` / `CheckCurrentStatusTag` — change/query the awareness
  state (mirrored to blackboard, broadcasts `OnStatusTagChanged`).
- `PerceptionEventReceived` — pure-ish virtual each enemy overrides to react to stimuli.
- `SetupSightInfo` / `SetupHearingInfo` — **read the data asset and configure the
  perception senses**, so designers only edit the data asset. Each enemy implements only the
  senses it has.
- `SetupBlackboardKeys`, `ResetAIState`.

> 🔧 **Developer:** add a new enemy by subclassing both base classes, providing a data asset
> and a Behavior Tree, and overriding `PerceptionEventReceived` + the relevant `Setup*Info`.
> The base handles team, status tag, blackboard mirroring and BT startup.

#### Enums & blackboard keys
- **`AIEnum.h`** — `E_AIType`, `E_AISpeedType`, `E_AITag` (the awareness ladder), `E_AISense`
  (None/Sight/Hearing/Touch), `E_AIHearingType` (Walk/Run/Crouch/Repeater), `E_AISightCone`
  (NotSeen/Backward/Peripheral/Wide/Narrow), `E_MantisAttackType` (None/CloseAttack/Shift).
- **`BlackboardKeyNames.h`** — centralized blackboard key names (base: `CurrentWaypoint`,
  `TargetActor`, `TargetLocation`, `CurrentStatusTag`, `IsOffNavMesh`, `NearestSafeLocation`…;
  Worm: `WasBellPlayed`, `HasRoared`, `IsDiving`; Mantis: `EQSLocation`, `DisturbanceLocation`,
  `AttackType`, hunting/investigating wait times…).

#### Data-asset hierarchy
- **`UAIInfo_DataAssetBase`** — `WalkSpeed (200)`, `RunSpeed (450)`, `RotationSpeed (4)`,
  `StunDuration (2)`.
- **`UAIWormInfo_DataAsset`** — vibration hearing ranges per movement (`Max/Run/Walk/Crouch
  HearingRange`), `AlertCooldownTimer`, `BellCooldownTimer`, dolphin-dive distances, debug
  range circles.
- **`UAIPerceptionsMeterInfo_DataAsset`** — the awareness/alert meter tuning (see §11.2).
- **`UAIMantisInfo_DataAsset`** — extends the meter asset with the multi-cone sight setup,
  hearing, EQS, BT timers and group awareness (see §11.2/§11.3).

<a id="112-perception--awareness-mantis-meter"></a>
### 11.2 Perception & awareness (Mantis meter)

The Mantis uses a **two-stage meter** in `UPerceptionsMeterComponent`
(`Public/EnemiesAI/Utility/Components/PerceptionsMeterComponent.h`), tuned by
`UAIPerceptionsMeterInfo_DataAsset`:

- **Awareness** (stage 1): filled by sight **and** hearing. Fills toward
  `MaxAwarenessValue`, then the AI becomes **Suspicious** and unlocks Alert. Passive decay
  after `AwarenessPauseTime`.
- **Alert** (stage 2): filled by **sight and touch only** (not hearing). When filled via
  sight → **Alerted**; via touch → **Hunting** directly.

Both meters gain different amounts depending on the **sight cone** the target is in
(`Narrow > Wide > Peripheral > Backward`) and the **hearing type** (`Run > Walk > Crouch=0`,
`Repeater = 100`). `ProcessSensoryInput` / `ProcessContinuousSight` route stimuli into the
meters; `ForceAwarenessToMax` / `ForceAlertToMax` are used by group propagation. Broadcasts
`OnPerceptionsUpdated(Awareness, Alert)` for the HUD.

**Multi-cone sight (`UAIMantisInfo_DataAsset`):** beyond the UE perception sight cone, the
Mantis classifies the target into `Narrow / Wide / Peripheral / Backward` cones, each with its
own radius and half-angle, plus `SightHeight`, `MaxSightHeightDifference`, `LoseSightRadius`,
`LoseSightTimer`. The controller's `GetTypeOfSightCone` resolves the cone.

**Group awareness:** `bEnableSuspiciousPropagation` / `bEnableAlertedPropagation` +
`GroupAwarenessRadius` make a Mantis propagate its state (and target) to nearby Mantises
(`PropagateGroupAwareness` → `ReceiveGroupAlert`), guarded against infinite loops.

**Visualization:** `UPerceptionVisualizerComponent` and the Worm's debug range circles draw
the perception state for tuning.

> 🎛️ **Designer:** the entire stealth difficulty lives in these data assets — how fast each
> cone/hearing type fills awareness vs. alert, decay rates and pause times, sight radii and
> angles, lose-sight timing, and group propagation. Crouch awareness defaults to `0` (silent);
> a `Repeater` noise instantly maxes awareness. Tune here, never in the controller.

<a id="113-the-mantis"></a>
### 11.3 The Mantis

#### `AAIMantisCharacter`
**Files:** `Public/EnemiesAI/Pawns/Mantis/AIMantisCharacter.h` (+ `.cpp`)
A walking hunter. Features:
- **Dormancy:** `bStartDormant` + `WakeUpMontage`; `Activate()` wakes it (montage →
  `FinishActivation` restarts BT/perception). Triggered by `AMantisActivationTrigger`.
- **NavMesh safety:** periodic `CheckNavMeshConsistency` (interval, radius, failure threshold)
  snaps it back onto the navmesh if it falls off (e.g. after a shift), with
  `IsPerformingNavLinkJump` state.
- **Combat:** an `AttackHitBox` (box), `PerceptionVisualizerComp`, `NotifyHit` /
  `NotifyHitByProjectile` reactions, custom `GetActorEyesViewPoint` for sight height.

#### `AAIMantisController`
**Files:** `Public/EnemiesAI/Controllers/Mantis/AIMantisController.h` (+ `.cpp`)
Owns the `UPerceptionsMeterComponent` and the **sight/hearing/touch** setup. It runs
continuous-sight updates, lose-sight timing, disturbance-location updates (with cooldown),
`ForceHuntTarget`, target/disturbance blackboard keys, and the group-awareness propagation.

#### Mantis abilities
- **`UGA_AttackBase`** (abstract, *data-driven* `UAttackBaseData`): caches the Mantis & target,
  binds the attack hitbox (`HitBoxTag`), applies `AttackHitEffectClass` on the player on
  overlap (once per swing), listens for stun, and applies a data-driven cooldown
  (`AttackCooldownDuration`). `UGA_CloseAttack` is the concrete melee (with `UCloseAttackData`).
- **`UGA_MantisShift`** (*data-driven* `UMantisShiftData`): the teleport/"phase shift", used
  both as a **navigation traversal** (NavLinks) and a **gap-closing attack**. Phases:
  fade-out (dissolve + time-dilation curves) → physical teleport (collision disabled) → quick
  translation → fade-in → optional follow-up attack with **target prediction**
  (`PredictTargetLocation` using the target's velocity). `ComputeLandingTarget` validates the
  landing on the navmesh; `bForceHuntOnLanding` forces Hunting on Chaos-Shift teleports.
  `UMantisChaosShiftComponent` drives the Chaos-cache shift variant.

**`UMantisShiftData` (selected):** shift distances (`Min/MaxShiftDistance`,
`LandingDistanceOffset`), timings (`FadeOut/Translation/FadeIn Duration`), dissolve curve +
opacity param, time-dilation curves, loop/burst cue tags, `bPerformAttackAfterShift` +
prediction (`PredictionTimeDelay`, `PredictionFactor`, `LandingZOffset`), `NavMeshQueryExtent`.

#### Mantis traversal
`ANavLink_MantisTraversal` (`Public/EnemiesAI/NavLinks/NavLink_MantisTraversal.h`) lets the
Mantis cross gaps by triggering the shift ability along a nav link.

> 🎛️ **Designer:** the Mantis attack/shift feel is entirely in `UCloseAttackData` /
> `UMantisShiftData` (hit effect, hitbox tag, cooldown, shift timings, dissolve/time-dilation
> curves, prediction). Use `AMantisActivationTrigger` + `bStartDormant` to spawn ambush
> Mantises that wake on cue.

<a id="114-the-worm-ground-sound-eater"></a>
### 11.4 The Worm

#### `AAIWormCharacter`
**Files:** `Public/EnemiesAI/Pawns/Worm/AIWormCharacter.h` (+ `.cpp`)
A segmented creature with **head/body/tail capsule colliders**, a `LookAtPos` scene component
(its aim target), and terrain VFX. It reports **touch** (`ReportTouchEvent`), damages the
player on head contact (`HandleHeadHitCharacter` + `HitEffectClass`), keeps distance from other
worms (`RegulateDistanceWithOtherAICharacters`), and continuously aligns its capsules to the
body (`AdjustCapsuleRotation`).

#### `AAIWormController`
**Files:** `Public/EnemiesAI/Controllers/Worm/AIWormController.h` (+ `.cpp`)
**Hearing/vibration-only** (no sight). `SetupHearingInfo` reads the worm data asset; on a
hearing stimulus (`PerceptionEventReceived`) it updates the target location/actor (the noise
source), drives the dive state, and runs an alert cooldown (`HandleAlertTimer`, with a separate
**bell** path — `WasBell`). Updates the `UWormAnimInstance`.

#### Worm abilities
- **`UGA_WormRoar`**: plays the roar montage and applies a `RoarEffect` (at an AnimNotify
  event) — the worm's alert/intimidation.
- **`UGA_WormDolphinDive`**: the dive attack — switches movement mode and dives toward the
  target, ending on movement-mode change; cancels on stun. Triggered along
  `ANavLink_WormDive`, which activates the dive ability when the worm crosses the link.

> 🎛️ **Designer:** the Worm "hears" vibrations through the floor. Its detection is the
> `UAIWormInfo_DataAsset` hearing ranges per movement state (and the `Vibration.*` noise tags
> from soil, §9.3) — tune `Walk/Run/CrouchHearingRange`, the dive distances, and the alert/bell
> cooldowns. The Worm is sightless: stealth against it is about *how loud you move*, not line
> of sight.

<a id="115-shared-ability--uga_stun"></a>
### 11.5 Shared ability — `UGA_Stun`
**Files:** `Public/.../Abilities/EnemiesAI/GA_Stun.h` (+ `.cpp`), *data-driven* `UStunData`.
Applied to an enemy (e.g. by a successful parry, §4.5) to stun it for the data asset's
duration, driving the dissolve/radius Niagara + material params and ignoring configured
collision channels while stunned. Adds the `Status.EnemyAI.Stunned` tag the pawns react to.

<a id="116-behavior-tree-library"></a>
### 11.6 Behavior Tree library
All BT logic is shared between enemies through reusable nodes (the actual trees are Blueprint
assets per enemy). Highlights:

**Tasks (`BehaviorTreeTasks/`):**
| Node | Purpose |
|---|---|
| `BTTask_MoveToLocation` | Move to a blackboard location. |
| `BTTask_SetNextWaypoint` | Advance the patrol to the next `AWaypoint`. |
| `BTTask_RotateTowards` | Rotate to face a target/location. |
| `BTTask_PlayMontage` | Play a montage and wait. |
| `BTTask_ActivateAbilityAndWait` | Activate a GAS ability and wait for it to end. |
| `BTTask_ClearBlackBoardKey` | Clear a key. |
| `BTTask_ReturnToNavMesh` | Move back onto the navmesh. |
| `BTTask_PlayAttackFeedbackVFX` | Spawn the attack-feedback VFX. |
| `BTTask_StartInvestigating` / `BTTask_StartHunting` (Mantis) | Enter the investigate/hunt phases (with wait times). |
| `BTTask_MoveLookAtPosToTarget` / `BTTask_UpdateTargetLocationFromActor` (Worm) | Drive the worm's look-at and target location. |

**Services (`BehaviorTreeServices/`):**
| Node | Purpose |
|---|---|
| `ChangeSpeedType` | Switch the pawn between Walk/Run speed. |
| `BTService_CheckNavMeshAndCachePosition` | Track on/off navmesh + cache a safe position. |
| `BTService_MantisSelectAttack` | Choose `CloseAttack` vs `Shift` (sets `AttackType`). |
| `CheckDistanceFromTarget` / `CheckDistanceFromActor` (Worm) | Update "close enough" flags. |

**Decorators (`BehaviorTreeDecorators/`):**
| Node | Purpose |
|---|---|
| `BTDecorator_CustomCheckGameplayTag` | Gate a branch on a gameplay tag. |
| `BTD_CheckAIStatus` | Gate on the current `E_AITag` status. |
| `BTD_CheckMantisAttackType` | Gate on the selected attack type. |
| `BTD_ShouldWormReturnToNavMesh` | Gate the worm's return-to-navmesh branch. |

<a id="117-eqs-contexts-eqscontext"></a>
### 11.7 EQS contexts (`EQS/Context/`)
Custom `UEnvQueryContext`s used by EQS queries: `EnvQueryContext_Player`,
`EnvQueryContext_Target`, `EnvQueryContext_CurrentWaypoint`,
`EnvQueryContext_DisturbanceLocation`. They expose the right reference point (player, current
target, patrol waypoint, last disturbance) so EQS can score points around it — e.g. the Mantis
picks a random patrol/investigation point within `EQSRadius` keeping `EQSDistanceFromEach`.

<a id="118-patrol--triggers"></a>
### 11.8 Patrol & triggers
- **`AWaypoint`** (`Utility/Patrol/Waypoint.h`, extends `ATargetPoint`) — a patrol node;
  pawns chain them via `StartingWaypoint`/`CurrentWaypoint` and `BTTask_SetNextWaypoint`
  (with per-waypoint wait time).
- **`AMantisActivationTrigger`** (`Triggers/`, implements `ISavable`) — a volume that wakes a
  dormant Mantis (§11.3); persists its fired state across saves.

<a id="119-enemy-animation"></a>
### 11.9 Enemy animation
- **`USoundEaterAnimInstanceBase`** (`Animations/SoundEaterAnimInstanceBase.h`) — shared base
  AnimInstance for the sound-eater enemies.
- **`UGroundSoundEaterAnimInstance`** — the Worm's ground-creature AnimInstance (extends the
  base).
- **`UWormAnimInstance`** / **`UMantisAnimInstance`** (`Animation/AnimInstances/EnemyAI/`) —
  per-enemy AnimInstances driven by their controllers. They consume the generic hitbox /
  noise / gameplay-event AnimNotifies documented in §9.4.

> 🔧 **Developer:** the Mantis and Worm share the GAS ability skeleton (§4.0), the AnimNotify
> hitbox/event nodes (§9.4), and the noise/hearing loop (§9.3) — only their perception model
> differs (Mantis = multi-cone sight + hearing + touch meter; Worm = vibration hearing only).
> When extending AI behavior, prefer adding reusable BT nodes and data-asset fields over
> per-enemy controller code.

<a id="phase-12"></a>
## 12 — Save System

A `UGameInstanceSubsystem`-based persistence layer with two independent save channels:
1. **Slot saves** (`USaveGameObject`, slots 1–3) — the game world state: every flagged
   actor's `SaveGame` properties, Flow state, unlocked abilities, audio-state stack.
2. **Settings** (`USettingsSaveGame`, a fixed slot) — user options: volumes, key rebinds,
   controls, language, graphics/upscaler, and **global collectibles**.

World state is serialized per **level → actor → component**, keyed by a stable **GUID** that
each savable actor carries via a `USaveIdComponent`. Persistence couples with the **checkpoint**
model (`IRestorable`, §5.1) for in-session rollback and with **Flow** (§13) for when a save is
triggered.

<a id="121-identity--serialization-primitives"></a>
### 12.1 Identity & serialization primitives

#### `ISavable` (`Public/SaveSystem/Savable.h`)
A two-method interface for custom save hooks: `OnPreSave()` (before serialization) and
`OnPostLoad()` (after — refresh UI/anim/material). Both `BlueprintNativeEvent`. Implemented by
interactables, triggers and others (§5.3, §11.8).

#### `USaveIdComponent` (`Public/SaveSystem/SaveIdComponent.h`)
Gives an actor a **stable `FGuid`** so its data survives across sessions. Only actors carrying
this component are considered for saving. `GenerateID` / `RegenerateID` are editor buttons.

> 🎛️ **Designer — important gotcha:** add a `SaveIdComponent` to any actor that must persist,
> then click **Generate ID** in the component details. The id is *not* auto-created on first
> placement (a known limitation noted in the code), and `RegenerateID` **breaks existing
> saves** for that actor — use it only intentionally.

#### `FSaveCoreArchive` (`Public/SaveSystem/FSaveCoreArchive.h`)
The custom archive used for (de)serialization. It sets `ArIsSaveGame = true` (only
`UPROPERTY(SaveGame)` fields are written) and `ArNoDelta = true` (write all such properties,
even defaults), wrapping `FObjectAndNameAsStringProxyArchive` so object/name refs serialize as
strings.

> 🔧 **Developer:** mark any field you want persisted with `UPROPERTY(SaveGame)`. Nothing else
> is saved. This is why the checkpoint snapshots in §5.3/§11 use `SaveGame` flags.

#### `SaveTypes.h`
The on-disk containers: `FComponentSaveData { Bytes }` → `FActorSaveData { Transform, Bytes,
ComponentsBytes }` → `FLevelSaveData { Map<FGuid, FActorSaveData> }`. An actor's transform plus
its (and its components') serialized bytes, indexed by GUID, grouped per level.

<a id="122-save-containers"></a>
### 12.2 Save containers

#### `USaveGameObject` (slot save)
**File:** `Public/SaveSystem/SaveGameObject.h`
Holds one slot's full state:
| Field | Meaning |
|---|---|
| `SaveVersion` | Versioning for migration. |
| `Levels` | `Map<FName, FLevelSaveData>` — per-level actor data. |
| `SlotName`, `SaveDate`, `LastLevelName`, `LastPlayedDate` | Slot metadata. |
| `CheckpointId` | Row in the checkpoint display table (set when a Flow checkpoint saves). |
| `FlowComponents`, `FlowInstances` | Flow graph state (§13). |
| `UnlockedAbilities` | `TSet<EMainCharacterAbilityType>` — permanent unlocks (§1.4). |
| `AudioStateStack` | Active audio states for music/ambience persistence (§8.1). |

#### `USettingsSaveGame` (settings)
**File:** `Public/SaveSystem/SettingsSaveGame.h`
Global, slot-independent options: audio volumes (Master/Music/Sfx/Voice/Ambience), input
`Bindings` (§2.1) + gamepad context/preset + vibration toggle/intensity, `Language`,
`DisplayGamma`, upscaler selection (`ActiveUpscaler` + FSR/DLSS/Reflex indices), and the global
`CollectedCollectibles` set.

> 🎛️ **Designer:** collectibles are **global** (stored in settings, not per-slot) — once
> collected, they stay collected across saves/new games. Permanent **abilities**, by contrast,
> are per-slot (in `USaveGameObject`).

<a id="123-usavesubsystem-orchestrator"></a>
### 12.3 `USaveSubsystem` (orchestrator)
**Files:** `Public/SaveSystem/SaveSubsystem.h`, `Private/SaveSystem/SaveSubsystem.cpp`

The central API for saving/loading. It hooks map load and level streaming
(`OnPostLoadMap`, `OnLevelAddedToWorld`, `OnLevelRemovedFromWorld`) so streamed-in levels get
their saved state applied automatically.

**Slot save/load:**
| Method | Purpose |
|---|---|
| `RequestSaveSlotAsync(Slot)` | Async save of the current world into a slot (broadcasts `OnSaveStarted` / `OnSaveFinished`). |
| `LoadFromSlot(Slot, …, bRestoreAudio)` | Load a slot and apply it (optionally restoring the audio state stack). |
| `ApplyLoadedDataToCurrentlyLoadedLevels` | Re-apply the loaded data to live levels (deferred on the first run). |
| `StartNewGame(Slot)` / `SetCurrentSlotToNewGame` / `ResetGameSession` | New-game flow. |
| `GetCurrentSlotSave` / `GetLatestSaveSlotIndex` / `HasAnySaveGames` / `LoadLatestSaveGame` / `DeleteSlot` | Slot queries/management (max `MaxSaveSlots = 3`). |

**Serialization flow (developer):** `BuildSaveObject` walks each level (`SaveLevelInto`) and
each savable actor (`SaveActorInto`), serializing `SaveGame` properties through
`FSaveCoreArchive` into `FActorSaveData` keyed by the actor's GUID (`TryGetActorGuid` via
`SaveIdComponent`). Loading reverses this per level (`ApplyLevel` → `ApplyLevelInternal`),
calling `ISavable::OnPreSave` / `OnPostLoad` around the process.

**Checkpoints:** `SetCurrentCheckpoint(Id)` records the checkpoint; a **Flow** checkpoint node
triggers `HandleFlowCheckpointSaveRequested` to write a save. Display data (name + image) for a
checkpoint is resolved from a checkpoint data table
(`GetCheckpointDisplayData` / `…CurrentSaveCheckpointDisplayData` / `…SlotCheckpointDisplayData`).

**Collectibles & abilities:** `CollectCollectible` / `IsCollectibleCollected` /
`GetCollectedCollectibles` / `ResetCollectibles` (global, in settings);
`UnlockPermanentAbility` / `RemovePermanentAbility` / `IsAbilityPermanentlyUnlocked` /
`GetPermanentlyUnlockedAbilities` (per-slot — consumed by the character on possess, §1.4).

**Settings:** `SaveSettingsAsync` / `LoadSettings` / `GetSaveSettings`, plus
`ApplyUpscalerFromSettings` and `ApplyLanguageFromSettings` (re-apply engine CVars/localization
after map load). Broadcasts `OnSettingsSaveStarted` / `OnSettingsSaveFinished`.

> 🔧 **Developer:** to make something persist, (1) put `UPROPERTY(SaveGame)` on the fields,
> (2) ensure the actor has a `SaveIdComponent` with a generated id, and optionally (3) implement
> `ISavable` for pre/post hooks. The subsystem handles streaming, GUID matching and async I/O.
> Saves are triggered from **Flow checkpoints**, not arbitrary code, so save points are authored
> in the level's Flow graph.

> 🎛️ **Designer:** save points are **checkpoints placed in the Flow graph** (§13). Each
> checkpoint carries a `CheckpointId` that drives the load-screen display (name + image via the
> checkpoint data table). There are 3 save slots; collectibles are global, world/ability state
> is per-slot.

<a id="phase-13"></a>
## 13 — Flow (Level Direction & Checkpoints)

StillHear uses the **Flow** plugin for level scripting/narrative — a node-graph that drives
the game beat by beat. The C++ layer adds a central **`ASceneManager`** (level streaming,
spawn, death/respawn, menu, checkpoint persistence), a **checkpoint** system that doubles as
the save trigger (§12), and a library of **custom Flow nodes** for gameplay/UI/audio scripting.

<a id="131-ascenemanager"></a>
### 13.1 `ASceneManager`
**Files:** `Public/Flow/SceneManager.h`, `Private/Flow/SceneManager.cpp`

The hub that owns the level lifecycle. Implements `ISavable` and carries a `SaveIdComponent`
so its own state persists. Responsibilities:

- **Streaming orchestration:** loads levels into memory (`LoadLevelsStreaming` → hidden),
  then shows/hides them (`ShowStreamingLevels` / `HideStreamingLevels`) with full async
  bookkeeping (per-level loaded/shown/hidden callbacks and counters). The `EShowLevelsContext`
  enum (`InitialLoad` / `RuntimeTransition` / `Respawn` / `LoadGame`) tells `OnAllLevelsShown`
  which follow-up to run.
- **Player spawn/reset:** `SpawnCharacter` (at `PlayerStart` or the saved
  `PlayerStartLocation/Rotation`), `ResetCharacter`, `ResetActors`, plus death handling
  (`OnCharacterDeath` → `ShowDeathScreen` → `RestoreStreamingLevels`) and
  `OnCharacterInitialized`.
- **Menu vs gameplay:** `bStartInMenuMode`, `SetupMenuView`, `TransitionToGameplay`,
  `ReturnToMainMenu`, `RefreshMenuBackground` — the menu/gameplay hand-off referenced by the
  player controller (§1.3).
- **Flow root:** starts `RootFlowAsset` (`StartFlowRoot`) once the scene is shown, and
  restores its instance (`SavedRootFlowInstanceName`) on load.
- **Checkpoint persistence:** the bridge to the save system — see §13.2.

**Saved fields (`UPROPERTY(SaveGame)`):** the streaming level layout
(`StreamingLevelsLoadedFromSave`), player start transform, `CurrentCheckpointPriority`, and the
flow root instance name.

> 🔧 **Developer:** `ASceneManager` is the single authority over what's loaded/visible.
> Runtime visibility (`RuntimeVisibleLevels`) is tracked separately from the **saved** layout,
> so non-saving checkpoints can change what's on screen without persisting it.
>
> 🎛️ **Designer:** configure the starting/streaming levels, the player class & `PlayerStart`,
> the menu camera tag, the `RootFlowAsset`, and the `CheckpointDataTable` here. `bBypassSaveGame`
> is a debug switch to ignore saves.

<a id="132-checkpoints"></a>
### 13.2 Checkpoints

#### `ACheckpointBase` (abstract)
**Files:** `Public/Flow/CheckpointBase.h`, `Private/Flow/CheckpointBase.cpp`
A checkpoint that changes the streamed-level layout as the player crosses it, and optionally
saves. Core idea: each checkpoint has a **priority** (higher = further in the game), and the
game **only saves when the player reaches a higher priority than the current one** (forward
progress) — preventing backward checkpoints from overwriting progress.

- `OnCheckpointActivated()` (forward) shows `LevelsToShow` / hides `LevelsToHide`, moves the
  player start to `PlayerNewStartPoint`, and — if `bCallSaveOnActivate` and the priority is
  higher (`CanActivateCheckpoint`) — calls `ASceneManager::UpdateSavedState` (persist) instead
  of just `UpdateRuntimeVisibility`.
- `OnCheckpointReversed()` (backward) reverses the visibility without saving.

| Exposed | Meaning |
|---|---|
| `LevelsToShow` / `LevelsToHide` | Streaming levels toggled on traversal. |
| `bCallSaveOnActivate` | Whether crossing forward writes a save. |
| `CheckpointPriority` | Progress ordering; gates saving. |
| `CheckpointId` | Row in `DT_CheckpointDisplay` → load-screen label + image (§12.3). |

#### `ACheckpointTrigger`
**Files:** `Public/Flow/CheckpointTrigger.h`, `Private/Flow/CheckpointTrigger.cpp`
The concrete trigger-volume checkpoint: a `BoxComponent` that records the entry location and,
on exit, detects **traversal direction** (forward vs backward) to call activate/reverse.
`bTriggerOnce` disables it after the first traversal.

> 🎛️ **Designer:** place `ACheckpointTrigger` volumes along the critical path. Set
> `CheckpointPriority` increasing along progress, list the `LevelsToShow/Hide` for that beat,
> enable `bCallSaveOnActivate` on the ones that should save, and give each a `CheckpointId`
> matching a row in the checkpoint display table (for the load screen). Backward crossings
> restore visibility but never save.

<a id="133-custom-flow-nodes"></a>
### 13.3 Custom Flow nodes
All `NotBlueprintable` C++ Flow nodes (authored in the Flow graph). They let designers script
gameplay/UI/audio without code:

**Gameplay tags & events:**
| Node | Purpose |
|---|---|
| `FlowNode_HasTag` ("Has Tag") | Branch True/False on whether an actor has a gameplay tag. |
| `FlowNode_OnGameplayEvent` ("On Gameplay Event") | Wait for a gameplay event on a target (tutorials: crystal broken, lever pulled…). |
| `FlowNode_OnTagRemoved` ("On Tag Removed") | Wait for a tag to be removed from a target. |
| `FlowNode_TagCounter` ("Tag Counter") | Count a tag's occurrences on a target; fire when a threshold is reached. |
| `FlowNode_IdleCheck` ("Idle Check") | Fire if the player stays idle for a time; cancelled only by a specific gameplay event. |

**Abilities:**
| Node | Purpose |
|---|---|
| `FlowNode_AddAbility` ("Add Ability") | Grant an ability to a target (usually the player). |
| `FlowNode_RemoveAbility` ("Remove Ability") | Remove an ability from a target. |

**UI:**
| Node | Purpose |
|---|---|
| `FlowNode_ShowWidget` ("Show Widget") | Push a widget to the UI subsystem and wait for a finish condition (tutorials/hints/popups). |
| `FlowNode_ClearUILayer` ("Clear UI Layer") | Clear all widgets from a UI layer. |

**Cinematics:**
| Node | Purpose |
|---|---|
| `FlowNode_PlaySequenceWithPlayer` ("Play Sequence (With Player Bound)") | Play a Level Sequence with the player character auto-bound at runtime (extends the Flow plugin's `FlowNode_PlayLevelSequence`, whose skip/complete events the player controller listens to, §1.3). |

**Audio** (documented in §8.8, defined under `Audio/`):
`FlowNode_PlayMusic`, `FlowNode_PlayAmbience`, `FlowNode_StopAudio`, `FlowNode_SetAudioState`
— drive the `UGameAudioSubsystem` from the graph.

**Template:** `CustomTestNode` ("Custom Node") is a sample/boilerplate node to copy when
authoring new ones.

> 🎛️ **Designer:** these nodes are the scripting vocabulary for level beats — gate progress on
> tags/events, grant/revoke abilities at story moments, show tutorial widgets, play cinematics
> with the player bound, and switch music/ambience. Combine them in the `RootFlowAsset` graph;
> the `SceneManager` runs it once the scene is shown.

> 🔧 **Developer:** Flow graph state (`FlowComponents` / `FlowInstances`) is serialized into the
> slot save (§12.2), so a loaded game resumes the narrative graph where it left off. New nodes
> should subclass the appropriate Flow base and stay `NotBlueprintable`, matching the existing
> set.

<a id="phase-14"></a>
## 14 — PCG (Procedural Generation along Splines)

A small toolkit built on Unreal's **PCG** framework to scatter/connect meshes along a spline.
It has two halves: **spline actors** that host a `UPCGComponent` and expose the source spline
+ mesh list to the graph, and **custom PCG nodes** that read that spline and emit/transform PCG
points which the graph turns into meshes.

<a id="141-spline-actors"></a>
### 14.1 Spline actors

#### `APCGSplineBase` (abstract)
**Files:** `Public/PCG/Actors/PCGSplineBase.h`, `Private/.../PCGSplineBase.cpp`
The base actor: a `USplineComponent` (the path) + a `UPCGComponent` (runs the graph), plus the
shared inputs the custom nodes read — a `Meshes` array and a `Distance` (spacing). Concrete
subclasses pick a generation style:

| Variable | Default | Meaning |
|---|---|---|
| `Meshes` | — | Candidate static meshes to spawn along the spline. |
| `Distance` | `100` | Spacing between generated points. |

#### `APCGSplineSequentialMeshes`
**File:** `Public/PCG/Actors/PCGSplineSequentialMeshes.h`
Scatters discrete meshes sequentially along the spline. Adds orientation/placement options:
`bApplyOrientation`, `bShrinkToFitNextPoint`, `LocationOffset`, `RotationOffset`, and
`RandomRotation` (used when orientation is off).

#### `ASplineSmoothConnectedMeshes`
**File:** `Public/PCG/Actors/SplineSmoothConnectedMeshes.h`
Builds a **continuous** run of `USplineMeshComponent` segments that deform to follow the spline
(fences, vines, pipes…). Options: `ForwardAxis`, `UpDirection` (anti-twist), `bEnableCollision`,
`MeshScale` (cross-section), and `OverlapAmount` (extend each segment to avoid gaps).

> 🎛️ **Designer:** drop the actor, draw the spline, fill `Meshes`, set `Distance`. Use
> **`SequentialMeshes`** for discrete scattered props (with offsets / random rotation), and
> **`SmoothConnectedMeshes`** for a connected deforming run (tune `ForwardAxis`/`UpDirection`
> to stop twisting, `OverlapAmount` to close seams). The actor's `UPCGComponent` runs the graph
> and regenerates on edit.

<a id="142-custom-pcg-nodes"></a>
### 14.2 Custom PCG nodes

#### `UPCGNode_CustomBaseSettings` (abstract) + `FPCGNode_CustomBaseElement`
**Files:** `Public/PCG/Nodes/PCGNode_CustomBase.h`, `Private/.../PCGNode_CustomBase.cpp`
The shared base for the project's PCG nodes: a common editor node color/type and a helper
(`GetPointDataFromInput`) to pull `UPCGPointData` off an input pin. Each concrete node defines
its title, input/output pins, and an `IPCGElement` whose `ExecuteInternal` does the work.

#### Concrete nodes
| Node | Role |
|---|---|
| `UPCGNode_GenerateSplinePointsSettings` ("Generate Spline Points") | Reads the source `APCGSplineBase`'s spline + `Distance` + `Meshes` and emits PCG **points** spaced along the spline, tagging each with the mesh to spawn (`MeshAttributeName = "MeshToSpawn"`). The entry point of the graph. |
| `UPCGNode_OrientToNextSettings` ("Orient To Next") | Reorients each point to face the next one along the spline (smooth directional alignment). |
| `UPCGNode_ApplyOffsetDataSettings` ("Apply Offset Data") | Applies location/rotation offset data to the points. |
| `UPCGNode_SpawnSmoothMeshesSettings` ("Spawn Smooth Meshes") | Consumes the points to spawn the connected spline-mesh run (paired with `ASplineSmoothConnectedMeshes`), reading `MeshToSpawn`. |

**Pattern (developer):** each node is the standard PCG `UPCGSettings` + `IPCGElement` pair —
`InputPinProperties` / `OutputPinProperties` declare the pins (e.g. `In: Any → Out: Point`),
`CreateElement` returns the element, and `ExecuteInternal` reads the settings + source spline
actor (`Context->SourceComponent->GetOwner()` cast to `APCGSplineBase`) to produce/transform
points.

> 🔧 **Developer:** the nodes intentionally read the **source spline actor** off the PCG
> context rather than taking the spline through a pin, so the same graph works on any
> `APCGSplineBase`. To add a generation style, subclass `UPCGNode_CustomBaseSettings` + an
> `IPCGElement`, following the existing four. The mesh-selection attribute is `"MeshToSpawn"`
> by convention (shared between Generate and Spawn nodes).

> 🎛️ **Designer:** a typical graph is **Generate Spline Points → (Orient To Next / Apply
> Offset Data) → spawn** (Static Mesh Spawner for discrete, or **Spawn Smooth Meshes** for the
> connected run). The points carry the chosen mesh, so you control variety via the actor's
> `Meshes` list and spacing via `Distance`.

<a id="phase-15"></a>
## 15 — VFX & Weather

A grab-bag of visual systems: gameplay VFX components (resonance minigame, spline-traveling
effects, lock-on marker, fake shadow) and an atmospheric **weather** system (rain, wind,
volumetric-cloud lightning, and single scripted lightning strikes).

<a id="151-vfx"></a>
### 15.1 VFX

#### `UResonanceManagerComponent`
**Files:** `Public/VFX/ResonanceManagerComponent.h`, `Private/.../ResonanceManagerComponent.cpp`
The **logic + visuals** behind the player's Resonance ability (§4.6). Lives on the player
(`ResonanceManagerComponent`, §1.4) and runs the resonance "match" minigame through phases
(`EResonancePhase`): `Inactive → Phase1` (ping-pong movement between `±MaxHeight`) `→ Phase2`
(fast one-shot) `→ Success` / `Reset`. The ability calls `StartResonance` / `AttemptMatch` /
`StopResonance`; the component drives Phase1/Phase2 Niagara VFX + sound, tracks the threshold
window (`bInThreshold`), and broadcasts `OnResonanceSuccess` / `OnResonanceInterrupted` back to
the ability. Tuned by the same `UResonanceData` (§4.6) — `MatchThreshold`, `SpeedMultiplier`,
`ResetSpeed`, `MaxHeight`, the phase VFX/sounds.

> 🔧 **Developer:** this is the "where the Phase2 tuning actually lives" answer from §4.6 — the
> ability handles input/camera/feedback, this component runs the match state machine and its
> VFX. `FResonanceParamNames` holds the Niagara parameter names (`HeightTop`, `HeightBottom`,
> `TriggerShockwave`).

#### `USplineVFXTravelerComponent`
**Files:** `Public/VFX/SplineVFXTravelerComponent.h`, `Private/.../SplineVFXTravelerComponent.cpp`
Spawns Niagara VFX (+ sounds) that **travel along the owner's spline**, optionally triggering
linked interactables on arrival. Multiple travelers can run at once (`FSplineTravelerInstance`),
but only one looping sound. `TriggerTravel(bReverse)` launches one; on reaching the end it fires
`ForwardLinkedObjects` (or `ReverseLinkedObjects` for reverse). This is what powers the
`LinkedVFXTravelers` field on interactables (§5.3) — e.g. an energy pulse running down a cable
to activate the next object.

| Variable (selected) | Meaning |
|---|---|
| `TravelVFX` / `StartVFX` / `EndVFX` | Niagara for travel / spawn / arrival. |
| `TravelLoopSound` / `StartSound` / `EndSound` | Audio. |
| `TravelCurve` / `FallbackTravelSpeed` | Movement along the spline (curve-driven, else `500 cm/s`). |
| `ForwardLinkedObjects` / `ReverseLinkedObjects` | Interactables triggered at the end/start. |

#### `UTargetMarkerNiagaraComponent`
The companion sound-wave lock-on reticle — documented in §7.3.

#### `UBlobShadowComponent`
**Files:** `Public/Character/Components/BlobShadowComponent.h`, `Private/.../BlobShadowComponent.cpp`
A fake **blob shadow** under the character (on the player, §1.4), drawn with decals. Modes
(`EBlobShadowMode`): `FootShadowsOnly`, `TrailOnly`, `Both`. Per foot (`FBlobShadowFoot`,
left/right) it traces down to the ground and projects a decal that **shrinks with height**
(`SizeFalloff`, `ShrinkDistance`, `ShrinkExponent`) and fades when airborne
(`ShadowFadeSpeed`), and optionally drops **trail** decals as the foot moves
(`TrailSpawnDistance`, `TrailLifespan`, `TrailDecalSize`).

| Group (selected) | Meaning |
|---|---|
| `LeftFoot` / `RightFoot` | Per-foot socket + trail config. |
| `ShadowMode`, `FootSpacing`, `DecalRotationOffset` | Mode and placement. |
| `ShadowBaseSize`, `ShadowProjectionDepth`, `SizeFalloff`, `ShrinkDistance`, `ShrinkExponent`, `ShadowFadeSpeed` | Shadow size/projection/falloff/fade. |
| `MaxTraceDistance`, `GroundOffset`, `FloorTraceChannel` | Ground trace. |

> 🎛️ **Designer:** assign foot sockets and pick a mode; `Both` gives a soft contact shadow
> plus footprints. Tune `ShadowBaseSize`/`SizeFalloff` for the look on the ground and
> `ShadowFadeSpeed` for how it pops when jumping. `SetTrailEnabled` toggles footprints at
> runtime.

<a id="152-weather"></a>
### 15.2 Weather

#### `ARainyWeatherVolume`
**Files:** `Public/Weather/RainyWeatherVolume.h`, `Private/.../RainyWeatherVolume.cpp`
A box-volume storm controller. When the player enters/exits, it **lerps** rain, wind,
volumetric-cloud lightning and a post-process exposure shift in/out. It drives:
- **Rain** — a Niagara `RainVFX` (spawn-rate lerp), can follow the player.
- **Wind** — lerped wind parameters.
- **Volumetric cloud lightning** — animates a `UMaterialParameterCollection` (cloud storm
  params: lightning color/flicker/mask, albedo…), caching engine defaults to restore on exit.
- **Lightning plane** — cycles through `LightningTextures` on a timed curve.
- Configured by a `URainyWeatherDataAsset` + two Material Parameter Collections (rain,
  volumetric clouds). `bCinematicMode` and follow-player toggles support scripted use.
  `RainyWeatherParameterNames.h` centralizes the MPC parameter names.

> 🎛️ **Designer:** place the volume over a storm area, assign the rain/clouds MPCs and the
> `RainyWeatherDataAsset`, and the storm fades in/out as the player enters/leaves. Use
> `bCinematicMode` / follow-player for authored sequences. The lightning textures cycle the
> sky flashes.

#### Single lightning strikes
- **`ASingleLightningBase`** (abstract, `Public/Weather/SingleLightningBase.h`) — one scripted
  bolt: a Niagara `LightningVfx`, an `FCameraEffectPreset` (§6.4) for the screen shake/flash, a
  delayed `LightningSfx` (`SoundDelayTime` for light-then-thunder), and an optional linked
  `AElectrifiedPole`. `TriggerLightning()` fires it.
- **`ASingleLightningTrigger`** — strikes when the player overlaps a `BoxComponent`.
- **`ASingleLightningTimer`** — strikes on a timer (`bIsTimeRandom` → fixed `Time` or random
  `Min/MaxTime`), rescheduling each strike.
- **`AElectrifiedPole`** (`Public/Weather/ElectrifiedPole.h`) — a mesh + electric Niagara that
  a lightning strike powers (`StartEffect`).

> 🎛️ **Designer:** use `ASingleLightningTimer` for ambient repeating strikes (fixed or random
> interval) and `ASingleLightningTrigger` for a strike on cue (player enters a volume). Each
> bolt plays its VFX, a camera-effect preset, and delayed thunder; link an `AElectrifiedPole`
> to electrify a prop on hit.

> 🔧 **Developer:** the lightning hierarchy splits *what a strike is* (`SingleLightningBase`:
> VFX + camera preset + delayed SFX + pole) from *when it fires* (`Trigger` = overlap, `Timer`
> = scheduled), mirroring the trigger/timer split used elsewhere (interactions, audio states).
> The weather volume reuses the camera-effects and MPC systems rather than bespoke rendering.

<a id="phase-16"></a>
## 16 — UI

The largest subsystem (~105 files), built on **CommonUI / CommonInput** for gamepad-first,
layer-based UI. The architecture is: a **HUD** spawns a **PrimaryGameLayout**, which holds
tag-addressed **layers** (stacks); gameplay/menu code pushes **activatable widgets** onto those
layers through the **UISubsystem**. A separate **indicator** system draws world-anchored
markers, and a pure-**Slate** loading screen survives level transitions.

<a id="160-layer-model"></a>
### 16.0 Layer model
UI layers are `FGameplayTag`s (§3.5): `UI.Layer.Game`, `UI.Layer.Menu`, `UI.Layer.Window`,
`UI.Layer.Modal`. Each maps to a `UCommonActivatableWidgetStack` in the layout. Pushing a
widget to a layer activates it (and can clear the layer / pause the game); the top of each
stack is the active widget for that layer.

<a id="161-core-layout--routing"></a>
### 16.1 Core layout & routing
| Class | Role |
|---|---|
| `ACustomHUD` (`Subsystem/CustomHUD.h`) | Spawns the `PrimaryLayoutClass` at `BeginPlay` and registers it with the UISubsystem. |
| `UPrimaryGameLayout` (`Widgets/PrimaryGameLayout.h`) | Owns the tag→stack `Layers` map; `RegisterLayer` / `PushWidgetToLayer` / `ClearLayer`. Tracks `PausingWidgets` and unpauses when they deactivate. |
| `UUISubsystem` (`Subsystem/UISubsystem.h`, `LocalPlayerSubsystem`) | The public API: `SetPrimaryLayout`, `GetPrimaryLayout`, `PushWidgetToLayer(LayerTag, Class, bClearLayer, bPauseGame)`, `ClearLayer`. Broadcasts `OnUILayoutReady`. This is what the controller/Flow use (§1.3, §13). |
| `UActivatableWidgetBase` (`Widgets/ActivatableWidgetBase.h`) | Base for all activatable widgets: input mode (`E_WidgetInputMode`), mouse capture, a `UIStateTag`, a back-action binding (`BackButton` / `BackInputAction`, `SetBackActionBlocked`), focus target (`GetPreferredFocusTarget`), and action routing (`RouteAction`). |
| `IUIWidgetInterface` (`Widgets/UIWidgetInterface.h`) | Generic `InitializeWidget(Text, Duration, InputActions)` so Flow nodes (§13) can push data into a widget. |
| `APC_MainMenu` (`Subsystem/PC_MainMenu.h`) | Dedicated player controller for the main-menu map. |
| `UIEnum.h` | `E_WidgetInputMode` (Default/GameAndMenu/Game/Menu), `EButtonContentMode` (Text/Icon/Both). |

> 🔧 **Developer:** never add widgets to the viewport directly — go through
> `UISubsystem::PushWidgetToLayer` with the right layer tag. Derive screens from
> `UActivatableWidgetBase` so back-navigation, input mode and focus work consistently.

<a id="162-reusable-elements-uielements"></a>
### 16.2 Reusable elements (`UI/Elements/`)
Building blocks used across menus:
| Element | Role |
|---|---|
| `UButtonBase` (`UCommonButtonBase`) | Base button (text/icon content mode). |
| `USliderBase`, `UDropdownBase`, `URotatorBase` (all `USettingsRowBase`) | Settings row controls (value slider, dropdown, left/right rotator). |
| `USettingsRowBase` | Base for a labeled settings row. |
| `UDropdownMenu` | The popup list for a dropdown. |
| `UTabWidgetBase` (`UActivatableWidgetBase`) | Tabbed container. |
| `UPopupButtonBase`, `UCollectibleSlotButton`, `USaveSlotButtonBase` (`UButtonBase`) | Specialized buttons (popup action, a collectible cell, a save-slot cell). |

<a id="163-menus--settings"></a>
### 16.3 Menus & settings
Settings are organized as tabbed pages built on `USettingsPageBase` (+ `USettingsRowBase`
rows), aggregated by `UGameSettings` (a `UTabWidgetBase`):
| Widget | Role |
|---|---|
| `UGameSettings` | The settings hub (tabs). |
| `UAudioSettingsWidget` | Volume sliders → `UGameAudioSubsystem` (§8). |
| `UGraphicsSettingsWidget` | Graphics options. |
| `UCustomQualitySettingsWidget` | Per-quality overrides. |
| `UFSRSettingsWidget` / `UDLSSSettingsWidget` | Upscaler options (persisted in settings, §12.2). |
| `UControlsSettingsWidget` | Controls hub (routes to keyboard/gamepad rebinding, §16.4). |
| `USaveSlotsWidget` (+ `USaveSlotButtonBase`) | Save/load slot selection → `USaveSubsystem` (§12). |
| `UCollectiblesWidget` (+ `UCollectibleSlotButton`) | Collectibles gallery from a data table (§12.2). |
| `UConfirmationWidget` | Generic yes/no modal. |

<a id="164-controls-rebinding-widgetscontrols"></a>
### 16.4 Controls rebinding (`Widgets/Controls/`)
The UI half of the input system (§2), split keyboard vs gamepad:
**Keyboard (`Controls/Keyboard/`):**
| Widget | Role |
|---|---|
| `UBindingsPageWidgetBase` (`USettingsPageBase`) | The keyboard rebinding page. |
| `UBindingsListWidget` | The list of rebindable actions. |
| `UBindingRowWidget` (`UButtonBase`) | One rebindable action row. |
| `UKeyboardMoveRowWidget` (`UBindingRowWidget`) | Special row for the WASD/move directions (`EKeyboardMoveDirection`, resolved via §2.4). |
| `UPressAnyKeyWidget` + `FPressAnyKeyInputProcessor` | Modal "press a key" capture (a Slate input pre-processor grabs the next key). |
| `UHorizontalBoxButton` | Layout helper button. |
**Gamepad (`Controls/Gamepad/`):** `UGamepadBindingsWidget` (`USettingsPageBase`) — gamepad
binding/preset page.

> 🎛️ **Designer:** the rebinding UI reads/writes through `UInputSubsystem` (§2). Key glyphs
> come from `GetBrushFromKey`; "press any key" uses the input processor to capture the new key,
> then commits via the subsystem's staged-rebind flow.

<a id="165-popups--hud-widgets"></a>
### 16.5 Popups & HUD widgets
| Widget | Role |
|---|---|
| `UPopupWidget` (`UActivatableWidgetBase`, `IUIWidgetInterface`) | Base popup; receives text/duration/actions from Flow. |
| `UTutorialPopupWidget` (`UPopupWidget`) | Tutorial message with `{0}`/`{1}` placeholders replaced by input-action glyph widgets. |
| `UImagePopupWidget` (`UPopupWidget`) | Image popup (e.g. collectible found, §5.4). |
| `UInputActionWidget` | A dynamic glyph widget for one input action (animated). |
| `UHoldProgressWidget` | On-screen hold-to-interact progress (§5.2 hold interactions). |
| `UGraphicsDebugWidget` | Real-time graphics/upscaler CVar overlay. |
| `UPSOLoadingWidget` | PSO precache/shader-compilation progress (§17 FunctionLibrary). |

<a id="166-world-anchored-indicators-uiindicator"></a>
### 16.6 World-anchored indicators (`UI/Indicator/`)
A screen-space marker system for pointing at world actors (interaction prompts, enemy status):
| Class | Role |
|---|---|
| `UIndicatorSubsystem` (`UWorldSubsystem`) | Registry of active `UIndicatorDescriptor`s (`AddIndicator` / `RemoveIndicator`). |
| `UIndicatorComponent` | Attached to an actor; describes its indicator (the interactables/enemies carry one). |
| `UIndicatorDescriptor` | The data for one indicator (target, widget class, …). |
| `UIndicatorLayer` | The on-screen container that projects descriptors to screen space. |
| `UIndicatorWidgetBase` | Base indicator widget. |
| `UPromptIndicatorWidget` (`UIndicatorWidgetBase`) | Interaction prompt marker. |
| `UEnemyStatus` (`UIndicatorWidgetBase`, `UI/AI/EnemyStatus`) + `EnemyIndicatorWidget` | Enemy awareness/status marker (fed by the AI perception events, §11.2). |
| `UIndicatorTriggerComponent` (`UBaseTriggerComponent`) | A trigger volume that registers/unregisters target actors' indicators on enter/exit (`bActivateOnEnter` / `bDeactivateOnExit`, `TargetActors` or `bManagedExternally`). |

> 🎛️ **Designer:** put a `UIndicatorComponent` on an actor to give it an on-screen marker,
> and use `UIndicatorTriggerComponent` volumes to show/hide markers by area. Enemy status
> markers reflect the awareness ladder (§11).

<a id="167-loading-screen-uislate"></a>
### 16.7 Loading screen (`UI/Slate/`)
`SLoadingScreenWidget` — a **pure Slate** full-screen loading widget (black background +
animated material icon). Because it's raw Slate (`SCompoundWidget` + `FGCObject`), it survives
level transitions and never freezes with the game thread. Static `Show()` / `Hide()` from
anywhere. Configured via the `LoadingScreenSettings` (§17).

> 🔧 **Developer:** use `SLoadingScreenWidget::Show/Hide` for transitions where a UMG widget
> would stall (map loads). It renders independently of the level's UI.

<a id="168-world-text--atypewritertextactor-uitextrenderer"></a>
### 16.8 World text — `ATypewriterTextActor` (`UI/TextRenderer/`)
A world-space `UTextRenderComponent` actor that reveals text character-by-character with a
per-letter Niagara FX + sound. Text comes from a `FTypewriterTextRow` **data table** row
(localized) or a fallback `FullText`; supports word-wrap (`MaxCharsPerLine`), reveal speed
(`CharactersPerSecond`), and an animatable `RevealProgress` (sequencer-friendly). API:
`StartReveal` / `StopReveal` / `CompleteReveal` / `ResetReveal` / `SetRevealProgress`.

| Field (selected) | Meaning |
|---|---|
| `TextDataTable` / `TextRowName` / `FullText` | Localized source vs fallback. |
| `CharactersPerSecond`, `MaxCharsPerLine`, `RevealProgress` | Reveal timing/layout. |
| `LetterFX`, `LetterSound`, `LetterSoundVolume`, `CharacterWorldWidth` | Per-letter FX. |

> 🎛️ **Designer:** use for diegetic world text that types itself in. Point it at a data-table
> row for localization; drive `RevealProgress` from a Level Sequence to sync the reveal to a
> cinematic, or call `StartReveal` at runtime.

<a id="phase-17"></a>
## 17 — Infrastructure & Shared Data

The remaining glue: the GameInstance, world/utility subsystems, project settings, editor tools,
and the catalog of shared Data Assets / Data Tables.

<a id="171-ustillheargameinstance"></a>
### 17.1 `UStillHearGameInstance`
**Files:** `Public/StillHearGameInstance.h`, `Private/StillHearGameInstance.cpp`

The game-wide root object, persistent across level loads. It holds the **central references**
the subsystems fetch (the input `MappingContextList`, §2.1, and the keyboard/Xbox/PlayStation
`UCommonInputBaseControllerData` glyph classes, §2.4) and coordinates the **new-game / load-game
hand-off** across a persistent-map reload:
| Member | Role |
|---|---|
| `PendingNewGameSlotIndex` / `PendingLoadGameSlotIndex` | Slot to pick up after the persistent map reloads (`-1` = none). |
| `bIsSpawningNewGame` / `bIsNewGameResetting` | Flags read by the character/scene on spawn (§1.4, §13). |
| `OnRequestWorldReset` / `OnCheckpointSnapshot` / `OnClearCheckpointState` | Delegates broadcasting world-reset and checkpoint snapshot/clear (drive `IRestorable`, §5.3/§12). |

> 🎛️ **Designer:** assign the input mapping-context list and the three controller glyph data
> classes on the GameInstance — the input/UI systems read them from here.

<a id="172-utimemanagementsubsystem"></a>
### 17.2 `UTimeManagementSubsystem`
**Files:** `Public/Subsystems/TimeManagementSubsystem.h`, `Private/.../TimeManagementSubsystem.cpp`

A `UWorldSubsystem` for **global time dilation**. `PlayTimeCurve(Curve)` drives slow-motion over
real time from a `UCurveFloat` (used by the parry slow-mo, §4.5, and the Mantis shift, §11.3);
`TriggerHitStop(Duration)` freezes the game briefly to add impact weight; `ResetTimeDilation()`
restores normal. It ticks the curve on real-world time (`FTSTicker`) so it works even while time
is dilated.

> 🔧 **Developer:** route all slow-mo/hit-stop through this subsystem so effects don't fight
> over `GlobalTimeDilation`. Curves are authored per effect (e.g. `UParryData::SloMoEffectCurve`).

<a id="173-upsoblueprintlibrary"></a>
### 17.3 `UPSOBlueprintLibrary`
**File:** `Public/FunctionLibrary/PSOBlueprintLibrary.h`
Two Blueprint-pure helpers exposing shader/PSO precompilation status:
`GetPSORemaining()` and `IsPSOCompilationComplete()`. Consumed by the `UPSOLoadingWidget` (§16.5)
to show a "compiling shaders" screen and gate startup.

<a id="174-project-settings-udevelopersettings"></a>
### 17.4 Project Settings (`UDeveloperSettings`)
| Settings class | Section | Role |
|---|---|---|
| `USoundDeveloperSettings` | StillHear → Game Sound | Audio wiring & defaults (§8.3). |
| `ULoadingScreenSettings` | Game → Loading Screen | Normal loading + **death screen** visuals (colors, materials, text, icon alignment/size, fonts) and death timing (`DeathScreenInitialDelay`, `DeathScreenDuration`). Drives the Slate loading screen (§16.7) and the death flow (§13.1). |

> 🎛️ **Designer:** the death screen look and timing live in *Project Settings → Game →
> Loading Screen* (`DeathText`, colors, delays), shared with the normal loading screen icon
> style.

<a id="175-tools--atrajectorypreview-editor"></a>
### 17.5 Tools — `ATrajectoryPreview` (editor)
**File:** `Public/Tools/TrajectoryPreview.h` (+ `.cpp`, `#if WITH_EDITOR`)
An **editor-only** helper actor that draws the predicted sound-wave projectile trajectory
(bounces, impact point) in the viewport, reading the same `USoundWaveData` (§7.2) so designers
can preview the shot arc while placing things. Redraws on actor move (`OnGlobalActorMoved`).

| Variable (selected) | Meaning |
|---|---|
| `SoundWaveData` | The projectile config to simulate. |
| `MaxBounces`, `MaxDistance` | Trajectory sim limits. |
| Arrow/Line/Point colors & sizes | Debug-draw styling. |

<a id="176-shared-data-catalog"></a>
### 17.6 Shared Data catalog

**Data Assets (`Data/DataAssets/`)** — most were documented with their systems; consolidated:
| Asset | Used by |
|---|---|
| `UCrouchData`, `UParryData`, `UResonanceData` | Player abilities (§4). |
| `UInteractionBaseData` (+ `UTap`/`UHoldInteractionData`) | Interaction abilities (§5.2). |
| `USoundWaveData` | Companion sound wave (§7.2). |
| `UFootStepData` | Footsteps (§8.7). |
| `UCameraEffectsPresetData` | Camera juice presets (§6.4). |
| `URainyWeatherDataAsset` | Weather volume (§15.2). |
| *(AI)* `UAIInfo_DataAssetBase` & subclasses, `UAttackBaseData`/`UCloseAttackData`, `UMantisShiftData`, `UStunData` | Enemies (§11). |

**`URainyWeatherDataAsset` (detail):** the full storm authoring asset — per-element enter/exit
config for **rain** (intensity, spawn rate, velocity, sprite, bounds, smoothing/delay), **wind**
(intensity, smoothing/delay), **lightning** (color, timing between strikes, appearance curve,
screen flash + exposure), and **volumetric clouds** (~18 storm params blended over time). Consumed
by `ARainyWeatherVolume` (§15.2).

**Data Tables (`Data/DataTables/`):**
| Row struct | Drives |
|---|---|
| `FCheckpointDisplayData` | Save-slot label + background image per checkpoint (§12.3/§13.2). |
| `FCollectibleData` | Collectible text/image/material (§16.3); helper `CollectibleRowNameToDisplayText`. |
| `FSettingDescriptionRow` | Title/description for settings rows (§16.3). |
| `FGamepadImageData` | Per-preset controller images (EN/ITA, Xbox/PS) for the gamepad settings (§16.4). |
| `FPSOMessageData` | Messages for the PSO/shader loading screen (§17.3). |

<a id="177-module-boilerplate"></a>
### 17.7 Module boilerplate
`StillHear.h` / `StillHear.cpp` implement the primary game module
(`IMPLEMENT_PRIMARY_GAME_MODULE`) and register the editor detail customizations (e.g. the input
data customization, §2 — where applicable). `StillHear.Build.cs` declares the dependencies
(documented in §0.2).

> 🔧 **Developer:** editor-only customizations and any startup registration belong in the
> module's `StartupModule`/`ShutdownModule`. Runtime globals belong on the GameInstance or a
> subsystem, not in the module.
