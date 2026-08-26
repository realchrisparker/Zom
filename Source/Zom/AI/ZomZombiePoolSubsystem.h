// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Zom/AI/Enums/ZomAIEnums.h"
#include "ZomZombiePoolSubsystem.generated.h"


class AZomZombieBase;
class UZombieTypeData;
class UZomZombieSpawnDirector;


/**
 * [Proposed type] UWorldSubsystem, not UGameInstanceSubsystem - it owns actual pooled actors that live in one
 * world/level, unlike UZomObjectiveSubsystem/UZomSaveGame's state, which must survive a level reload (Section
 * 9 of the dev doc). Pre-spawns and recycles pooled zombie actors via SetActorHiddenInGame/SetActorEnableCollision,
 * never SpawnActor()/Destroy() at runtime - PrewarmPool() is initial setup, not a runtime activation/deactivation.
 */
UCLASS()
class ZOM_API UZomZombiePoolSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// Owned by this subsystem (Section 9 of the dev doc), created in Initialize().
	UPROPERTY(BlueprintReadOnly, Category = "Zom|Pooling")
	TObjectPtr<UZomZombieSpawnDirector> SpawnDirector;

	// Class to spawn for Crowd-category zombies (Walker/Runner/Auds/Eyes - type differs by UZombieTypeData
	// assigned per-activation via AcquireZombie, not by class).
	UPROPERTY(EditDefaultsOnly, Category = "Zom|Pooling")
	TSubclassOf<AZomZombieBase> CrowdZombieClass;

	UPROPERTY(EditDefaultsOnly, Category = "Zom|Pooling")
	TSubclassOf<AZomZombieBase> BloaterZombieClass;

	// Crowd pool sized to the 15-zombie ceiling plus headroom (design doc suggests 20).
	UPROPERTY(EditDefaultsOnly, Category = "Zom|Pooling")
	int32 CrowdPoolSize = 20;

	// Separate small Bloater pool capped at 2.
	UPROPERTY(EditDefaultsOnly, Category = "Zom|Pooling")
	int32 BloaterPoolSize = 2;

	// Pre-spawns the pool (the one place this subsystem calls SpawnActor - initial setup, not a runtime
	// acquire/release). Call once, e.g. from AZomGameMode::BeginPlay after CrowdZombieClass/BloaterZombieClass
	// are configured; this subsystem has no level-placed Blueprint defaults surface of its own.
	UFUNCTION(BlueprintCallable, Category = "Zom|Pooling")
	void PrewarmPool();

	// Activates a hidden/disabled pooled zombie of the given category as InTypeData, at SpawnTransform.
	// Returns nullptr if the budget for that category is exhausted. Boss is never drawn from this pool.
	UFUNCTION(BlueprintCallable, Category = "Zom|Pooling")
	AZomZombieBase* AcquireZombie(EZomZombieCategory Category, UZombieTypeData* InTypeData, const FTransform& SpawnTransform);

	// Returns a zombie to the pool (hidden, collision disabled, ticking off). Never calls Destroy().
	UFUNCTION(BlueprintCallable, Category = "Zom|Pooling")
	void ReleaseZombie(AZomZombieBase* Zombie);

	// Number of currently-activated (non-hidden) Crowd-category zombies. Read by UZomZombieSpawnDirector
	// against UZomDifficultyData::TargetActiveCrowdCount (Section 10).
	UFUNCTION(BlueprintCallable, Category = "Zom|Pooling")
	int32 GetActiveCrowdCount() const;

private:
	void SpawnPoolBatch(TSubclassOf<AZomZombieBase> ZombieClass, int32 Count, TArray<TObjectPtr<AZomZombieBase>>& OutPool);
	static void DeactivateZombie(AZomZombieBase* Zombie);

	UPROPERTY()
	TArray<TObjectPtr<AZomZombieBase>> CrowdPool;

	UPROPERTY()
	TArray<TObjectPtr<AZomZombieBase>> BloaterPool;

	bool bPoolPrewarmed = false;
};
