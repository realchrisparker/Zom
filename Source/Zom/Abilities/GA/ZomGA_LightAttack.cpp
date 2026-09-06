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
}

void UZomGA_LightAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
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
	NotifyAttackMontageEnded();
	EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, true, false);
}

void UZomGA_LightAttack::OnMontageInterruptedOrCancelled()
{
	NotifyAttackMontageEnded();
	EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, true, true);
}
