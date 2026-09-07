// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Abilities/GA/ZomGA_Dodge.h"
#include "Zom/Misc/ZomGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"


UZomGA_Dodge::UZomGA_Dodge()
{
	// TODO: Dodge's own Stamina cost isn't set up yet. It previously pointed CostGameplayEffectClass at
	// UZomGE_StaminaDrain, but that effect's modifier is now SetByCaller (see UZomGameplayAbilityBase::
	// ApplyStaminaCostForAttack) - Dodge doesn't resolve an FMCS_AttackEntry to source a cost from, since it
	// isn't an MCS-driven attack, so CommitAbility would never set that SetByCaller value and would just warn.
	// Give Dodge its own cost mechanism (e.g. a fixed EditDefaultsOnly float) when this is prioritized.
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
