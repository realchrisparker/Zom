// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZomAIEnums.generated.h"


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
