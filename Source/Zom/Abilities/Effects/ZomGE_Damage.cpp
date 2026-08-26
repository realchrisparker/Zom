// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Abilities/Effects/ZomGE_Damage.h"
#include "Zom/Abilities/AttributeSets/ZomAttributeSetBase.h"
#include "Zom/Misc/ZomGameplayTags.h"


UZomGE_Damage::UZomGE_Damage()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FSetByCallerFloat SetByCallerMagnitude;
	SetByCallerMagnitude.DataTag = TAG_Zom_SetByCaller_Magnitude.GetTag();

	FGameplayModifierInfo DamageModifier;
	DamageModifier.Attribute = UZomAttributeSetBase::GetDamageAttribute();
	DamageModifier.ModifierOp = EGameplayModOp::Additive;
	DamageModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCallerMagnitude);

	Modifiers.Add(DamageModifier);
}
