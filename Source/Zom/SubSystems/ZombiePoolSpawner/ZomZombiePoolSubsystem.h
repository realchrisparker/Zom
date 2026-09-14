// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Zom/Characters/Enums/ZomCharacterEnums.h"
#include "ZomZombiePoolSubsystem.generated.h"


class AZomZombieBase;
class UZombieTypeData;
class UZomZombieSpawnDirector;


DECLARE_MULTICAST_DELEGATE_OneParam(FZomOnZombieReleased, AZomZombieBase* /*Zombie*/);


/**
 * [Proposed type] UWorldSubsystem, not UGameInstanceSubsystem - it owns actual pooled actors that live in one
 * world/level, unlike UZomObjectiveSubsystem/UZomSaveGame's state, which must survive a level reload (Section
 * 9 of the dev doc). Pre-spawns and recycles pooled zombie actors via SetActorHiddenInGame/SetActorEnableCollision,
 * never SpawnActor()/Destroy() at runtime - PrewarmPool() is initial setup, not a runtime activation/deactivation.
 *
 * Configured from UZomZombieSpawnSettings (Project Settings > Game > Zom Zombie Spawning) at world begin play, then
 * prewarms itself and starts SpawnDirector a tick later - nothing else needs to call PrewarmPool().
 */
UCLASS()
class ZOM_API UZomZombiePoolSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	// Owned by this subsystem (Section 9 of the dev doc), created in Initialize().
	UPROPERTY(BlueprintReadOnly, Category = "Zom|Pooling")
	TObjectPtr<UZomZombieSpawnDirector> SpawnDirector;

	// Class to spawn for Crowd-category zombies (Walker/Runner/Auds/Eyes - type differs by UZombieTypeData
	// assigned per-activation via AcquireZombie, not by class). Copied from UZomZombieSpawnSettings.
	UPROPERTY(BlueprintReadOnly, Category = "Zom|Pooling")
	TSubclassOf<AZomZombieBase> CrowdZombieClass;

	UPROPERTY(BlueprintReadOnly, Category = "Zom|Pooling")
	TSubclassOf<AZomZombieBase> BloaterZombieClass;

	// Crowd pool sized to the 15-zombie ceiling plus headroom (design doc suggests 20).
	UPROPERTY(BlueprintReadOnly, Category = "Zom|Pooling")
	int32 CrowdPoolSize = 20;

	// Separate small Bloater pool capped at 2.
	UPROPERTY(BlueprintReadOnly, Category = "Zom|Pooling")
	int32 BloaterPoolSize = 2;

	// Fired after a zombie is returned to the pool (death or despawn). UZomZombieSpawnDirector listens so the
	// spawn volume that owned it stops counting it.
	FZomOnZombieReleased OnZombieReleased;

	// Pre-spawns the pool (the one place this subsystem calls SpawnActor - initial setup, not a runtime
	// acquire/release). Runs automatically the tick after world begin play; later calls do nothing.
	UFUNCTION(BlueprintCallable, Category = "Zom|Pooling")
	void PrewarmPool();

	// Activates a hidden/disabled pooled zombie of the given category as InTypeData, at SpawnTransform (nudged out
	// of blocking geometry and other zombies if needed, with any leftover velocity cleared).
	// Returns nullptr if the budget for that category is exhausted. Boss is never drawn from this pool.
	UFUNCTION(BlueprintCallable, Category = "Zom|Pooling")
	AZomZombieBase* AcquireZombie(EZomZombieCategory Category, UZombieTypeData* InTypeData, const FTransform& SpawnTransform);

	// Returns a zombie to the pool (hidden, collision disabled, ticking off) and broadcasts OnZombieReleased.
	// Never calls Destroy(). Does nothing for a zombie that's already pooled.
	UFUNCTION(BlueprintCallable, Category = "Zom|Pooling")
	void ReleaseZombie(AZomZombieBase* Zombie);

	// Number of currently-activated (non-hidden) Crowd-category zombies. Read by UZomZombieSpawnDirector
	// against UZomDifficultyData::TargetActiveCrowdCount (Section 10).
	UFUNCTION(BlueprintCallable, Category = "Zom|Pooling")
	int32 GetActiveCrowdCount() const;

	// Capsule half height of CrowdZombieClass, used to lift floor-level spawn points (0 if no class is set).
	float GetCrowdCapsuleHalfHeight() const;

protected:
	// Game and PIE worlds only - editor and asset preview worlds shouldn't get pooled zombies.
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	// Deferred from OnWorldBeginPlay by a tick: that runs before actors get BeginPlay, so zombies spawned there would
	// have their BeginPlay (which can start the State Tree) run after DeactivateZombie had paused them.
	void StartSpawning();

	void SpawnPoolBatch(TSubclassOf<AZomZombieBase> ZombieClass, int32 Count, TArray<TObjectPtr<AZomZombieBase>>& OutPool);
	static void DeactivateZombie(AZomZombieBase* Zombie);

	UPROPERTY()
	TArray<TObjectPtr<AZomZombieBase>> CrowdPool;

	UPROPERTY()
	TArray<TObjectPtr<AZomZombieBase>> BloaterPool;

	bool bPoolPrewarmed = false;
};
