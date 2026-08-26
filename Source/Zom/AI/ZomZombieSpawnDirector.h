// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Zom/AI/Enums/ZomAIEnums.h"
#include "ZomZombieSpawnDirector.generated.h"


class AZomZombieBase;
class UZombieTypeData;
class UZomZombiePoolSubsystem;
class UZomDifficultyData;


/**
 * Plain UObject, owned by UZomZombiePoolSubsystem. Decides which pooled zombie to activate, where,
 * and of which type (Section 9 of the dev doc). Reading UZomDifficultyData for the active tier is deferred to
 * Phase 9 - that class doesn't exist yet, so type/rate selection here is a simple random pick until then.
 */
UCLASS()
class ZOM_API UZomZombieSpawnDirector : public UObject
{
	GENERATED_BODY()

public:
	// Candidate types to pick from when spawning a Crowd zombie (Walker/Runner/Auds/Eyes - no C++ subclass
	// per type, per Section 5.2).
	UPROPERTY(EditDefaultsOnly, Category = "Zom|Spawning")
	TArray<TObjectPtr<UZombieTypeData>> CrowdTypes;

	UPROPERTY(EditDefaultsOnly, Category = "Zom|Spawning")
	TObjectPtr<UZombieTypeData> BloaterType;

	// Active difficulty tier (Section 10). A new tier is a new UZomDifficultyData asset assigned here, not
	// new code.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zom|Spawning")
	TObjectPtr<UZomDifficultyData> ActiveDifficultyTier;

	// Requests one Crowd-category activation at SpawnTransform, picking a random type from CrowdTypes. If
	// ActiveDifficultyTier is set, refuses once PoolSubsystem->GetActiveCrowdCount() already meets its
	// TargetActiveCrowdCount (unmetered if no tier is assigned).
	UFUNCTION(BlueprintCallable, Category = "Zom|Spawning")
	AZomZombieBase* RequestCrowdSpawn(UZomZombiePoolSubsystem* PoolSubsystem, const FTransform& SpawnTransform) const;

	// Requests one Bloater-category activation at SpawnTransform.
	UFUNCTION(BlueprintCallable, Category = "Zom|Spawning")
	AZomZombieBase* RequestBloaterSpawn(UZomZombiePoolSubsystem* PoolSubsystem, const FTransform& SpawnTransform) const;
};
