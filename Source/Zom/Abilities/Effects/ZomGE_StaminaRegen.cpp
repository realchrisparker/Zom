// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Abilities/Effects/ZomGE_StaminaRegen.h"
#include "Zom/Abilities/AttributeSets/ZomPlayerAttributeSet.h"


namespace
{
	// Stamina restored per second while this effect is active. Single tunable number - retune directly for
	// game balance, same as UZomPlayerAttributeSet's InitStamina/InitMaxStamina defaults.
	constexpr float StaminaRegenPerSecond = 0.5f;

	// How often the modifier reapplies. Smaller = smoother regen curve at the cost of more frequent
	// attribute-change events (OnRep_Stamina, GAS delegates, etc).
	constexpr float StaminaRegenPeriodSeconds = 1.0f;
}

UZomGE_StaminaRegen::UZomGE_StaminaRegen()
{
	DurationPolicy = EGameplayEffectDurationType::Infinite; // This effect lasts indefinitely, providing continuous stamina regeneration.
	Period = FScalableFloat(StaminaRegenPeriodSeconds); // How often the modifier reapplies. Smaller = smoother regen curve.

	// Don't grant a free tick's worth the instant this effect is applied (character possess/spawn) - wait for
	// the first Period to elapse like every subsequent tick does.
	bExecutePeriodicEffectOnApplication = false;

	// Set up the periodic stamina regen modifier.
	FGameplayModifierInfo StaminaModifier;
	StaminaModifier.Attribute = UZomPlayerAttributeSet::GetStaminaAttribute();
	StaminaModifier.ModifierOp = EGameplayModOp::Additive;
	StaminaModifier.ModifierMagnitude = FScalableFloat(StaminaRegenPerSecond * StaminaRegenPeriodSeconds);

	// Add the modifier to the effect's list of modifiers.
	Modifiers.Add(StaminaModifier);
}
