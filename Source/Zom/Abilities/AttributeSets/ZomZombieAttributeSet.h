// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Zom/Abilities/AttributeSets/ZomAttributeSetBase.h"
#include "ZomZombieAttributeSet.generated.h"


/**
 * Zombie/Boss-only attributes. [Proposed, per Section 4.1 of the dev doc] AttackDamage as a real GAS attribute
 * lets UZomDifficultyData scale zombie damage at runtime via an Infinite GameplayEffect on spawn, rather than
 * needing a second data-asset override path alongside UZombieTypeData's static damage value.
 */
UCLASS()
class ZOM_API UZomZombieAttributeSet : public UZomAttributeSetBase
{
	GENERATED_BODY()

public:
	UZomZombieAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Damage dealt per attack. Not clamped against a Max counterpart; UZombieTypeData seeds the initial value at
	// BeginPlay, GameplayEffects (e.g. difficulty scaling) modify it afterward.
	UPROPERTY(BlueprintReadOnly, Category = "Zom|Attributes", ReplicatedUsing = OnRep_AttackDamage)
	FGameplayAttributeData AttackDamage;
	ATTRIBUTE_ACCESSORS(UZomZombieAttributeSet, AttackDamage)

protected:
	UFUNCTION()
	virtual void OnRep_AttackDamage(const FGameplayAttributeData& OldAttackDamage);
};
