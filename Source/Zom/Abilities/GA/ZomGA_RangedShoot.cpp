// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Abilities/GA/ZomGA_RangedShoot.h"


UZomGA_RangedShoot::UZomGA_RangedShoot()
{
}

void UZomGA_RangedShoot::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// TODO: line-trace/fire the equipped weapon and consume ammo once Section 7's UZomInventoryComponent exists.
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
