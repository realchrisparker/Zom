// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Zom/Abilities/GA/Base/ZomGameplayAbilityBase.h"
#include "ZomGA_Parry.generated.h"


/**
 * Activated via HandleDefenseResolved when the Defense Chooser resolves a Parry-intent entry
 * (see AZomPlayerController::Input_Parry, which requests EMCS_DefenseIntent::Parry and separately
 * calls CombatDefense->TryParry() to resolve success/fail against the timing window - this ability
 * only owns playing the resolved parry montage, mirroring UZomGA_Dodge's structure).
 */
UCLASS()
class ZOM_API UZomGA_Parry : public UZomGameplayAbilityBase
{
	GENERATED_BODY()

public:
	UZomGA_Parry();

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
