// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Abilities/ZomGameplayAbility.h"
#include "Zom/Misc/ZomGameplayTags.h"


UZomGameplayAbility::UZomGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	ActivationBlockedTags.AddTag(TAG_Zom_Status_Staggered.GetTag());
}
