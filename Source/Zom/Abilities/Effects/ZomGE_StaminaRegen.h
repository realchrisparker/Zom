// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Zom/Abilities/Effects/Base/ZomGameplayEffectBase.h"
#include "ZomGE_StaminaRegen.generated.h"


/**
 * Passive Stamina regeneration. Infinite duration with a Period, so once applied it just keeps re-executing
 * its modifier every Period seconds for the rest of the character's life - no manual re-triggering needed.
 * Granted once via AZomPlayerCharacter's DefaultGameplayEffects (see AZomCharacterBase::
 * GrantDefaultAbilitiesAndEffects, which applies it as soon as the ASC is initialized). Relies on
 * UZomPlayerAttributeSet's own [0, MaxStamina] clamp to cap regen at full Stamina - no separate stop condition
 * needed here.
 */
UCLASS()
class ZOM_API UZomGE_StaminaRegen : public UZomGameplayEffectBase
{
	GENERATED_BODY()

public:
	UZomGE_StaminaRegen();
};
