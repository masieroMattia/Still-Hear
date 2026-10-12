#include "GameplayAbilitySystem/Tags/GameplayTags.h"

#pragma region MACROS
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayAbility, "GameplayAbility");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayAbility_Active, "GameplayAbility.Active");
#pragma endregion

#pragma region SHARED
#pragma region Abilities
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayAbility_HoldInteraction, "GameplayAbility.HoldInteraction");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayAbility_HoldInteraction_Active, "GameplayAbility.HoldInteraction.Active");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayAbility_Death, "GameplayAbility.Death");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayAbility_Death_Active, "GameplayAbility.Death.Active");
#pragma endregion

#pragma region Status
UE_DEFINE_GAMEPLAY_TAG(TAG_Status_FreeDragging, "Status.FreeDragging");
UE_DEFINE_GAMEPLAY_TAG(TAG_Status_RailDragging, "Status.RailDragging");
UE_DEFINE_GAMEPLAY_TAG(TAG_Status_Death, "Status.Death");
UE_DEFINE_GAMEPLAY_TAG(TAG_Status_Falling, "Status.Falling");
#pragma endregion

#pragma region Event
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_AttackHit, "Event.AttackHit");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_StopInteraction, "Event.StopInteraction");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Interaction_Success, "Event.Interaction.Success");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Resonance_CrystalBroken, "Event.Resonance.CrystalBroken");
#pragma endregion
#pragma endregion

#pragma region MAIN_CHARACTER
#pragma region Abilities
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayAbility_MainCharacter, "GameplayAbility.MainCharacter");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayAbility_MainCharacter_Jump, "GameplayAbility.MainCharacter.Jump");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayAbility_MainCharacter_Jump_Active, "GameplayAbility.MainCharacter.Jump.Active");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayAbility_MainCharacter_Sprint, "GameplayAbility.MainCharacter.Sprint");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayAbility_MainCharacter_Sprint_Active, "GameplayAbility.MainCharacter.Sprint.Active");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayAbility_MainCharacter_Crouch, "GameplayAbility.MainCharacter.Crouch");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayAbility_MainCharacter_Crouch_Active, "GameplayAbility.MainCharacter.Crouch.Active");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayAbility_MainCharacter_Climb, "GameplayAbility.MainCharacter.Climb");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayAbility_MainCharacter_Climb_Active, "GameplayAbility.MainCharacter.Climb.Active");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayAbility_MainCharacter_Parry, "GameplayAbility.MainCharacter.Parry");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayAbility_MainCharacter_Parry_Active, "GameplayAbility.MainCharacter.Parry.Active");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayAbility_MainCharacter_Resonance, "GameplayAbility.MainCharacter.Resonance");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayAbility_MainCharacter_Resonance_Active, "GameplayAbility.MainCharacter.Resonance.Active");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayAbility_MainCharacter_TapInteraction, "GameplayAbility.MainCharacter.TapInteraction");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayAbility_MainCharacter_TapInteraction_Active, "GameplayAbility.MainCharacter.TapInteraction.Active");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayAbility_MainCharacter_HoldInteraction, "GameplayAbility.MainCharacter.DragInteraction");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayAbility_MainCharacter_HoldInteraction_Active, "GameplayAbility.MainCharacter.DragInteraction.Active");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayAbility_MainCharacter_LowGetOnTop, "GameplayAbility.MainCharacter.LowGetOnTop");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayAbility_MainCharacter_LowGetOnTop_Active, "GameplayAbility.MainCharacter.LowGetOnTop.Active");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayAbility_MainCharacter_LowVault, "GameplayAbility.MainCharacter.LowVault");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayAbility_MainCharacter_LowVault_Active, "GameplayAbility.MainCharacter.LowVault.Active");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayAbility_MainCharacter_RecallCompanion, "GameplayAbility.MainCharacter.RecallCompanion");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayAbility_MainCharacter_RecallCompanion_Active, "GameplayAbility.MainCharacter.RecallCompanion.Active");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayAbility_MainCharacter_SetCompanionFree, "GameplayAbility.MainCharacter.SetCompanionFree");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayAbility_MainCharacter_SetCompanionFree_Active, "GameplayAbility.MainCharacter.SetCompanionFree.Active");
#pragma endregion 

#pragma region Status
UE_DEFINE_GAMEPLAY_TAG(TAG_Status_MainCharacter_Crouched, "Status.MainCharacter.Crouched");
UE_DEFINE_GAMEPLAY_TAG(TAG_Status_MainCharacter_ParryCooldown, "Status.MainCharacter.ParryCooldown");
UE_DEFINE_GAMEPLAY_TAG(TAG_Status_MainCharacter_ForceMoving, "Status.MainCharacter.ForceMoving");
UE_DEFINE_GAMEPLAY_TAG(TAG_Status_MainCharacter_Moving, "Status.MainCharacter.Moving");
UE_DEFINE_GAMEPLAY_TAG(TAG_Status_MainCharacter_PreventJumping, "Status.MainCharacter.PreventJumping");
UE_DEFINE_GAMEPLAY_TAG(TAG_Status_MainCharacter_PreventDeath, "Status.MainCharacter.PreventDeath");
#pragma endregion

