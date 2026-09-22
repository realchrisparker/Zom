// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Chaos/ChaosEngineInterface.h"
#include "GameplayTagContainer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Zom/SubSystems/SurfaceAudio/Data/ZomSurfaceAudioSet.h"
#include "ZomSurfaceAudioSubsystem.generated.h"


/**
 * Answers "how loud is this surface, and which footstep plays on it" in O(1).
 *
 * The authored data is a TMap on UZomSurfaceAudioSet, which is convenient to edit and the wrong shape to
 * query every few frames. This subsystem reads that asset exactly once and flattens it into a fixed array
 * indexed directly by EPhysicalSurface, so a lookup is a bounds check and an index - no hashing, no
 * soft-pointer resolve, no asset access on the hot path. The whole table is a few hundred bytes.
 *
 * A UGameInstanceSubsystem rather than a UWorldSubsystem: the data is level-independent, and checkpoint
 * restores reload the level, so world scoping would re-load and re-flatten the same asset every time.
 */
UCLASS()
class ZOM_API UZomSurfaceAudioSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	// -------------
	// Functions
	// -------------

	// Loads the configured surface set and builds the flattened table. One synchronous load at game
	// instance init, which is early enough that no gameplay code can beat it to a query.
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// Direct index into the flattened table. Never fails: an out-of-range surface returns the fallback.
	const FZomSurfaceNoiseEntry& GetSurfaceEntry(EPhysicalSurface Surface) const;

	// How loud this surface is underfoot, as a multiplier on a character's locomotion weight.
	UFUNCTION(BlueprintPure, Category = "Zom|Surface", meta = (DisplayName = "Get Surface Loudness"))
	float GetSurfaceLoudness(EPhysicalSurface Surface) const;

	// Which Zom.Audio.Footstep.* row to play on this surface. May be invalid, meaning "use the caller's default".
	UFUNCTION(BlueprintPure, Category = "Zom|Surface", meta = (DisplayName = "Get Surface Footstep Sound Tag"))
	FGameplayTag GetSurfaceFootstepSoundTag(EPhysicalSurface Surface) const;

	// Re-reads the data asset and rebuilds the table. For iterating on loudness values in PIE without
	// restarting; not a gameplay path.
	UFUNCTION(BlueprintCallable, Category = "Zom|Surface", meta = (DisplayName = "Rebuild Surface Table"))
	void RebuildSurfaceTable();

private:

	// -------------
	// Properties
	// -------------

	// Flattened, indexed by EPhysicalSurface, sized SurfaceType_Max. Deliberately holds no UObject pointer,
	// so it needs no UPROPERTY and takes no part in GC. If a USoundBase* is ever added to the entry struct,
	// this must become a UPROPERTY array or the sounds will be collected out from under it.
	TArray<FZomSurfaceNoiseEntry> SurfaceTable;

	// The loaded set, kept alive for RebuildSurfaceTable and so the soft pointer is resolved only once.
	UPROPERTY()
	TObjectPtr<UZomSurfaceAudioSet> SurfaceAudioSet;

	// Used when no set is configured at all, and as the out-of-range return. Audible on purpose: a silent
	// fallback would make an unconfigured project look like the noise system was broken.
	FZomSurfaceNoiseEntry FallbackEntry;
};
