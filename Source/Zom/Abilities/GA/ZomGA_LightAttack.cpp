// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Abilities/GA/ZomGA_LightAttack.h"
#include "Zom/Misc/ZomGameplayTags.h"
#include "Zom/Characters/Base/ZomCharacterBase.h"
#include "MotionCombatSystem/Structs/MCS_AttackEntry.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"


UZomGA_LightAttack::UZomGA_LightAttack()
{
	// Set the asset tags for this ability.
	SetAssetTags(FGameplayTagContainer(TAG_Zom_Combat_Attack_Light.GetTag()));

	// Fixes an attack-lockup: MCS's combo system can hand off from this attack to a Heavy attack (or vice
	// versa) via a DataTable row's AllowedNextAttacks, resolving a different AttackTag while THIS ability is
	// still active waiting on its own PlayMontageAndWait task. Since that task's montage gets silently
	// replaced on the AnimInstance rather than going through GAS cancellation, its OnCompleted/OnInterrupted
	// never fires and the spec is stuck IsActive()==true forever, permanently blocking future activation of
	// whichever attack got orphaned. CancelAbilitiesWithTag is GAS's built-in fix: PreActivate cancels any
	// matching active ability (properly calling its EndAbility) before this one runs.
	CancelAbilitiesWithTag.AddTag(TAG_Zom_Combat_Attack_Heavy.GetTag());
}

void UZomGA_LightAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// TEMP DIAGNOSTIC (attack-lockup investigation): remove once the lockup is diagnosed.
	UE_LOG(LogTemp, Warning, TEXT("[AttackDiag] LightAttack::ActivateAbility Handle=%s"), *Handle.ToString());

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	CachedHandle = Handle; // Cache the ability spec handle for later use in montage callbacks.
	CachedActorInfo = ActorInfo; // Cache the actor info for later use in montage callbacks.
	CachedActivationInfo = ActivationInfo; // Cache the activation info for later use in montage callbacks.

	// Resolve the current attack entry.
	const FMCS_AttackEntry ResolvedAttack = GetCurrentAttackEntry();
	ApplyStaminaCostForAttack(ResolvedAttack);

	if (ResolvedAttack.HasValidMontage())
	{
		// Play the resolved attack montage using an ability task.
		if (UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, NAME_None, ResolvedAttack.AttackMontage, 1.f, ResolvedAttack.MontageSection, true))
		{
			// Bind montage notifies for the resolved attack montage.
			BindMontageNotifies(ResolvedAttack.AttackMontage);

			// Bind callbacks for montage completion, interruption, and cancellation.
			PlayMontageTask->OnCompleted.AddDynamic(this, &UZomGA_LightAttack::OnMontageCompleted);
			PlayMontageTask->OnInterrupted.AddDynamic(this, &UZomGA_LightAttack::OnMontageInterruptedOrCancelled);
			PlayMontageTask->OnCancelled.AddDynamic(this, &UZomGA_LightAttack::OnMontageInterruptedOrCancelled);

			// Activate the montage task to start playing the montage.
			PlayMontageTask->ReadyForActivation();
			return;
		}
	}

	// No valid montage was resolved, so end the ability immediately.
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UZomGA_LightAttack::OnMontageCompleted()
{
	// TEMP DIAGNOSTIC (attack-lockup investigation): CachedHandle is a plain member, not per-activation - if
	// two activations ever overlap on this InstancedPerActor instance, this logs which handle the SECOND
	// activation clobbered it with, ending the wrong one. Remove once the lockup is diagnosed.
	UE_LOG(LogTemp, Warning, TEXT("[AttackDiag] LightAttack::OnMontageCompleted using CachedHandle=%s"), *CachedHandle.ToString());

	NotifyAttackMontageEnded();
	EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, true, false);
}

void UZomGA_LightAttack::OnMontageInterruptedOrCancelled()
{
	UE_LOG(LogTemp, Warning, TEXT("[AttackDiag] LightAttack::OnMontageInterruptedOrCancelled using CachedHandle=%s"), *CachedHandle.ToString());

	NotifyAttackMontageEnded();
	EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, true, true);
}
