// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ZomDifficultyData.generated.h"


/**
 * [Design] One asset per tier, read by UZomZombieSpawnDirector. A new tier is a new asset, not new code
 * (Section 10 of the dev doc). Explicitly does NOT scale the Bloater cap or the Boss encounter, only crowd
 * density/cadence - no fields for either are added here, since that would misrepresent the design.
 */
UCLASS(BlueprintType)
class ZOM_API UZomDifficultyData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// How many crowd zombies UZomZombieSpawnDirector tries to keep active at once (within the pool's own budget).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom|Difficulty")
	int32 TargetActiveCrowdCount = 8;

	// Seconds between crowd spawn attempts.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom|Difficulty")
	float CrowdSpawnInterval = 5.f;
};
