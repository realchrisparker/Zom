// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Zom/Abilities/ZomGameplayAbility.h"
#include "ZomGA_HeavyAttack.generated.h"


/**
 * [Design] Melee heavy attack, gated by Stamina cost. CostGameplayEffectClass defaults to the base
 * UZomGE_StaminaDrain (0 magnitude); assign a tuned Blueprint child (e.g. GE_StaminaDrain_HeavyAttack) to
 * this ability's own Blueprint child to set the actual cost.
 */
UCLASS()
class ZOM_API UZomGA_HeavyAttack : public UZomGameplayAbility
{
	GENERATED_BODY()

public:
	UZomGA_HeavyAttack();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
