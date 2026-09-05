// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "ZomGameplayAbility.generated.h"


class AZomCharacterBase;
struct FMCS_AttackEntry;


/**
 * Shared ability base for all six Zom abilities. Sets InstancingPolicy/NetExecutionPolicy once,
 * consistent with the full-GAS decision to teach production patterns even though prediction goes unused in a
 * singleplayer build, and blocks activation while staggered so no individual ability class has to remember to
 * (Section 4.2 of the dev doc).
 */
UCLASS()
class ZOM_API UZomGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UZomGameplayAbility();

protected:
	// Returns the avatar actor executing this ability, cast to AZomCharacterBase - every Zom ability's avatar
	// is one, so this replaces a repeated Cast<AZomCharacterBase>(GetAvatarActorFromActorInfo()) in every
	// ActivateAbility override that needs it.
	AZomCharacterBase* GetOwningCharacter() const;

	// Returns the attack entry the owning character's MCS CombatCoreComponent most recently resolved (a
	// default-constructed FMCS_AttackEntry if there's no owning character or no CombatCoreComponent). Attack
	// abilities (LightAttack, HeavyAttack, future weapon-specific variants) read AttackMontage/MontageSection
	// off this - see UMCS_CombatCoreComponent::GetCurrentAttack()'s own doc comment on this exact usage.
	FMCS_AttackEntry GetCurrentAttackEntry() const;

	// Call once this ability's own montage task ends (completed, blended out, interrupted, or cancelled), if
	// it played a montage sourced from GetCurrentAttackEntry(). UMCS_CombatCoreComponent's own redundant
	// "belt-and-suspenders" end signal (bound via Montage_SetEndDelegate in PlayCurrentAttack, guarding against
	// the AttackEnd AnimNotify sitting in the blend-out tail and getting skipped) only fires on its own
	// Montage_Play call - it returns immediately once it hands off to GAS, so that safety net never runs for a
	// GAS-driven attack unless the ability replicates it here. OnAttackEnd is a public BlueprintAssignable
	// delegate, so broadcasting it from outside the component is legal.
	void NotifyAttackMontageEnded() const;
};
