// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZomObjectiveEnums.generated.h"


/**
 * [Proposed] The five-step objective chain (Section 8 of the dev doc). This is the interim implementation -
 * the future Objective plugin (not scheduled) retires this enum in favor of tagged UObjectiveDefinition content.
 */
UENUM(BlueprintType)
enum class EZomObjectiveStep : uint8
{
	Fetch		UMETA(DisplayName = "Fetch"),
	Repair		UMETA(DisplayName = "Repair"),
	Defend		UMETA(DisplayName = "Defend"),
	Boss		UMETA(DisplayName = "Boss"),
	Extracted	UMETA(DisplayName = "Extracted")
};
