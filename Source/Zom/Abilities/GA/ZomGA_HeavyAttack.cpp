// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Abilities/GA/ZomGA_HeavyAttack.h"
#include "Zom/Abilities/Effects/ZomGE_StaminaDrain.h"


UZomGA_HeavyAttack::UZomGA_HeavyAttack()
{
	CostGameplayEffectClass = UZomGE_StaminaDrain::StaticClass();
}

void UZomGA_HeavyAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// TODO: play the equipped weapon's heavy-attack montage and let its AnimNotify drive hit-detection/UZomGE_Damage
	// application once Section 7's weapon system exists. Ends immediately for now so the ability shell is usable/testable.
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
