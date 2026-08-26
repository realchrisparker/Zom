// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Abilities/AttributeSets/ZomZombieAttributeSet.h"
#include "Net/UnrealNetwork.h"


UZomZombieAttributeSet::UZomZombieAttributeSet()
{
}

void UZomZombieAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UZomZombieAttributeSet, AttackDamage, COND_None, REPNOTIFY_Always);
}

void UZomZombieAttributeSet::OnRep_AttackDamage(const FGameplayAttributeData& OldAttackDamage)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UZomZombieAttributeSet, AttackDamage, OldAttackDamage);
}
