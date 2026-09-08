// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Abilities/GA/ZomGA_HeavyAttack.h"
#include "Zom/Misc/ZomGameplayTags.h"
#include "MotionCombatSystem/Structs/MCS_AttackEntry.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"


UZomGA_HeavyAttack::UZomGA_HeavyAttack()
{
	// Stamina cost is no longer paid through CostGameplayEffectClass + CommitAbility - see
	// ApplyStaminaCostForAttack's comment. It's applied manually in ActivateAbility from
	// GetCurrentAttackEntry().StaminaCost instead, since the cost varies per resolved attack, not per ability.
	SetAssetTags(FGameplayTagContainer(TAG_Zom_Combat_Attack_Heavy.GetTag()));

	// See UZomGA_LightAttack's constructor comment - same attack-lockup fix, mirrored so a combo hand-off in
	// either direction (Light->Heavy or Heavy->Light) properly cancels the ability being left behind instead
	// of orphaning it in a permanently-active state.
	CancelAbilitiesWithTag.AddTag(TAG_Zom_Combat_Attack_Light.GetTag());
}

void UZomGA_HeavyAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	CachedHandle = Handle;
	CachedActorInfo = ActorInfo;
	CachedActivationInfo = ActivationInfo;

	// Resolve the current attack entry.
	const FMCS_AttackEntry ResolvedAttack = GetCurrentAttackEntry();
	ApplyStaminaCostForAttack(ResolvedAttack);

	if (ResolvedAttack.HasValidMontage())
	{
		// Bind montage notifies for the resolved attack montage.
		if (UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, NAME_None, ResolvedAttack.AttackMontage, 1.f, ResolvedAttack.MontageSection, true))
		{
			// Play the resolved attack montage using an ability task.
			BindMontageNotifies(ResolvedAttack.AttackMontage);

			// Bind callbacks for montage completion, interruption, and cancellation.
			PlayMontageTask->OnCompleted.AddDynamic(this, &UZomGA_HeavyAttack::OnMontageCompleted);
			PlayMontageTask->OnInterrupted.AddDynamic(this, &UZomGA_HeavyAttack::OnMontageInterruptedOrCancelled);
			PlayMontageTask->OnCancelled.AddDynamic(this, &UZomGA_HeavyAttack::OnMontageInterruptedOrCancelled);

			// Activate the montage task to start playing the montage.
			PlayMontageTask->ReadyForActivation();
			return;
		}
	}

	// No montage resolved (no CombatCoreComponent, no current attack, or task creation failed) - end
	// immediately rather than leave the ability hung.
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UZomGA_HeavyAttack::OnMontageCompleted()
{
	NotifyAttackMontageEnded();
	EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, true, false);
}

void UZomGA_HeavyAttack::OnMontageInterruptedOrCancelled()
{
	NotifyAttackMontageEnded();
	EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, true, true);
}
