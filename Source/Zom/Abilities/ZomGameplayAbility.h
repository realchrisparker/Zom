// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "ZomGameplayAbility.generated.h"


/**
 * [Proposed] Shared ability base for all six Zom abilities. Sets InstancingPolicy/NetExecutionPolicy once,
 * consistent with the full-GAS decision to teach production patterns even though prediction goes unused in a
 * singleplayer build, and blocks activation while staggered so no individual ability class has to remember to
 * (Section 4.2 of the dev doc).
 */
UCLASS()
class ZOM_API UZomGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UZomGameplayAbility();
};
