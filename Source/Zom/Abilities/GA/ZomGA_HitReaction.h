// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Zom/Abilities/GA/Base/ZomGameplayAbilityBase.h"
#include "ZomGA_HitReaction.generated.h"


/**
 * Activated via AZomCharacterBase::HandleHitReactionResolved when the struck character's
 * UMCS_CombatHitReactionComponent resolves a reaction whose HitReactionTag is Zom.Combat.HitReaction (generic hits
 * from PerformHitReaction and scripted receiver reactions alike). Plays the resolved reaction montage, mirroring
 * UZomGA_Parry/UZomGA_Dodge's structure. Interrupts attacks and defenses, is allowed while staggered, and
 * retriggers on each new hit so back-to-back hits restart the reaction.
 */
UCLASS()
class ZOM_API UZomGA_HitReaction : public UZomGameplayAbilityBase
{
	GENERATED_BODY()

public:
	UZomGA_HitReaction();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageInterruptedOrCancelled();

private:
	FGameplayAbilitySpecHandle CachedHandle;
	const FGameplayAbilityActorInfo* CachedActorInfo = nullptr;
	FGameplayAbilityActivationInfo CachedActivationInfo;
};
