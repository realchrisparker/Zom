// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Abilities/GA/ZomGA_Dodge.h"
#include "Zom/Abilities/Effects/ZomGE_StaminaDrain.h"
#include "Zom/Misc/ZomGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"


UZomGA_Dodge::UZomGA_Dodge()
{
	CostGameplayEffectClass = UZomGE_StaminaDrain::StaticClass();
	ActivationOwnedTags.AddTag(TAG_Zom_Status_Dodging.GetTag());
}

void UZomGA_Dodge::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	CachedHandle = Handle;
	CachedActorInfo = ActorInfo;
	CachedActivationInfo = ActivationInfo;

	// TODO: play a dodge/roll montage once one is authored; for now Zom.Status.Dodging stays active for
	// DodgeDuration via this timer, giving abilities/animation something concrete to gate/query against.
	if (UAbilityTask_WaitDelay* WaitDelayTask = UAbilityTask_WaitDelay::WaitDelay(this, DodgeDuration))
	{
		WaitDelayTask->OnFinish.AddDynamic(this, &UZomGA_Dodge::OnDodgeFinished);
		WaitDelayTask->ReadyForActivation();
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UZomGA_Dodge::OnDodgeFinished()
{
	EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, true, false);
}
