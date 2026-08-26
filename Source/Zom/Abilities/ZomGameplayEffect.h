// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "ZomGameplayEffect.generated.h"


/**
 * [Proposed] Shared effect base for the four Zom effects. GameplayEffects are mostly data (duration policy,
 * modifiers, tags), not code, so unlike UZomGameplayAbility there's little runtime logic to hoist here - its
 * real value is a single place to anchor naming/tag conventions across UZomGE_Damage/StaminaDrain/Infection/Stagger
 * (Section 4.3 of the dev doc).
 */
UCLASS()
class ZOM_API UZomGameplayEffect : public UGameplayEffect
{
	GENERATED_BODY()
};
