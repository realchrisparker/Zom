// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Zom/Abilities/GA/Base/ZomGameplayAbilityBase.h"
#include "ZomGA_Reload.generated.h"


/**
 * [Design] Reloads the equipped ranged weapon. Actual ammo-transfer logic depends on the
 * GameplayInventorySystem plugin (Section 7), not yet built - this class establishes the activation shape.
 */
UCLASS()
class ZOM_API UZomGA_Reload : public UZomGameplayAbilityBase
{
	GENERATED_BODY()

public:
	UZomGA_Reload();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
