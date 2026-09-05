// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Zom/Abilities/ZomGameplayAbility.h"
#include "ZomGA_LightAttack.generated.h"


/**
 * Melee light attack (cooldown only, no Stamina cost per the dev doc). Commits, then plays whatever montage
 * the MCS chooser most recently resolved (GetCurrentAttackEntry()) via UAbilityTask_PlayMontageAndWait - the
 * real hit-detection/damage-application flow is driven by an AnimNotify on that montage (content, not yet
 * authored), which is expected to call into UZomInventoryComponent/UZomGE_Damage once Section 7's weapon
 * system exists. Carries AssetTags = Zom.Combat.Attack.Light so AZomPlayerController::HandleAttackResolved's
 * TryActivateAbilitiesByTag dispatch finds it for any light-attack DataTable row (see ZomGameplayTags.h).
 */
UCLASS()
class ZOM_API UZomGA_LightAttack : public UZomGameplayAbility
{
	GENERATED_BODY()

public:
	UZomGA_LightAttack();

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
