// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ZomAbilitySetData.generated.h"


class UZomGameplayAbility;
class UZomGameplayEffect;


/**
 * [Design] Grants an ability/effect set to a character on BeginPlay - the extensibility hook for combat
 * (Section 4.4 of the dev doc). Consumed by AZomPlayerCharacterBase::GrantAbilitySet().
 */
UCLASS(BlueprintType)
class ZOM_API UZomAbilitySetData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// Abilities granted to the character's ASC.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom|AbilitySet")
	TArray<TSubclassOf<UZomGameplayAbility>> GrantedAbilities;

	// Effects applied to the character's ASC once, at grant time (e.g. initial buffs, not per-activation costs).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom|AbilitySet")
	TArray<TSubclassOf<UZomGameplayEffect>> StartingEffects;
};
