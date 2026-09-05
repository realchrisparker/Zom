// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Abilities/GA/ZomGA_LightAttack.h"
#include "Zom/Misc/ZomGameplayTags.h"
#include "MotionCombatSystem/Structs/MCS_AttackEntry.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"


UZomGA_LightAttack::UZomGA_LightAttack()
{
	SetAssetTags(FGameplayTagContainer(TAG_Zom_Combat_Attack_Light.GetTag()));
}

void UZomGA_LightAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	CachedHandle = Handle;
	CachedActorInfo = ActorInfo;
	CachedActivationInfo = ActivationInfo;

	// TODO: an AnimNotify on the resolved montage should drive hit-detection/UZomGE_Damage application once
	// Section 7's weapon system exists.
	const FMCS_AttackEntry ResolvedAttack = GetCurrentAttackEntry();
	if (ResolvedAttack.HasValidMontage())
	{
		if (UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, NAME_None, ResolvedAttack.AttackMontage, 1.f, ResolvedAttack.MontageSection, true))
		{
			PlayMontageTask->OnCompleted.AddDynamic(this, &UZomGA_LightAttack::OnMontageCompleted);
			PlayMontageTask->OnInterrupted.AddDynamic(this, &UZomGA_LightAttack::OnMontageInterruptedOrCancelled);
			PlayMontageTask->OnCancelled.AddDynamic(this, &UZomGA_LightAttack::OnMontageInterruptedOrCancelled);
			PlayMontageTask->ReadyForActivation();
			return;
		}
	}

	// No montage resolved (no CombatCoreComponent, no current attack, or task creation failed) - end
	// immediately rather than leave the ability hung.
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UZomGA_LightAttack::OnMontageCompleted()
{
	NotifyAttackMontageEnded();
	EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, true, false);
}

void UZomGA_LightAttack::OnMontageInterruptedOrCancelled()
{
	NotifyAttackMontageEnded();
	EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, true, true);
}
