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
	Unarmed	UMETA(DisplayName = "Unarmed"),
	Machete	UMETA(DisplayName = "Machete"),
	Pistol	UMETA(DisplayName = "Pistol"),
	Rifle	UMETA(DisplayName = "Rifle"),
	Shotgun	UMETA(DisplayName = "Shotgun"),
};