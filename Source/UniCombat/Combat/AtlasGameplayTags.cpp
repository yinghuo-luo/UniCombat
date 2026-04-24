// Copyright Epic Games, Inc. All Rights Reserved.

#include "Combat/AtlasGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG(TAG_Faction_Player, "Faction.Player");
UE_DEFINE_GAMEPLAY_TAG(TAG_Faction_Enemy, "Faction.Enemy");
UE_DEFINE_GAMEPLAY_TAG(TAG_Faction_Neutral, "Faction.Neutral");
UE_DEFINE_GAMEPLAY_TAG(TAG_Faction_Summon, "Faction.Summon");

UE_DEFINE_GAMEPLAY_TAG(TAG_State_Dead, "State.Dead");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Stunned, "State.Stunned");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_StaminaConsuming, "State.StaminaConsuming");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_StaminaRecovering, "State.StaminaRecovering");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_HealthRecovering, "State.HealthConsuming");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Immobilized, "State.Immobilized");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_KnockedDown, "State.KnockedDown");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_SuperArmor, "State.SuperArmor");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Invincible, "State.Invincible");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Casting, "State.Casting");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Attacking, "State.Attacking");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Combat_Unarmed, "State.Combat.Unarmed");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Combat_Knife, "State.Combat.Knife");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Revealed, "State.Revealed");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_PhaseShifted, "State.PhaseShifted");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Stabilized, "State.Stabilized");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Executable, "State.Executable");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_ExorciseWindow, "State.ExorciseWindow");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_HiddenTrueForm, "State.HiddenTrueForm");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_BrokenVeil, "State.BrokenVeil");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Phase_HangedMatron_PhaseTwo, "State.Phase.HangedMatron.PhaseTwo");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Phase_WellBride_PhaseTwo, "State.Phase.WellBride.PhaseTwo");

UE_DEFINE_GAMEPLAY_TAG(TAG_Type_Humanoid, "Type.Humanoid");
UE_DEFINE_GAMEPLAY_TAG(TAG_Type_Ghost, "Type.Ghost");
UE_DEFINE_GAMEPLAY_TAG(TAG_Type_Corpse, "Type.Corpse");
UE_DEFINE_GAMEPLAY_TAG(TAG_Type_Boss, "Type.Boss");
UE_DEFINE_GAMEPLAY_TAG(TAG_Type_Elite, "Type.Elite");
UE_DEFINE_GAMEPLAY_TAG(TAG_Type_Summon, "Type.Summon");

//能力
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Damage_Test, "Ability.Damage.Test");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Common_HitReact, "Ability.Common.HitReact");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Common_Death, "Ability.Common.Death");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Common_Stunned, "Ability.Common.Stunned");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Common_ExecutionWindow, "Ability.Common.ExecutionWindow");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Common_StaminaRegen, "Ability.Common.StaminaRegen");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Common_HealthRegen, "Ability.Common.HealthRegen");

UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Attack_Light, "Ability.Attack.Light");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Attack_Heavy, "Ability.Attack.Heavy");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Combat_Switch, "Ability.Combat.Switch");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Move_Dodge, "Ability.Move.Dodge");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Move_Sprint, "Ability.Move.Sprint");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Move_Dash, "Ability.Move.Dash");

//输入
UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Damage_Test, "Input.Damage.Test");
UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Common_StaminaRegen, "Input.Common.StaminaRegen");
UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Common_HealthRegen, "Input.Common.HealthRegen");

UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Attack_Light, "Input.Attack.Light");
UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Attack_Heavy, "Input.Attack.Heavy");
UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Combat_Switch, "Input.Combat.Switch");
UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Move_Dodge, "Input.Move.Dodge");
UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Move_Sprint, "Input.Move.Sprint");
UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Move_Dash, "Input.Move.Dash");

UE_DEFINE_GAMEPLAY_TAG(TAG_Interaction_Targetable, "Interaction.Targetable");
UE_DEFINE_GAMEPLAY_TAG(TAG_Interaction_Executable, "Interaction.Executable");
UE_DEFINE_GAMEPLAY_TAG(TAG_Interaction_Exorcise, "Interaction.Exorcise");
UE_DEFINE_GAMEPLAY_TAG(TAG_Interaction_Sealable, "Interaction.Sealable");
UE_DEFINE_GAMEPLAY_TAG(TAG_Interaction_Revealable, "Interaction.Revealable");
UE_DEFINE_GAMEPLAY_TAG(TAG_Interaction_BellReactive, "Interaction.BellReactive");
UE_DEFINE_GAMEPLAY_TAG(TAG_Interaction_Exorcisable, "Interaction.Exorcisable");
UE_DEFINE_GAMEPLAY_TAG(TAG_Interaction_RitualTarget, "Interaction.RitualTarget");
UE_DEFINE_GAMEPLAY_TAG(TAG_Interaction_GhostDomainNode, "Interaction.GhostDomainNode");

UE_DEFINE_GAMEPLAY_TAG(TAG_Rule_WeakToReveal, "Rule.WeakToReveal");
UE_DEFINE_GAMEPLAY_TAG(TAG_Rule_WeakToBell, "Rule.WeakToBell");
UE_DEFINE_GAMEPLAY_TAG(TAG_Rule_WeakToExorcise, "Rule.WeakToExorcise");

UE_DEFINE_GAMEPLAY_TAG(TAG_Damage_Physical, "Damage.Physical");
UE_DEFINE_GAMEPLAY_TAG(TAG_Damage_Spell, "Damage.Spell");
UE_DEFINE_GAMEPLAY_TAG(TAG_Damage_Exorcise, "Damage.Exorcise");
UE_DEFINE_GAMEPLAY_TAG(TAG_Damage_BonusVsGhost, "Damage.BonusVsGhost");
UE_DEFINE_GAMEPLAY_TAG(TAG_Damage_BonusVsCorpse, "Damage.BonusVsCorpse");

UE_DEFINE_GAMEPLAY_TAG(TAG_SetByCaller_Data_Damage, "SetByCaller.Data.Damage");
UE_DEFINE_GAMEPLAY_TAG(TAG_SetByCaller_Data_PoiseDamage, "SetByCaller.Data.PoiseDamage");
UE_DEFINE_GAMEPLAY_TAG(TAG_SetByCaller_Data_Healing, "SetByCaller.Data.Healing");

//Cue
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayCue_Move_Sprint, "GameplayCue.Move.Sprint");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayCue_Move_Dash, "GameplayCue.Move.Dash");