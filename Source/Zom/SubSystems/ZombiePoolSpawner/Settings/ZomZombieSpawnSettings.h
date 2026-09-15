// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ZomZombieSpawnSettings.generated.h"


class UZomDifficultyData;


/**
 * Project Settings > Game > Zom Zombie Spawning. Level-wide spawn director configuration (Section 9 of the dev doc) -
 * UZomZombieSpawnDirector is created at runtime, so it has no class defaults that can be edited. Zombie types are not
 * here: they live on each spawn/defend volume, and each type's Blueprint class and pool size live on its
 * UZombieTypeData. The pool subsystem copies these onto its director at world begin play.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Zom Zombie Spawning"))
class ZOM_API UZomZombieSpawnSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UZomZombieSpawnSettings();

	// Difficulty tier active when a level starts (Section 10). Unset = crowd spawns are limited only by pool sizes.
	UPROPERTY(Config, EditAnywhere, Category = "Director")
	TSoftObjectPtr<UZomDifficultyData> DefaultDifficultyTier;

	// Seconds between director updates when no difficulty tier is assigned (the tier's CrowdSpawnInterval otherwise).
	UPROPERTY(Config, EditAnywhere, Category = "Director", meta = (ClampMin = "0.1", Units = "s"))
	float FallbackSpawnInterval = 5.f;

	// A spawn volume starts populating once any player is within this distance of its box.
	UPROPERTY(Config, EditAnywhere, Category = "Director", meta = (ClampMin = "0", Units = "cm"))
	float ActivationRadius = 5000.f;

	// Once every player is farther than this from a volume, its zombies that are also this far from every player go
	// back to the pool, so the budget follows the player. Treated as at least ActivationRadius, so volumes near the
	// edge don't flicker between populating and despawning.
	UPROPERTY(Config, EditAnywhere, Category = "Director", meta = (ClampMin = "0", Units = "cm"))
	float DespawnRadius = 7000.f;

	// Random points tried per zombie before a volume gives up on that spawn until the next update.
	UPROPERTY(Config, EditAnywhere, Category = "Director", meta = (ClampMin = "1"))
	int32 SpawnLocationAttempts = 8;
};
