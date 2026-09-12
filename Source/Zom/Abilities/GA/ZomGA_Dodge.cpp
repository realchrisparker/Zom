// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Abilities/GA/ZomGA_Dodge.h"
#include "Zom/Misc/ZomGameplayTags.h"
#include "Zom/Characters/Base/ZomCharacterBase.h"
#include "MotionCombatSystem/Structs/MCS_DefenseEntry.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"


UZomGA_Dodge::UZomGA_Dodge()
{
	// Asset tag so TryActivateAbilitiesByTag(TAG_Zom_Combat_Defense_Dodge) can find and activate this ability.
	SetAssetTags(FGameplayTagContainer(TAG_Zom_Combat_Defense_Dodge.GetTag()));

	// Persistent tag on the ASC for the duration of the dodge, per the class comment above.
	ActivationOwnedTags.AddTag(TAG_Zom_Status_Dodging.GetTag());

	// Set the tag for canceling other abilities.
	CancelAbilitiesWithTag.AddTag(TAG_Zom_Combat_Defense_Parry.GetTag());
	CancelAbilitiesWithTag.AddTag(TAG_Zom_Combat_Attack_Heavy.GetTag());
	CancelAbilitiesWithTag.AddTag(TAG_Zom_Combat_Attack_Light.GetTag());
	CancelAbilitiesWithTag.AddTag(TAG_Zom_Combat_Defense_Dodge.GetTag());
}

void UZomGA_Dodge::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
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
	const FMCS_DefenseEntry ResolvedDefense = GetCurrentDefenseEntry();
	ApplyStaminaCostForDefense(ResolvedDefense);

	if (ResolvedDefense.HasValidMontage())
	{
		// Play the resolved attack montage using an ability task.
		if (UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, NAME_None, ResolvedDefense.DefenseMontage, 1.f, ResolvedDefense.MontageSection, true))
		{
			// Bind montage notifies for the resolved attack montage.
			BindMontageNotifies(ResolvedDefense.DefenseMontage);

			// Bind callbacks for montage completion, interruption, and cancellation.
			PlayMontageTask->OnCompleted.AddDynamic(this, &UZomGA_Dodge::OnMontageCompleted);
			PlayMontageTask->OnInterrupted.AddDynamic(this, &UZomGA_Dodge::OnMontageInterruptedOrCancelled);
			PlayMontageTask->OnCancelled.AddDynamic(this, &UZomGA_Dodge::OnMontageInterruptedOrCancelled);

			// Activate the montage task to start playing the montage.
			PlayMontageTask->ReadyForActivation();
			return;
		}
	}

	// No valid montage was resolved, so end the ability immediately.
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UZomGA_Dodge::OnMontageCompleted()
{
	EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, true, false);
}

void UZomGA_Dodge::OnMontageInterruptedOrCancelled()
{
	EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, true, true);
}