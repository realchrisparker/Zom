// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Abilities/GA/ZomGA_HitReaction.h"
#include "Zom/Misc/ZomGameplayTags.h"
#include "MotionCombatSystem/Structs/MCS_HitReaction.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"


UZomGA_HitReaction::UZomGA_HitReaction()
{
	// Asset tag so TryActivateAbilitiesByTag(TAG_Zom_Combat_HitReaction) can find and activate this ability.
	SetAssetTags(FGameplayTagContainer(TAG_Zom_Combat_HitReaction.GetTag()));

	// Persistent tag on the ASC for the duration of the reaction.
	ActivationOwnedTags.AddTag(TAG_Zom_Status_HitReacting.GetTag());

	// The base blocks every ability while staggered, but a hit landing mid-stagger still has to play a reaction.
	ActivationBlockedTags.RemoveTag(TAG_Zom_Status_Staggered.GetTag());

	// Getting hit interrupts any attack or defense in progress.
	CancelAbilitiesWithTag.AddTag(TAG_Zom_Combat_Attack_Light.GetTag());
	CancelAbilitiesWithTag.AddTag(TAG_Zom_Combat_Attack_Heavy.GetTag());
	CancelAbilitiesWithTag.AddTag(TAG_Zom_Combat_Defense_Dodge.GetTag());
	CancelAbilitiesWithTag.AddTag(TAG_Zom_Combat_Defense_Parry.GetTag());

	// Without this, InstancedPerActor activation silently fails while a reaction is already playing, so a second
	// hit would get no reaction at all. Retriggering ends the current reaction and plays the newly resolved one.
	bRetriggerInstancedAbility = true;
}

void UZomGA_HitReaction::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	CachedHandle = Handle; // Cache the ability spec handle for later use in montage callbacks.
	CachedActorInfo = ActorInfo; // Cache the actor info for later use in montage callbacks.
	CachedActivationInfo = ActivationInfo; // Cache the activation info for later use in montage callbacks.

	// The hit reaction component resolved this row and broadcast OnHitReactionResolved right before activating us.
	const FMCS_HitReaction ResolvedReaction = GetCurrentHitReactionEntry();

	if (ResolvedReaction.Montage)
	{
		if (UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, NAME_None, ResolvedReaction.Montage, ResolvedReaction.PlayRate, NAME_None, true))
		{
			PlayMontageTask->OnCompleted.AddDynamic(this, &UZomGA_HitReaction::OnMontageCompleted);
			PlayMontageTask->OnInterrupted.AddDynamic(this, &UZomGA_HitReaction::OnMontageInterruptedOrCancelled);
			PlayMontageTask->OnCancelled.AddDynamic(this, &UZomGA_HitReaction::OnMontageInterruptedOrCancelled);

			// Plays synchronously, which the component relies on to arm its death/explicit-reaction locks.
			PlayMontageTask->ReadyForActivation();
			return;
		}
	}

	// No valid montage was resolved, so end the ability immediately.
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UZomGA_HitReaction::OnMontageCompleted()
{
	EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, true, false);
}

void UZomGA_HitReaction::OnMontageInterruptedOrCancelled()
{
	EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, true, true);
}
