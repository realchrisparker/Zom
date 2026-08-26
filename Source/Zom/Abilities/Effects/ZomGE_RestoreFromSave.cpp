// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Abilities/Effects/ZomGE_RestoreFromSave.h"
#include "Zom/Abilities/AttributeSets/ZomAttributeSetBase.h"
#include "Zom/Abilities/AttributeSets/ZomPlayerAttributeSet.h"
#include "Zom/Misc/ZomGameplayTags.h"


UZomGE_RestoreFromSave::UZomGE_RestoreFromSave()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FSetByCallerFloat HealthMagnitude;
	HealthMagnitude.DataTag = TAG_Zom_SetByCaller_RestoreHealth.GetTag();

	FGameplayModifierInfo HealthModifier;
	HealthModifier.Attribute = UZomAttributeSetBase::GetHealthAttribute();
	HealthModifier.ModifierOp = EGameplayModOp::Override;
	HealthModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(HealthMagnitude);
	Modifiers.Add(HealthModifier);

	FSetByCallerFloat StaminaMagnitude;
	StaminaMagnitude.DataTag = TAG_Zom_SetByCaller_RestoreStamina.GetTag();

	FGameplayModifierInfo StaminaModifier;
	StaminaModifier.Attribute = UZomPlayerAttributeSet::GetStaminaAttribute();
	StaminaModifier.ModifierOp = EGameplayModOp::Override;
	StaminaModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(StaminaMagnitude);
	Modifiers.Add(StaminaModifier);
}
