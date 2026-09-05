// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NativeGameplayTags.h"

// Native Gameplay Tags for the Zom project. Declared here, defined with descriptions in ZomGameplayTags.cpp.
// See Zom_Development_Document.md Section 4.4 for the tag taxonomy (Status / Objective / Perception).

// -------------
// Status (persistent, live on the ASC's tag container; drive animation and ability gating)
// -------------

ZOM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Zom_Status_Attacking);
ZOM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Zom_Status_Staggered);
ZOM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Zom_Status_Dodging);

// -------------
// Combat (asset tags on the C++ attack abilities themselves; the MCS attack DataTables' AttackTag column
// must be authored with these SAME tags so HandleAttackResolved's TryActivateAbilitiesByTag dispatch finds
// the right granted spec)
// -------------

ZOM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Zom_Combat_Attack_Light);
ZOM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Zom_Combat_Attack_Heavy);

// -------------
// Objective (GAS-queryable mirror of EZomObjectiveStep progress; set alongside the enum, never alone)
// -------------

ZOM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Zom_Objective_Fetch_Complete);
ZOM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Zom_Objective_Repair_Complete);
ZOM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Zom_Objective_Defend_Complete);
ZOM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Zom_Objective_Boss_Complete);
ZOM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Zom_Objective_Extracted_Complete);

// -------------
// Perception (transient; passed once through FStateTreeEvent via SendStateTreeEvent, never touch a tag container)
// -------------

ZOM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Zom_Perception_Sight_TargetSeen);
ZOM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Zom_Perception_Sight_TargetLost);
ZOM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Zom_Perception_Hearing_NoiseHeard);
ZOM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Zom_Perception_Damage_Taken);

// -------------
// SetByCaller (consistent scheme every UZomGameplayEffect uses for its runtime-supplied magnitude/duration,
// per Section 4.3 of the dev doc, so effect tuning comes from ability/spawn code rather than being hardcoded)
// -------------

ZOM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Zom_SetByCaller_Magnitude);
ZOM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Zom_SetByCaller_Duration);

// UZomGE_RestoreFromSave needs two independent modifiers (Health, Stamina) in one spec application, so the
// single generic Magnitude tag above can't serve both at once - each needs its own tag.
ZOM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Zom_SetByCaller_RestoreHealth);
ZOM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Zom_SetByCaller_RestoreStamina);

// -------------
// Boss (Section 6 of the dev doc mentions a phase-transition tag "toggled off the AttributeSet's health-change
// delegate" but never names it - this is that tag)
// -------------

ZOM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Zom_Boss_Phase2);

// -------------
// Camera (drives which CameraRigAsset UZomPlayerCameraDirectorEvaluator activates for AZomPlayerCharacter;
// add new states here as gameplay systems need their own rig, mirroring them in the CDE_Player asset's
// CameraRigsByTag map)
// -------------

ZOM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Zom_Camera_State_Default);
