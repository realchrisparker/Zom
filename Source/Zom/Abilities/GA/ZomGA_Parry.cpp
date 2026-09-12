// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Abilities/GA/ZomGA_Parry.h"
#include "Zom/Misc/ZomGameplayTags.h"
#include "Zom/Characters/Base/ZomCharacterBase.h"
#include "MotionCombatSystem/Structs/MCS_DefenseEntry.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"


UZomGA_Parry::UZomGA_Parry()
{
	// Asset tag so TryActivateAbilitiesByTag(TAG_Zom_Combat_Defense_Parry) can find and activate this ability.
	SetAssetTags(FGameplayTagContainer(TAG_Zom_Combat_Defense_Parry.GetTag()));

	// Persistent tag on the ASC for the duration of the parry attempt, per the class comment above.
	ActivationOwnedTags.AddTag(TAG_Zom_Status_Parrying.GetTag());

	// Set the tag for canceling other abilities.
	CancelAbilitiesWithTag.AddTag(TAG_Zom_Combat_Defense_Dodge.GetTag());
	CancelAbilitiesWithTag.AddTag(TAG_Zom_Combat_Attack_Heavy.GetTag());
	CancelAbilitiesWithTag.AddTag(TAG_Zom_Combat_Attack_Light.GetTag());
	CancelAbilitiesWithTag.AddTag(TAG_Zom_Combat_Defense_Parry.GetTag());
}

void UZomGA_Parry::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	CachedHandle = Handle; // Cache the ability spec handle for later use in montage callbacks.
	CachedActorInfo = ActorInfo; // Cache the actor info for later use in montage callbacks.
	CachedActivationInfo = ActivationInfo; // Cache the activation info for later use in montage callbacks.

	// Resolve the current defense entry. Whether this parry actually succeeds (negates the incoming hit) was
	// already decided synchronously by CombatDefense->TryParry() back in Input_Parry - this ability's only
	// job is playing the resolved parry montage, win or whiff, exactly like UZomGA_Dodge does for Defense.
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
			PlayMontageTask->OnCompleted.AddDynamic(this, &UZomGA_Parry::OnMontageCompleted);
			PlayMontageTask->OnInterrupted.AddDynamic(this, &UZomGA_Parry::OnMontageInterruptedOrCancelled);
			PlayMontageTask->OnCancelled.AddDynamic(this, &UZomGA_Parry::OnMontageInterruptedOrCancelled);

			// Activate the montage task to start playing the montage.
			PlayMontageTask->ReadyForActivation();
			return;
		}
	}

	// No valid montage was resolved, so end the ability immediately.
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UZomGA_Parry::OnMontageCompleted()
{
	EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, true, false);
}

void UZomGA_Parry::OnMontageInterruptedOrCancelled()
{
	EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, true, true);
}
