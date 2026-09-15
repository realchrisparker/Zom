// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ZomZombiePoolSubsystem.generated.h"


class AZomZombieBase;
class UZombieTypeData;
class UZomZombieSpawnDirector;


DECLARE_MULTICAST_DELEGATE_OneParam(FZomOnZombieReleased, AZomZombieBase* /*Zombie*/);


// Pooled instances of one zombie Blueprint class.
USTRUCT()
struct FZomZombieClassPool
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<TObjectPtr<AZomZombieBase>> Zombies;
};


/**
 * [Proposed type] UWorldSubsystem, not UGameInstanceSubsystem - it owns actual pooled actors that live in one
 * world/level, unlike UZomObjectiveSubsystem/UZomSaveGame's state, which must survive a level reload (Section
 * 9 of the dev doc). Pre-spawns and recycles pooled zombie actors via SetActorHiddenInGame/SetActorEnableCollision,
 * never SpawnActor()/Destroy() at runtime - prewarming is setup, not a runtime activation/deactivation.
 *
 * Keeps one pool per zombie Blueprint class (UZombieTypeData::ZombieClass), sized by UZombieTypeData::PoolSize, so a
 * Tank is always a BP_Tank and a Bloater a BP_Zombie_Bloater. Spawn/defend volumes register the types they use.
 * Types registered before spawning starts are prewarmed together the tick after world begin play (then SpawnDirector
 * starts); later ones - e.g. from a streamed-in volume - are prewarmed as they register.
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

	// Fired after a zombie is returned to the pool (death or despawn). UZomZombieSpawnDirector listens so the
	// spawn volume that owned it stops counting it.
	FZomOnZombieReleased OnZombieReleased;

	// Ensures InTypeData's ZombieClass has a pool of at least InTypeData->PoolSize (spawning the difference is the one
	// place this subsystem calls SpawnActor). Before spawning starts the type is only queued, so pooled zombies never
	// spawn ahead of actor BeginPlay. Safe to call repeatedly; null does nothing.
	UFUNCTION(BlueprintCallable, Category = "Zom|Pooling")
	void RegisterZombieType(UZombieTypeData* InTypeData);

	// True if InTypeData's class pool has an inactive zombie ready to activate.
	UFUNCTION(BlueprintPure, Category = "Zom|Pooling")
	bool HasAvailableZombie(const UZombieTypeData* InTypeData) const;

	// Activates an inactive pooled zombie of InTypeData's ZombieClass as InTypeData, at SpawnTransform (nudged out of
	// blocking geometry and other zombies if needed, with any leftover velocity cleared). Returns nullptr if that
	// class has no free zombie or was never registered. Budget rules live in UZomZombieSpawnDirector, not here.
	UFUNCTION(BlueprintCallable, Category = "Zom|Pooling")
	AZomZombieBase* AcquireZombie(UZombieTypeData* InTypeData, const FTransform& SpawnTransform);

	// Returns a zombie to the pool (hidden, collision disabled, ticking off) and broadcasts OnZombieReleased.
	// Never calls Destroy(). Does nothing for a zombie that's already pooled.
	UFUNCTION(BlueprintCallable, Category = "Zom|Pooling")
	void ReleaseZombie(AZomZombieBase* Zombie);

	// Active zombies whose current type is Crowd-category, across every class pool. Read by UZomZombieSpawnDirector
	// against UZomDifficultyData::TargetActiveCrowdCount (Section 10).
	UFUNCTION(BlueprintCallable, Category = "Zom|Pooling")
	int32 GetActiveCrowdCount() const;

	// Capsule half height of InTypeData's ZombieClass, used to lift floor-level spawn points (0 if not loaded).
	float GetCapsuleHalfHeight(const UZombieTypeData* InTypeData) const;

protected:
	// Game and PIE worlds only - editor and asset preview worlds shouldn't get pooled zombies.
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	// Deferred from OnWorldBeginPlay by a tick: that runs before actors get BeginPlay, so zombies spawned there would
	// have their BeginPlay (which can start the State Tree) run after DeactivateZombie had paused them.
	void StartSpawning();

	void PrewarmType(UZombieTypeData* InTypeData);
	static void DeactivateZombie(AZomZombieBase* Zombie);

	UPROPERTY()
	TMap<TObjectPtr<UClass>, FZomZombieClassPool> ClassPools;

	// Registered before StartSpawning; prewarmed there.
	UPROPERTY()
	TArray<TObjectPtr<UZombieTypeData>> PendingTypes;

	bool bSpawningStarted = false;
};
