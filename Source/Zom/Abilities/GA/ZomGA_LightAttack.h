// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Zom/Abilities/ZomGameplayAbility.h"
#include "ZomGA_LightAttack.generated.h"


/**
 * Melee light attack. Commits (cooldown only, no Stamina cost per the dev doc) and ends immediately -
 * the real hit-detection/damage-application flow is driven by an AnimNotify on the weapon's light-attack
 * montage (content, not yet authored), which is expected to call into UZomInventoryComponent/UZomGE_Damage
 * once Section 7's weapon system exists. This class establishes the activation shape now.
 */
UCLASS()
class ZOM_API UZomGA_LightAttack : public UZomGameplayAbility
{
	GENERATED_BODY()

public:
	UZomGA_LightAttack();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
