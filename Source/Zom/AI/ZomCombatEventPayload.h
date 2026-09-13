// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZomCombatEventPayload.generated.h"


/**
 * Payload carried by the Zom.Combat.Event.* tagged State Tree events AZomZombieAIController raises when
 * relaying UMCS_CombatEventBus signals into its own State Tree. Wrapped via FConstStructView::Make() when
 * calling SendStateTreeEvent. Not every field is meaningful for every event - see each SendStateTreeEvent
 * call site for which fields it actually populates (e.g. Duration is only set for DefenseWindowOpened).
 */
USTRUCT(BlueprintType)
struct FZomCombatEventPayload
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Zom|Combat")
	TWeakObjectPtr<AActor> Defender;

	UPROPERTY(BlueprintReadOnly, Category = "Zom|Combat")
	TWeakObjectPtr<AActor> Attacker;

	UPROPERTY(BlueprintReadOnly, Category = "Zom|Combat")
	float Duration = 0.f;
};
