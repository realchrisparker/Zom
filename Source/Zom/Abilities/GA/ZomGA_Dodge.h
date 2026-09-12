// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Zom/Abilities/GA/Base/ZomGameplayAbilityBase.h"
#include "ZomGA_Dodge.generated.h"


/**
 * Gated by Stamina, applies Zom.Status.Dodging for the dodge's duration. The tag add/remove is
 * automatic (ActivationOwnedTags, per GAS's own ability-activation lifecycle) - no manual AddLooseGameplayTag
 * needed. Timed via UAbilityTask_WaitDelay rather than a montage since no dodge/roll animation exists yet;
 * swap this for an AbilityTask_PlayMontageAndWait once one is authored.
 */
UCLASS()
class ZOM_API UZomGA_Dodge : public UZomGameplayAbilityBase
{
	GENERATED_BODY()

public:
	UZomGA_Dodge();

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
