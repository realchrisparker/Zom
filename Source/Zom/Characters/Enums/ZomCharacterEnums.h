// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZomCharacterEnums.generated.h"


/**
 * Movement gait for a character, used to drive locomotion speed and animation blending.
 */
UENUM(BlueprintType, meta = (DisplayName = "Zom Character Gait"))
enum class EGait : uint8
{
	Walk	UMETA(DisplayName = "Walk"),
	Run		UMETA(DisplayName = "Run"),
	Sprint	UMETA(DisplayName = "Sprint")
};

/**
 * Directional facing of movement relative to the character, used to select directional locomotion blends.
 */
UENUM(BlueprintType, meta = (DisplayName = "Zom Movement Direction"))
enum class EMovementDirection : uint8
{
	F	UMETA(DisplayName = "F"),
	B	UMETA(DisplayName = "B"),
	LL	UMETA(DisplayName = "LL"),
	LR	UMETA(DisplayName = "LR"),
	RL	UMETA(DisplayName = "RL"),
	RR	UMETA(DisplayName = "RR")
};

/**
 * High level movement mode for a character. Named ECharacterMovementMode to avoid colliding with the engine's global EMovementMode.
 */
UENUM(BlueprintType, meta = (DisplayName = "Zom Character Movement Mode"))
enum class ECharacterMovementMode : uint8
{
	OnGround	UMETA(DisplayName = "OnGround"),
	InAir		UMETA(DisplayName = "InAir"),
	Sliding		UMETA(DisplayName = "Sliding"),
	Traversing	UMETA(DisplayName = "Traversing"),
	Ragdoll		UMETA(DisplayName = "Ragdoll"),
	Flying		UMETA(DisplayName = "Flying")
};

/**
 * Whether a character is idle or actively moving.
 */
UENUM(BlueprintType, meta = (DisplayName = "Zom Movement State"))
enum class EMovementState : uint8
{
	Idle	UMETA(DisplayName = "Idle"),
	Moving	UMETA(DisplayName = "Moving")
};

/**
 * How a character's rotation is driven relative to its movement or view direction.
 */
UENUM(BlueprintType, meta = (DisplayName = "Zom Rotation Mode"))
enum class ERotationMode : uint8
{
	OrientToMovement	UMETA(DisplayName = "OrientToMovement"),
	Strafe				UMETA(DisplayName = "Strafe"),
	Aim					UMETA(DisplayName = "Aim")
};

/**
 * Stance for a character.
 */
UENUM(BlueprintType, meta = (DisplayName = "Zom Character Stance"))
enum class EStance : uint8
{
	Stand	UMETA(DisplayName = "Stand"),
	Crouch	UMETA(DisplayName = "Crouch")
};

/**
 * Combat state for a character.
 */
UENUM(BlueprintType, meta = (DisplayName = "Zom Character Combat State"))
enum class ECombatState : uint8
{
	None	UMETA(DisplayName = "None"),
	Unarmed	UMETA(DisplayName = "Unarmed"),
	Machete	UMETA(DisplayName = "Machete"),
	Pistol	UMETA(DisplayName = "Pistol"),
	Rifle	UMETA(DisplayName = "Rifle"),
	Shotgun	UMETA(DisplayName = "Shotgun"),
};

/**
 * Zombie types.
 */
UENUM(BlueprintType, meta = (DisplayName = "Zom Zombie Type"))
enum class EZombieType : uint8
{
	Walker	UMETA(DisplayName = "Walker"),
	Runner	UMETA(DisplayName = "Runner"),
	Tank	UMETA(DisplayName = "Tank"),
	Bloater	UMETA(DisplayName = "Bloater"),
};

/**
 * Used by UZomZombiePoolSubsystem to enforce the two separate density budgets (5-15 crowd, 1-2
 * Bloater) called out in the design doc - two independently-tracked counters, cleaner as an enum than a bool.
 */
UENUM(BlueprintType, meta = (DisplayName = "Zom Zombie Category"))
enum class EZomZombieCategory : uint8
{
	Crowd	UMETA(DisplayName = "Crowd"),
	Bloater	UMETA(DisplayName = "Bloater"),
	Boss	UMETA(DisplayName = "Boss")
};
/**
 * How UZomCharacterNoiseComponent decides a foot has planted. Both routes converge on the same
 * NotifyFootstep() entry point, so the noise math, audio call and debug drawing exist exactly once.
 */
UENUM(BlueprintType, meta = (DisplayName = "Zom Footstep Trigger"))
enum class EZomFootstepTrigger : uint8
{
	// Distance-based cadence from a self-re-arming timer. Works on any animation, including motion-matched
	// locomotion with no authored notifies, so it needs zero content to function. The default.
	StrideTimer	UMETA(DisplayName = "Stride Timer"),

	// UAnimNotify_ZomFootstep placed on the locomotion animations. Frame-accurate to the visible foot
	// plant, at the cost of authoring and maintaining a notify on every locomotion asset.
	AnimNotify	UMETA(DisplayName = "Anim Notify")
};

/**
 * Which foot a footstep belongs to. Selects the mesh socket a noise emission originates from.
 */
UENUM(BlueprintType, meta = (DisplayName = "Zom Foot"))
enum class EZomFoot : uint8
{
	Left		UMETA(DisplayName = "Left"),
	Right		UMETA(DisplayName = "Right"),

	// Origin falls back to the capsule bottom rather than a foot socket.
	Unspecified	UMETA(DisplayName = "Unspecified")
};
