// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Zom/Abilities/ZomGameplayEffect.h"
#include "ZomGE_RestoreFromSave.generated.h"


/**
 * [New, not in the dev doc's Section 4.3 table] Reapplies saved Health/Stamina via a Gameplay Effect rather
 * than a direct attribute write, per Section 11's explicit restore-flow requirement ("stay consistent with
 * GAS usage elsewhere"). The doc names this constraint but never names an effect class for it - added here to
 * satisfy it. Two independent Override modifiers (Health, Stamina), set via Zom.SetByCaller.RestoreHealth/
 * RestoreStamina respectively.
 */
UCLASS()
class ZOM_API UZomGE_RestoreFromSave : public UZomGameplayEffect
{
	GENERATED_BODY()

public:
	UZomGE_RestoreFromSave();
};
