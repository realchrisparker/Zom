// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZomCheckpointEnums.generated.h"


/** [Design] Section 11 of the dev doc. */
UENUM(BlueprintType)
enum class EZomCheckpointID : uint8
{
	Entry		UMETA(DisplayName = "Entry"),
	PostFetch	UMETA(DisplayName = "PostFetch"),
	PostRepair	UMETA(DisplayName = "PostRepair"),
	PreBoss		UMETA(DisplayName = "PreBoss")
};
