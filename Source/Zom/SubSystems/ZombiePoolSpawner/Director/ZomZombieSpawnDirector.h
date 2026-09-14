// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Zom/Characters/Enums/ZomCharacterEnums.h"
#include "ZomZombieSpawnDirector.generated.h"


class AZomZombieBase;
class AZomZombieSpawnVolume;
class UZombieTypeData;
class UZomZombiePoolSubsystem;
class UZomDifficultyData;
class UZomZombieSpawnSettings;


/**
 * Plain UObject, owned by UZomZombiePoolSubsystem. Decides which pooled zombie to activate, where, and of which
 * type (Section 9 of the dev doc). Level-placed AZomZombieSpawnVolumes register here. On a self-rescheduling timer
 * (the active tier's CrowdSpawnInterval) it populates volumes near players - nearest first, so a capped budget goes
 * where the player is - and returns zombies from volumes every player has left back to the pool, so the budget
 * follows the player through an open world.
 */
UCLASS()
class ZOM_API UZomZombieSpawnDirector : public UObject
{
	GENERATED_BODY()

public:
	// Candidate types to pick from when spawning a Crowd zombie (Walker/Runner/Auds/Eyes - no C++ subclass
	// per type, per Section 5.2). Loaded from UZomZombieSpawnSettings when the pool subsystem starts.
	UPROPERTY(BlueprintReadOnly, Category = "Zom|Spawning")
	TArray<TObjectPtr<UZombieTypeData>> CrowdTypes;

	UPROPERTY(BlueprintReadOnly, Category = "Zom|Spawning")
	TObjectPtr<UZombieTypeData> BloaterType;

	// Active difficulty tier (Section 10). A new tier is a new UZomDifficultyData asset assigned here, not new code.
	// Starts as UZomZombieSpawnSettings::DefaultDifficultyTier; swapping it at runtime applies from the next update.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zom|Spawning")
	TObjectPtr<UZomDifficultyData> ActiveDifficultyTier;

	// Loads types/tier and copies radii from Settings. Called by the owning pool subsystem.
	void ApplySettings(const UZomZombieSpawnSettings& Settings);

	// Listens for zombies returning to the pool and runs the first update. Called by the owning pool subsystem
	// once the pool is prewarmed.
	void StartDirector();

	void StopDirector();

	// Called by AZomZombieSpawnVolume::BeginPlay. Safe to call more than once.
	UFUNCTION(BlueprintCallable, Category = "Zom|Spawning")
	void RegisterSpawnVolume(AZomZombieSpawnVolume* Volume);

	// Called by AZomZombieSpawnVolume::EndPlay. bReleaseZombies returns the volume's active zombies to the pool.
	UFUNCTION(BlueprintCallable, Category = "Zom|Spawning")
	void UnregisterSpawnVolume(AZomZombieSpawnVolume* Volume, bool bReleaseZombies = true);

	// Starts a population cycle on Volume (if it's allowed one) and fills it now instead of on the next update.
	// Used by PlayerEnter volumes.
	UFUNCTION(BlueprintCallable, Category = "Zom|Spawning")
	void PopulateVolume(AZomZombieSpawnVolume* Volume);

	// Requests one Crowd-category activation at SpawnTransform, picking a random type from CrowdTypes. If
	// ActiveDifficultyTier is set, refuses once PoolSubsystem->GetActiveCrowdCount() already meets its
	// TargetActiveCrowdCount (unmetered if no tier is assigned).
	UFUNCTION(BlueprintCallable, Category = "Zom|Spawning")
	AZomZombieBase* RequestCrowdSpawn(UZomZombiePoolSubsystem* PoolSubsystem, const FTransform& SpawnTransform) const;

	// Requests one Bloater-category activation at SpawnTransform.
	UFUNCTION(BlueprintCallable, Category = "Zom|Spawning")
	AZomZombieBase* RequestBloaterSpawn(UZomZombiePoolSubsystem* PoolSubsystem, const FTransform& SpawnTransform) const;

private:
	void UpdateDirector();
	void ScheduleNextUpdate();

	// Spawns Volume's pending zombies. Returns false once the pool/tier budget is exhausted, so the caller can stop.
	bool FillVolume(UZomZombiePoolSubsystem& PoolSubsystem, AZomZombieSpawnVolume& Volume, const TArray<FVector>& PlayerLocations);

	// Returns Volume's zombies that are beyond DespawnRadius from every player to the pool.
	void DespawnVolume(UZomZombiePoolSubsystem& PoolSubsystem, AZomZombieSpawnVolume& Volume, const TArray<FVector>& PlayerLocations);

	// Bound to UZomZombiePoolSubsystem::OnZombieReleased - tells the owning volume its zombie was killed.
	void HandleZombieReleased(AZomZombieBase* Zombie);

	void GatherPlayerLocations(TArray<FVector>& OutLocations) const;

	UZomZombiePoolSubsystem* GetPoolSubsystem() const;

	UPROPERTY()
	TArray<TWeakObjectPtr<AZomZombieSpawnVolume>> SpawnVolumes;

	// Copied from UZomZombieSpawnSettings in ApplySettings.
	float FallbackSpawnInterval = 5.f;
	float ActivationRadius = 5000.f;
	float DespawnRadius = 7000.f;
	float MinSpawnDistanceFromPlayer = 1500.f;
	int32 SpawnLocationAttempts = 8;

	FTimerHandle UpdateTimer;
	FDelegateHandle ZombieReleasedHandle;
};
