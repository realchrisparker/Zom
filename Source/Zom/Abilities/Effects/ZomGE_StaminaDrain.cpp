// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Abilities/Effects/ZomGE_StaminaDrain.h"
#include "Zom/Abilities/AttributeSets/ZomPlayerAttributeSet.h"


UZomGE_StaminaDrain::UZomGE_StaminaDrain()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayModifierInfo StaminaModifier;
	StaminaModifier.Attribute = UZomPlayerAttributeSet::GetStaminaAttribute();
	StaminaModifier.ModifierOp = EGameplayModOp::Additive;
	// ModifierMagnitude left at its ScalableFloat default (0) - each Blueprint child (per-ability cost) sets
	// its own negative magnitude.

	Modifiers.Add(StaminaModifier);
}
