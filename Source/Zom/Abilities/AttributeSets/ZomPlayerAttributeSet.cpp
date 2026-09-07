// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Abilities/AttributeSets/ZomPlayerAttributeSet.h"
#include "Net/UnrealNetwork.h"


UZomPlayerAttributeSet::UZomPlayerAttributeSet()
{
	// Sensible starting point so Stamina drain (UZomGE_StaminaDrain) has something to actually drain -
	// nothing else currently initializes this attribute for the player (unlike zombies/Boss, whose
	// MaxHealth is seeded per-instance from ZombieTypeData/BossData, the player has no equivalent varying
	// source, so a flat compile-time default here is enough). Retune directly if game balance needs differ.
	InitMaxStamina(100.f);
	InitStamina(100.f);
}

void UZomPlayerAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UZomPlayerAttributeSet, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UZomPlayerAttributeSet, MaxStamina, COND_None, REPNOTIFY_Always);
}

void UZomPlayerAttributeSet::ClampStamina(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetStaminaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxStamina());
	}
}

void UZomPlayerAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	ClampStamina(Attribute, NewValue);
}

void UZomPlayerAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);

	ClampStamina(Attribute, NewValue);
}

void UZomPlayerAttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldStamina)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UZomPlayerAttributeSet, Stamina, OldStamina);
}

void UZomPlayerAttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UZomPlayerAttributeSet, MaxStamina, OldMaxStamina);
}

