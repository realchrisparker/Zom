// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Zom/Abilities/ZomGameplayEffect.h"
#include "ZomGE_StaminaDrain.generated.h"


/**
 * Sprint and heavy-action Stamina cost. Unlike UZomGE_Damage/Stagger (applied by many sources with widely
 * varying per-hit magnitudes, hence SetByCaller), a cost is fundamentally "this specific ability costs X" -
 * so the modifier magnitude here is a plain content-editable ScalableFloat (left at engine default in C++),
 * tuned per ability via a Blueprint child (e.g. GE_StaminaDrain_HeavyAttack, GE_StaminaDrain_Dodge each with
 * their own negative magnitude) and consumed through UZomGameplayAbility's standard CostGameplayEffectClass
 * + CommitAbility() pipeline rather than manual SetByCaller injection.
 */
UCLASS()
class ZOM_API UZomGE_StaminaDrain : public UZomGameplayEffect
{
	GENERATED_BODY()

public:
	UZomGE_StaminaDrain();
};
