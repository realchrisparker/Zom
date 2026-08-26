// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/AI/ZomZombieSpawnDirector.h"
#include "Zom/AI/ZomZombiePoolSubsystem.h"
#include "Zom/AI/ZombieTypeData.h"
#include "Zom/AI/ZomDifficultyData.h"


AZomZombieBase* UZomZombieSpawnDirector::RequestCrowdSpawn(UZomZombiePoolSubsystem* PoolSubsystem, const FTransform& SpawnTransform) const
{
	if (!PoolSubsystem || CrowdTypes.Num() == 0)
	{
		return nullptr;
	}

	if (ActiveDifficultyTier && PoolSubsystem->GetActiveCrowdCount() >= ActiveDifficultyTier->TargetActiveCrowdCount)
	{
		return nullptr;
	}

	UZombieTypeData* ChosenType = CrowdTypes[FMath::RandRange(0, CrowdTypes.Num() - 1)];
	return PoolSubsystem->AcquireZombie(EZomZombieCategory::Crowd, ChosenType, SpawnTransform);
}

AZomZombieBase* UZomZombieSpawnDirector::RequestBloaterSpawn(UZomZombiePoolSubsystem* PoolSubsystem, const FTransform& SpawnTransform) const
{
	if (!PoolSubsystem || !BloaterType)
	{
		return nullptr;
	}

	return PoolSubsystem->AcquireZombie(EZomZombieCategory::Bloater, BloaterType, SpawnTransform);
}
