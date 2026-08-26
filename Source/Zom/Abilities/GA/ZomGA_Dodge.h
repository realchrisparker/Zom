// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Zom/Abilities/ZomGameplayAbility.h"
#include "ZomGA_Dodge.generated.h"


/**
 * [Design] Gated by Stamina, applies Zom.Status.Dodging for the dodge's duration. The tag add/remove is
 * automatic (ActivationOwnedTags, per GAS's own ability-activation lifecycle) - no manual AddLooseGameplayTag
 * needed. Timed via UAbilityTask_WaitDelay rather than a montage since no dodge/roll animation exists yet;
 * swap this for an AbilityTask_PlayMontageAndWait once one is authored.
 */
UCLASS()
class ZOM_API UZomGA_Dodge : public UZomGameplayAbility
{
	GENERATED_BODY()

public:
	UZomGA_Dodge();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	// How long the dodge (and its Zom.Status.Dodging tag) stays active.
	UPROPERTY(EditDefaultsOnly, Category = "Zom|Dodge")
	float DodgeDuration = 0.5f;

	UFUNCTION()
	void OnDodgeFinished();

private:
	FGameplayAbilitySpecHandle CachedHandle;
	const FGameplayAbilityActorInfo* CachedActorInfo = nullptr;
	FGameplayAbilityActivationInfo CachedActivationInfo;
};
