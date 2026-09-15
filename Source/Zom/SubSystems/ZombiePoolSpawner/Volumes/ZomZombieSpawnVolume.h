// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ZomZombieSpawnVolume.generated.h"


class UBoxComponent;
class AZomZombieBase;
class UZombieTypeData;
class UZomZombieSpawnDirector;


UENUM(BlueprintType, meta = (DisplayName = "Zom Spawn Volume Activation"))
enum class EZomSpawnVolumeActivation : uint8
{
	// The spawn director populates the volume whenever a player comes within its ActivationRadius.
	Director		UMETA(DisplayName = "Director"),
	// Populated all at once the first time a player walks into the box - for set-piece encounters.
	PlayerEnter		UMETA(DisplayName = "Player Enter")
};


/**
 * Designer-placed box that zombies are spawned inside (Section 9 of the dev doc). Holds no spawning logic of its
 * own: it registers with UZomZombieSpawnDirector on BeginPlay, and the director decides when to populate it, picks
 * points inside it, and returns its zombies to UZomZombiePoolSubsystem once players move away.
 *
 * Each population cycle rolls a count between SpawnMinimum and SpawnMaximum. Killed zombies stay dead for the rest
 * of the cycle; zombies despawned because players left are restored when a player returns. Once every rolled
 * zombie has been killed the cycle ends, and the volume can repopulate after RepopulateDelay (never, if bSpawnOnce).
 * Spawn points are projected onto the navmesh, so the box must overlap navigable ground.
 */
UCLASS(meta = (DisplayName = "Zom Zombie Spawn Volume"))
class ZOM_API AZomZombieSpawnVolume : public AActor
{
	GENERATED_BODY()

public:
	AZomZombieSpawnVolume();

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	// -------------
	// Properties
	// -------------

	// Fewest zombies rolled per population cycle.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zom|Spawning", meta = (ClampMin = "0", UIMin = "0"))
	int32 SpawnMinimum = 3;

	// Most zombies rolled per population cycle. Kept >= SpawnMinimum when edited in the editor.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zom|Spawning", meta = (ClampMin = "0", UIMin = "0"))
	int32 SpawnMaximum = 6;

	// Types this volume picks from at random for each zombie it spawns, so each zone sets its own mix (Walker, Runner,
	// Tank, Bloater - anything but Boss). List a type more than once to weight it. Each type's Zombie Class is
	// prewarmed in the pool when the volume registers. A volume with none never spawns.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zom|Spawning")
	TArray<TObjectPtr<UZombieTypeData>> ZombieTypes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zom|Spawning")
	EZomSpawnVolumeActivation ActivationMode = EZomSpawnVolumeActivation::Director;

	// If true, the volume never repopulates once its first cycle has been cleared.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zom|Spawning")
	bool bSpawnOnce = false;

	// Seconds after a cycle is cleared before the volume can populate again.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zom|Spawning", meta = (ClampMin = "0", Units = "s", EditCondition = "!bSpawnOnce"))
	float RepopulateDelay = 60.f;

	// Spawn points closer than this to any player are rejected. 0 = no minimum.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zom|Spawning", meta = (ClampMin = "0", UIMin = "0", Units = "cm"))
	float MinSpawnDistanceFromPlayer = 0.f;

	// -------------
	// Functions
	// -------------

	// Zombies from this volume that are currently active (not yet killed or despawned).
	UFUNCTION(BlueprintPure, Category = "Zom|Spawning")
	int32 GetAliveCount() const;

	// -------------
	// Director interface (C++ only, driven by UZomZombieSpawnDirector)
	// -------------

	bool IsPopulationCycleActive() const { return bCycleActive; }

	// True if ZombieTypes has at least one assigned entry.
	bool HasZombieTypes() const;

	// Not mid-cycle, not exhausted, past RepopulateDelay, and (for PlayerEnter) triggered by a player.
	bool CanStartPopulationCycle(double WorldTime) const;

	// Rolls this cycle's count between SpawnMinimum and SpawnMaximum.
	void StartPopulationCycle();

	// Zombies still owed to the current cycle (rolled count minus those spawned and not despawned).
	int32 GetPendingSpawnCount() const;

	// Random navmesh point inside the box, at least MinSpawnDistanceFromPlayer from every player. Returns false if
	// none was found within Attempts tries.
	bool FindSpawnLocation(const TArray<FVector>& PlayerLocations, int32 Attempts, FVector& OutNavLocation) const;

	// Squared distance from Point to the box's world bounds (0 inside) - measured to the box, not its center, so
	// one ActivationRadius works for volumes of any size.
	double GetDistanceSquaredToPoint(const FVector& Point) const;

	void AddSpawnedZombie(AZomZombieBase* Zombie);

	// Called when any zombie is returned to the pool. If it was ours it counts as killed - returns true.
	bool HandleZombieReleased(const AZomZombieBase* Zombie);

	// Stops tracking Zombie without counting it as killed, so it's respawned when a player returns. Call before
	// releasing it to the pool.
	void RemoveZombieForDespawn(const AZomZombieBase* Zombie);

	// Drops zombies that were destroyed outside the pool.
	void RemoveStaleZombies();

	const TArray<TWeakObjectPtr<AZomZombieBase>>& GetSpawnedZombies() const { return SpawnedZombies; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Only bound for PlayerEnter volumes.
	UFUNCTION()
	void OnSpawnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// -------------
	// Components
	// -------------

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zom", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> SpawnBox;

private:
	UZomZombieSpawnDirector* GetSpawnDirector() const;

	void TryEndPopulationCycle();
	void EndPopulationCycle();

	TArray<TWeakObjectPtr<AZomZombieBase>> SpawnedZombies;

	int32 CycleTargetCount = 0;
	int32 CycleSpawnedCount = 0;
	double RepopulateAllowedTime = 0.0;
	bool bCycleActive = false;
	bool bExhausted = false;
	bool bPlayerTriggered = false;
};
