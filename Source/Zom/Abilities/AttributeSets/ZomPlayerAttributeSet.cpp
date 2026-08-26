// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Abilities/AttributeSets/ZomPlayerAttributeSet.h"
#include "Net/UnrealNetwork.h"


UZomPlayerAttributeSet::UZomPlayerAttributeSet()
{
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
