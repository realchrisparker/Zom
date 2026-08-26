// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Zom/Abilities/ZomGameplayAbility.h"
#include "ZomGA_RangedShoot.generated.h"


/**
 * [Design] Ranged weapon fire. The actual line trace/projectile and ammo-consumption logic depends on which
 * weapon is equipped (Section 7's UZomInventoryComponent, not yet built) and isn't fabricated here - this
 * class establishes the activation shape (commit, no Stamina cost per the dev doc).
 */
UCLASS()
class ZOM_API UZomGA_RangedShoot : public UZomGameplayAbility
{
	GENERATED_BODY()

public:
	UZomGA_RangedShoot();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