#pragma region Events
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_MainCharacter_StopResonance, "Event.MainCharacter.StopResonance");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_MainCharacter_ActivateResonance, "Event.MainCharacter.ActivateResonance");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_MainCharacter_EndCrouch, "Event.MainCharacter.EndCrouch");
#pragma endregion

#pragma region Cues
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayCue_MainCharacter_Parry, "GameplayCue.MainCharacter.Parry");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayCue_MainCharacter_ParryImpact, "GameplayCue.MainCharacter.ParryImpact");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayCue_MainCharacter_ParryCooldownFinished, "GameplayCue.MainCharacter.ParryCooldownFinished");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayCue_MainCharacter_Death, "GameplayCue.MainCharacter.Death");
#pragma endregion 

#pragma region Data
UE_DEFINE_GAMEPLAY_TAG(TAG_Data_MainCharacter_ParryCooldown, "Data.MainCharacter.ParryCooldown");
UE_DEFINE_GAMEPLAY_TAG(TAG_Data_MainCharacter_CrouchCooldown, "Data.MainCharacter.CrouchCooldown");
#pragma endregion
#pragma endregion

#pragma region COMPANION
#pragma region Abilities
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayAbility_Companion_SoundWave, "GameplayAbility.Companion.SoundWave");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayAbility_Companion_SoundWave_Active, "GameplayAbility.Companion.SoundWave.Active");
#pragma endregion

#pragma region Status
UE_DEFINE_GAMEPLAY_TAG(TAG_Status_Companion_Outside, "Status.Companion.Outside");
UE_DEFINE_GAMEPLAY_TAG(TAG_Status_Companion_Aiming, "Status.Companion.Aiming");
UE_DEFINE_GAMEPLAY_TAG(TAG_Status_Companion_SoundWaveCooldown, "Status.Companion.SoundWaveCooldown");
#pragma endregion

#pragma region Event
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Companion_Recall, "Event.Companion.Recall");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Companion_Spawn, "Event.Companion.Spawn");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Companion_Despawn, "Event.Companion.Despawn");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Companion_ShootSoundWave, "Event.Companion.ShootSoundWave");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Companion_SwitchSoundWaveTarget, "Event.Companion.SwitchSoundWaveTarget");
#pragma endregion

#pragma region Cues
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayCue_Companion_SoundWaveCharge, "GameplayCue.Companion.SoundWaveCharge");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayCue_Companion_SoundWaveAim, "GameplayCue.Companion.SoundWaveAim");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayCue_Companion_SoundWaveShoot, "GameplayCue.Companion.SoundWaveShoot");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayCue_Companion_SoundWaveFinishCooldown, "GameplayCue.Companion.SoundWaveFinishCooldown");
#pragma endregion

#pragma region Data
UE_DEFINE_GAMEPLAY_TAG(TAG_Data_SoundWaveChargeDuration, "Data.SoundWaveChargeDuration");
UE_DEFINE_GAMEPLAY_TAG(TAG_Data_SoundWaveCooldown, "Data.SoundWaveCooldown");
#pragma endregion
#pragma endregion

#pragma region ENEMIES
#pragma region Abilities
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayAbility_EnemyAI_Attack, "GameplayAbility.EnemyAI.Attack");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayAbility_EnemyAI_Attack_Active, "GameplayAbility.EnemyAI.Attack.Active");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayAbility_EnemyAI_CloseAttack, "GameplayAbility.EnemyAI.CloseAttack");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayAbility_EnemyAI_CloseAttack_Active, "GameplayAbility.EnemyAI.CloseAttack.Active");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayAbility_EnemyAI_MantisShift, "GameplayAbility.EnemyAI.MantisShift");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayAbility_EnemyAI_MantisShift_Active, "GameplayAbility.EnemyAI.MantisShift.Active");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayAbility_EnemyAI_MantisShift_Nav, "GameplayAbility.EnemyAI.MantisShift.Nav");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayAbility_EnemyAI_MantisShift_Attack, "GameplayAbility.EnemyAI.MantisShift.Attack");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayAbility_EnemyAI_WormRoar, "GameplayAbility.EnemyAI.Roar");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayAbility_EnemyAI_WormRoar_Active, "GameplayAbility.EnemyAI.Roar.Active");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayAbility_EnemyAI_WormDolphinDive, "GameplayAbility.EnemyAI.DolphinDive");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayAbility_EnemyAI_WormDolphinDive_Active, "GameplayAbility.EnemyAI.DolphinDive.Active");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayAbility_EnemyAI_Stun, "GameplayAbility.EnemyAI.Stun");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayAbility_EnemyAI_Stun_Active, "GameplayAbility.EnemyAI.Stun.Active");
#pragma endregion

