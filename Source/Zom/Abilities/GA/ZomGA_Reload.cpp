// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Abilities/GA/ZomGA_Reload.h"


UZomGA_Reload::UZomGA_Reload()
{
}

void UZomGA_Reload::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// TODO: transfer ammo from reserve to the equipped weapon's magazine once Section 7's UZomInventoryComponent exists.
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
