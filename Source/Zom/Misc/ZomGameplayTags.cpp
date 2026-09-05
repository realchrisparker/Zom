// Fill out your copyright notice in the Description page of Project Settings.

#include "Zom/Misc/ZomGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Zom_Status_Attacking, "Zom.Status.Attacking", "Persistent tag on the ASC while an attack ability is active.")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Zom_Status_Staggered, "Zom.Status.Staggered", "Persistent tag on the ASC while staggered; blocks all UZomGameplayAbility activation.")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Zom_Status_Dodging, "Zom.Status.Dodging", "Persistent tag on the ASC while UZomGA_Dodge is active.")

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Zom_Combat_Attack_Light, "Zom.Combat.Attack.Light", "Shared AssetTag on every light-attack UZomGameplayAbility; must match the AttackTag column value on light-attack DataTable rows for TryActivateAbilitiesByTag dispatch.")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Zom_Combat_Attack_Heavy, "Zom.Combat.Attack.Heavy", "Shared AssetTag on every heavy-attack UZomGameplayAbility; must match the AttackTag column value on heavy-attack DataTable rows for TryActivateAbilitiesByTag dispatch.")

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Zom_Objective_Fetch_Complete, "Zom.Objective.Fetch.Complete", "GAS-queryable mirror of EZomObjectiveStep::Fetch completion.")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Zom_Objective_Repair_Complete, "Zom.Objective.Repair.Complete", "GAS-queryable mirror of EZomObjectiveStep::Repair completion.")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Zom_Objective_Defend_Complete, "Zom.Objective.Defend.Complete", "GAS-queryable mirror of EZomObjectiveStep::Defend completion.")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Zom_Objective_Boss_Complete, "Zom.Objective.Boss.Complete", "GAS-queryable mirror of EZomObjectiveStep::Boss completion.")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Zom_Objective_Extracted_Complete, "Zom.Objective.Extracted.Complete", "GAS-queryable mirror of EZomObjectiveStep::Extracted completion.")

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Zom_Perception_Sight_TargetSeen, "Zom.Perception.Sight.TargetSeen", "Raised by AZomZombieAIController when the sight sense successfully senses the player.")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Zom_Perception_Sight_TargetLost, "Zom.Perception.Sight.TargetLost", "Raised by AZomZombieAIController when the sight sense loses the player.")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Zom_Perception_Hearing_NoiseHeard, "Zom.Perception.Hearing.NoiseHeard", "Raised by AZomZombieAIController when the hearing sense registers a stimulus.")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Zom_Perception_Damage_Taken, "Zom.Perception.Damage.Taken", "Raised by AZomZombieAIController when the zombie takes damage, regardless of whether the instigator is in sight/hearing range.")

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Zom_SetByCaller_Magnitude, "Zom.SetByCaller.Magnitude", "Runtime-supplied modifier magnitude for UZomGameplayEffect subclasses (e.g. damage amount, stamina cost).")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Zom_SetByCaller_Duration, "Zom.SetByCaller.Duration", "Runtime-supplied duration for UZomGameplayEffect subclasses that need one set dynamically (e.g. infection extension).")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Zom_SetByCaller_RestoreHealth, "Zom.SetByCaller.RestoreHealth", "UZomGE_RestoreFromSave's Health-override modifier magnitude.")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Zom_SetByCaller_RestoreStamina, "Zom.SetByCaller.RestoreStamina", "UZomGE_RestoreFromSave's Stamina-override modifier magnitude.")

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Zom_Boss_Phase2, "Zom.Boss.Phase2", "Toggled on AZomBoss's ASC when Health crosses the phase-two threshold (~50%), off the AttributeSet's health-change delegate.")

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Zom_Camera_State_Default, "Zom.Camera.State.Default", "AZomPlayerCharacter::CameraStateTag's default value; maps to CR_Player_Default in CDE_Player's CameraRigsByTag.")
