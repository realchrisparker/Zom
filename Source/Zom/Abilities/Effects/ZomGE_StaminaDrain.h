// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Zom/Abilities/Effects/Base/ZomGameplayEffectBase.h"
#include "ZomGE_StaminaDrain.generated.h"


/**
 * Stamina cost for MCS-driven attacks. The modifier magnitude is SetByCaller, keyed by StaminaCostSetByCallerName
 * (see .cpp) - unlike Health/MaxHealth-style "this ability costs a fixed X", one GAS ability here (e.g.
 * UZomGA_HeavyAttack) plays many different resolved FMCS_AttackEntry rows via the MCS chooser, each able to
 * carry its own FMCS_AttackEntry::StaminaCost. Applied manually via UZomGameplayAbilityBase::
 * ApplyStaminaCostForCurrentAttack() rather than through CostGameplayEffectClass + CommitAbility(), since the
 * automatic cost pipeline has no hook for injecting a per-activation SetByCaller value before it builds its
 * own spec.
 */
UCLASS()
class ZOM_API UZomGE_StaminaDrain : public UZomGameplayEffectBase
{
	GENERATED_BODY()

public:
	UZomGE_StaminaDrain();

	// SetByCaller key for this effect's Stamina modifier magnitude. Shared with
	// UZomGameplayAbilityBase::ApplyStaminaCostForCurrentAttack, which is the only intended caller.
	static const FName StaminaCostSetByCallerName;
};
