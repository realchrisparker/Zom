// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Zom/AI/Enums/ZomAIEnums.h"
#include "ZombieTypeData.generated.h"


/**
 * Health, speed, per-sense detection radius, damage, attack cooldown - one asset per zombie type
 * (Walker/Runner/Auds/Eyes/Bloater). New crowd types are content, not code (Section 5.1 of the dev doc).
 * Auds/Eyes are the same shape as Walker/Runner with one sense's radius left at (or near) zero.
 *
 * Health/Speed/AttackDamage are *initial* values only, read once at BeginPlay to seed the zombie's GAS
 * attributes (Section 4.1) - after that the live attribute is authoritative, this asset is never re-read.
 */
UCLASS(BlueprintType)
class ZOM_API UZombieTypeData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// Which pool budget this type draws from (UZomZombiePoolSubsystem) and, on death, returns to
	// (AZomZombieBase::HandleDeath). Boss doesn't use UZombieTypeData at all (Section 5.1/6).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom|ZombieType")
	EZomZombieCategory Category = EZomZombieCategory::Crowd;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom|ZombieType")
	float Health = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom|ZombieType")
	float Speed = 200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom|ZombieType")
	float AttackDamage = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom|ZombieType")
	float AttackCooldown = 1.5f;

	// -------------
	// Perception
	// -------------

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom|ZombieType|Sight")
	float SightRadius = 800.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom|ZombieType|Sight")
	float SightAngleDegrees = 70.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom|ZombieType|Hearing")
	float HearingRadius = 600.f;
};