#pragma region Status
UE_DEFINE_GAMEPLAY_TAG(TAG_Status_EnemyAI_Stunned, "Status.EnemyAI.Stunned");
UE_DEFINE_GAMEPLAY_TAG(TAG_Status_EnemyAI_AttackCooldown, "Status.EnemyAI.AttackCooldown");
UE_DEFINE_GAMEPLAY_TAG(TAG_Status_EnemyAI_Unaware, "Status.EnemyAI.Unaware");
UE_DEFINE_GAMEPLAY_TAG(TAG_Status_EnemyAI_Suspicious, "Status.EnemyAI.Suspicious");
UE_DEFINE_GAMEPLAY_TAG(TAG_Status_EnemyAI_Alerted, "Status.EnemyAI.Alerted");
UE_DEFINE_GAMEPLAY_TAG(TAG_Status_EnemyAI_Hunting, "Status.EnemyAI.Hunting");
UE_DEFINE_GAMEPLAY_TAG(TAG_Status_EnemyAI_WormDolphinDiveCooldown, "Status.EnemyAI.DolphinDiveCooldown");
#pragma endregion

#pragma region Events
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_EnemyAI_Hit, "Event.EnemyAI.Hit");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_EnemyAI_WormRoar, "Event.EnemyAI.WormRoar");
#pragma endregion

#pragma region Cues
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayCue_EnemyAI_WormRoar, "GameplayCue.EnemyAI.WormRoar");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayCue_EnemyAI_Stun, "GameplayCue.EnemyAI.Stun");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayCue_EnemyAI_Mantis_Shift_Loop, "GameplayCue.EnemyAI.Mantis.Shift.Loop");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayCue_EnemyAI_Mantis_Shift_Burst, "GameplayCue.EnemyAI.Mantis.Shift.Burst");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayCue_EnemyAI_AttackFeedback, "GameplayCue.EnemyAI.AttackFeedback");
#pragma endregion

#pragma region Data
UE_DEFINE_GAMEPLAY_TAG(TAG_Data_EnemyAI_AttackCooldown, "Data.EnemyAI.AttackCooldown");
UE_DEFINE_GAMEPLAY_TAG(TAG_Data_EnemyAI_StunCooldown, "Data.EnemyAI.StunCooldown");
#pragma endregion
#pragma endregion

#pragma region INPUTS
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_InputReleased_Crouch, "Event.InputReleased.Crouch");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_InputReleased_Sprint, "Event.InputReleased.Sprint");
#pragma endregion

#pragma region COLLISIONS
#pragma region Events
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Collision_Activate, "Event.Collision.Activate");
#pragma endregion
#pragma endregion

#pragma region INTERACTIONS
UE_DEFINE_GAMEPLAY_TAG(TAG_Interact_Tap, "Interact.Tap");
UE_DEFINE_GAMEPLAY_TAG(TAG_Interact_Deflect, "Interact.Deflect");
UE_DEFINE_GAMEPLAY_TAG(TAG_Interact_Projectile, "Interact.Projectile");
UE_DEFINE_GAMEPLAY_TAG(TAG_Interact_Hold_DragFree, "Interact.Hold.DragFree");
UE_DEFINE_GAMEPLAY_TAG(TAG_Interact_Hold_DragRail, "Interact.Hold.DragRail");
UE_DEFINE_GAMEPLAY_TAG(TAG_Interact_Resonance, "Interact.Resonance");
#pragma endregion

#pragma region UI
#pragma region Layers
UE_DEFINE_GAMEPLAY_TAG(TAG_UI_Layer_Game, "UI.Layer.Game");
UE_DEFINE_GAMEPLAY_TAG(TAG_UI_Layer_Menu, "UI.Layer.Menu");
UE_DEFINE_GAMEPLAY_TAG(TAG_UI_Layer_Window, "UI.Layer.Window");
UE_DEFINE_GAMEPLAY_TAG(TAG_UI_Layer_Modal, "UI.Layer.Modal");
UE_DEFINE_GAMEPLAY_TAG(TAG_UI_Layer_Debug, "UI.Layer.Debug");
#pragma endregion
#pragma region Actions
UE_DEFINE_GAMEPLAY_TAG(TAG_UI_Action_Settings, "UI.Action.Settings");
#pragma endregion
#pragma endregion

#pragma region AUDIO
#pragma region States
UE_DEFINE_GAMEPLAY_TAG(TAG_Audio_State_MainMenu, "Audio.State.MainMenu");
#pragma endregion
#pragma endregion

#pragma region UTILS
UE_DEFINE_GAMEPLAY_TAG(TAG_Climb, "Climb");
UE_DEFINE_GAMEPLAY_TAG(TAG_Counterweight, "Counterweight");
UE_DEFINE_GAMEPLAY_TAG(TAG_AttackHitBox, "AttackHitBox");
UE_DEFINE_GAMEPLAY_TAG(TAG_ParrySphere, "ParrySphere");
UE_DEFINE_GAMEPLAY_TAG(TAG_MainCharacter, "MainCharacter");
UE_DEFINE_GAMEPLAY_TAG(TAG_Enemy, "Enemy");

#pragma region Cues
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayCue_GroundDebris, "GameplayCue.GroundDebris");
#pragma endregion
#pragma endregion
