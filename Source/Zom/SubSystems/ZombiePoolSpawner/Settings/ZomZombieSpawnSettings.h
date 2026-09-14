// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ZomZombieSpawnSettings.generated.h"


class AZomZombieBase;
class UZombieTypeData;
class UZomDifficultyData;


/**
 * Project Settings > Game > Zom Zombie Spawning. The one designer-editable home for pooling and spawn director
 * configuration (Section 9 of the dev doc) - UZomZombiePoolSubsystem is a UWorldSubsystem and
 * UZomZombieSpawnDirector is created at runtime, so neither has class defaults that can be edited. The pool
 * subsystem copies these onto itself and its director at world begin play.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Zom Zombie Spawning"))
class ZOM_API UZomZombieSpawnSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UZomZombieSpawnSettings();

	// -------------
	// Pooling
	// -------------

	// Class pre-spawned for Crowd-category zombies (Walker/Runner/Auds/Eyes - type differs by the UZombieTypeData
	// assigned per activation, not by class).
	UPROPERTY(Config, EditAnywhere, Category = "Pooling")
	TSoftClassPtr<AZomZombieBase> CrowdZombieClass;

	UPROPERTY(Config, EditAnywhere, Category = "Pooling")
	TSoftClassPtr<AZomZombieBase> BloaterZombieClass;

	// Crowd pool sized to the 15-zombie ceiling plus headroom (design doc suggests 20).
	UPROPERTY(Config, EditAnywhere, Category = "Pooling", meta = (ClampMin = "0"))
	int32 CrowdPoolSize = 20;

	// Separate small Bloater pool capped at 2.
	UPROPERTY(Config, EditAnywhere, Category = "Pooling", meta = (ClampMin = "0"))
	int32 BloaterPoolSize = 2;

	// -------------
	// Director
	// -------------

	// Candidate types the director picks from for each Crowd activation.
	UPROPERTY(Config, EditAnywhere, Category = "Director")
	TArray<TSoftObjectPtr<UZombieTypeData>> CrowdTypes;

	UPROPERTY(Config, EditAnywhere, Category = "Director")
	TSoftObjectPtr<UZombieTypeData> BloaterType;

	// Difficulty tier active when a level starts (Section 10). Unset = crowd spawns are limited only by pool size.
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

	// Spawn points closer than this to any player are rejected, so zombies don't appear in the player's face.
	UPROPERTY(Config, EditAnywhere, Category = "Director", meta = (ClampMin = "0", Units = "cm"))
	float MinSpawnDistanceFromPlayer = 1500.f;

	// Random points tried per zombie before a volume gives up on that spawn until the next update.
	UPROPERTY(Config, EditAnywhere, Category = "Director", meta = (ClampMin = "1"))
	int32 SpawnLocationAttempts = 8;
};
