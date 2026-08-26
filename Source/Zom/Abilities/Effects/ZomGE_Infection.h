// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Zom/Abilities/ZomGameplayEffect.h"
#include "ZomGE_Infection.generated.h"


/**
 * Shared by bite infection (persistent, cleared by Medicine) and Bloater gas infection (5s duration, extends
 * rather than stacks) - Section 4.3 of the dev doc. The two cases need different DurationPolicy values
 * (Infinite for bite, HasDuration for gas), which is a per-application CDO setting GAS can't vary at runtime,
 * so this C++ base deliberately leaves DurationPolicy unset: author two Blueprint children of this class
 * (e.g. GE_Infection_Bite = Infinite, GE_Infection_Gas = HasDuration + Zom.SetByCaller.Duration), each picking
 * its own policy in the editor.
 *
 * Extend-not-stack (the gas case) uses GAS's native EGameplayEffectStackingDurationPolicy::ExtendDuration -
 * confirmed present in UE 5.8's GameplayEffect.h. This corrects the dev doc's assumption that a custom
 * duration-extend helper is needed; that assumption predates this engine version having a built-in option.
 */
UCLASS()
class ZOM_API UZomGE_Infection : public UZomGameplayEffect
{
	GENERATED_BODY()

public:
	UZomGE_Infection();
};
