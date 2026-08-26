// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Zom/Abilities/ZomGameplayEffect.h"
#include "ZomGE_Damage.generated.h"


/**
 * Applied by melee/ranged hits. Writes to the shared Damage meta-attribute (Section 4.1) via a SetByCaller
 * modifier (Zom.SetByCaller.Magnitude) so the actual damage amount comes from the attacking ability, not a
 * hardcoded value here.
 */
UCLASS()
class ZOM_API UZomGE_Damage : public UZomGameplayEffect
{
	GENERATED_BODY()

public:
	UZomGE_Damage();
};
