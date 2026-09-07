// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Abilities/Effects/ZomGE_StaminaDrain.h"
#include "Zom/Abilities/AttributeSets/ZomPlayerAttributeSet.h"


const FName UZomGE_StaminaDrain::StaminaCostSetByCallerName(TEXT("Penalty"));

UZomGE_StaminaDrain::UZomGE_StaminaDrain()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayModifierInfo StaminaModifier;
	StaminaModifier.Attribute = UZomPlayerAttributeSet::GetStaminaAttribute();
	StaminaModifier.ModifierOp = EGameplayModOp::Additive;

	FSetByCallerFloat SetByCallerMagnitude;
	SetByCallerMagnitude.DataName = StaminaCostSetByCallerName;
	StaminaModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCallerMagnitude);

	Modifiers.Add(StaminaModifier);
}
