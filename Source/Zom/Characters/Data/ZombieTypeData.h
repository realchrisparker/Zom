// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Zom/Characters/Enums/ZomCharacterEnums.h"
#include "ZombieTypeData.generated.h"


class AZomZombieBase;
class UZomCharacterSoundSet;


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
	// Which budget this type counts against: Crowd types count toward the difficulty tier's TargetActiveCrowdCount;
	// Bloaters are capped by their own PoolSize (and spawn toxic gas on death, AZomZombieBase::HandleDeath). Boss
	// doesn't use UZombieTypeData at all (Section 5.1/6).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom", meta = (DisplayName = "Category"))
	EZomZombieCategory Category = EZomZombieCategory::Crowd;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zom", meta = (DisplayName = "Type"))
	EZombieType ZombieType = EZombieType::Walker;

	// -------------
	// Pooling
	// -------------

	// Blueprint pooled and spawned for this type (e.g. BP_Tank). Soft so this asset and the Blueprint - which usually
	// defaults to this asset - don't hard-reference each other.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom|Pooling", meta = (DisplayName = "Zombie Class"))
	TSoftClassPtr<AZomZombieBase> ZombieClass;

	// Instances of ZombieClass pre-spawned once a level uses this type - also the most that can be active at once.
	// Types sharing a class share one pool, sized to the largest PoolSize among them.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom|Pooling", meta = (DisplayName = "Pool Size", ClampMin = "0"))
	int32 PoolSize = 10;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom", meta = (DisplayName = "Health"))
	float Health = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom", meta = (DisplayName = "Speed"))
	float Speed = 200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom", meta = (DisplayName = "Attack Damage"))
	float AttackDamage = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom", meta = (DisplayName = "Attack Cooldown"))
	float AttackCooldown = 1.5f;

	// -------------
	// Perception
	// -------------

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom|Perception", meta = (DisplayName = "Sight Radius"))
	float SightRadius = 800.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom|Perception", meta = (DisplayName = "Sight Angle Degrees"))
	float SightAngleDegrees = 70.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom|Perception", meta = (DisplayName = "Hearing Radius"))
	float HearingRadius = 600.f;

	// -------------
	// Audio
	// -------------

	// Vocals (idle growl, hit, death) for this type, applied to UZomCharacterAudioComponent on every activation. Types
	// can share one asset. Null keeps the Blueprint's DefaultSoundSet.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom|Audio", meta = (DisplayName = "Sound Set"))
	TObjectPtr<UZomCharacterSoundSet> SoundSet;
};
