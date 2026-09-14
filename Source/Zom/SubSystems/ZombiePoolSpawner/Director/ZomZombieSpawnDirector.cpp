// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/SubSystems/ZombiePoolSpawner/Director/ZomZombieSpawnDirector.h"
#include "Zom/SubSystems/ZombiePoolSpawner/ZomZombiePoolSubsystem.h"
#include "Zom/SubSystems/ZombiePoolSpawner/Settings/ZomZombieSpawnSettings.h"
#include "Zom/SubSystems/ZombiePoolSpawner/Volumes/ZomZombieSpawnVolume.h"
#include "Zom/SubSystems/ZombiePoolSpawner/Settings/Data/ZomDifficultyData.h"
#include "Zom/Characters/ZomZombieBase.h"
#include "Zom/Characters/Data/ZombieTypeData.h"
#include "Zom/Misc/ZomLogChannels.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"


void UZomZombieSpawnDirector::ApplySettings(const UZomZombieSpawnSettings& Settings)
{
	CrowdTypes.Reset();
	for (const TSoftObjectPtr<UZombieTypeData>& SoftType : Settings.CrowdTypes)
	{
		if (UZombieTypeData* Type = SoftType.LoadSynchronous())
		{
			CrowdTypes.Add(Type);
		}
	}

	BloaterType = Settings.BloaterType.LoadSynchronous();
	ActiveDifficultyTier = Settings.DefaultDifficultyTier.LoadSynchronous();

	FallbackSpawnInterval = Settings.FallbackSpawnInterval;
	ActivationRadius = Settings.ActivationRadius;
	// Never inside ActivationRadius, or a volume could be despawned and repopulated by consecutive updates.
	DespawnRadius = FMath::Max(Settings.DespawnRadius, Settings.ActivationRadius);
	MinSpawnDistanceFromPlayer = Settings.MinSpawnDistanceFromPlayer;
	SpawnLocationAttempts = FMath::Max(1, Settings.SpawnLocationAttempts);

	if (CrowdTypes.IsEmpty())
	{
		UE_LOG(LogZomAI, Warning, TEXT("No CrowdTypes set in Project Settings > Game > Zom Zombie Spawning - crowd spawns will be refused."));
	}
}

void UZomZombieSpawnDirector::StartDirector()
{
	UZomZombiePoolSubsystem* PoolSubsystem = GetPoolSubsystem();
	if (!PoolSubsystem)
	{
		return;
	}

	PoolSubsystem->OnZombieReleased.Remove(ZombieReleasedHandle);
	ZombieReleasedHandle = PoolSubsystem->OnZombieReleased.AddUObject(this, &UZomZombieSpawnDirector::HandleZombieReleased);

	UpdateDirector();
}

void UZomZombieSpawnDirector::StopDirector()
{
	UZomZombiePoolSubsystem* PoolSubsystem = GetPoolSubsystem();
	if (!PoolSubsystem)
	{
		return;
	}

	PoolSubsystem->OnZombieReleased.Remove(ZombieReleasedHandle);
	ZombieReleasedHandle.Reset();

	if (UWorld* World = PoolSubsystem->GetWorld())
	{
		World->GetTimerManager().ClearTimer(UpdateTimer);
	}
}

void UZomZombieSpawnDirector::RegisterSpawnVolume(AZomZombieSpawnVolume* Volume)
{
	if (Volume)
	{
		SpawnVolumes.AddUnique(Volume);
	}
}

void UZomZombieSpawnDirector::UnregisterSpawnVolume(AZomZombieSpawnVolume* Volume, bool bReleaseZombies)
{
	if (!Volume)
	{
		return;
	}

	SpawnVolumes.RemoveSwap(Volume);

	UZomZombiePoolSubsystem* PoolSubsystem = GetPoolSubsystem();
	if (!bReleaseZombies || !PoolSubsystem)
	{
		return;
	}

	// Copy - RemoveZombieForDespawn edits the volume's list.
	const TArray<TWeakObjectPtr<AZomZombieBase>> Zombies = Volume->GetSpawnedZombies();
	for (const TWeakObjectPtr<AZomZombieBase>& Tracked : Zombies)
	{
		if (AZomZombieBase* Zombie = Tracked.Get())
		{
			Volume->RemoveZombieForDespawn(Zombie);
			PoolSubsystem->ReleaseZombie(Zombie);
		}
	}
}

void UZomZombieSpawnDirector::PopulateVolume(AZomZombieSpawnVolume* Volume)
{
	UZomZombiePoolSubsystem* PoolSubsystem = GetPoolSubsystem();
	const UWorld* World = PoolSubsystem ? PoolSubsystem->GetWorld() : nullptr;
	if (!Volume || !World)
	{
		return;
	}

	RegisterSpawnVolume(Volume);

	if (!Volume->IsPopulationCycleActive())
	{
		if (!Volume->CanStartPopulationCycle(World->GetTimeSeconds()))
		{
			return;
		}
		Volume->StartPopulationCycle();
	}

	TArray<FVector> PlayerLocations;
	GatherPlayerLocations(PlayerLocations);

	if (!FillVolume(*PoolSubsystem, *Volume, PlayerLocations))
	{
		UE_LOG(LogZomAI, Log, TEXT("%s: spawn budget reached with %d zombie(s) still pending - topped up by later updates as budget frees."),
			*Volume->GetName(), Volume->GetPendingSpawnCount());
	}
}

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

void UZomZombieSpawnDirector::UpdateDirector()
{
	ScheduleNextUpdate();

	UZomZombiePoolSubsystem* PoolSubsystem = GetPoolSubsystem();
	const UWorld* World = PoolSubsystem ? PoolSubsystem->GetWorld() : nullptr;
	if (!World)
	{
		return;
	}

	SpawnVolumes.RemoveAllSwap([](const TWeakObjectPtr<AZomZombieSpawnVolume>& Tracked)
	{
		return !Tracked.IsValid();
	});

	TArray<FVector> PlayerLocations;
	GatherPlayerLocations(PlayerLocations);
	if (PlayerLocations.IsEmpty())
	{
		return;
	}

	const double Now = World->GetTimeSeconds();
	const double ActivationRadiusSq = FMath::Square(ActivationRadius);
	const double DespawnRadiusSq = FMath::Square(DespawnRadius);

	struct FSpawnCandidate
	{
		AZomZombieSpawnVolume* Volume;
		double NearestPlayerDistanceSq;
	};
	TArray<FSpawnCandidate> Candidates;

	for (const TWeakObjectPtr<AZomZombieSpawnVolume>& Tracked : SpawnVolumes)
	{
		AZomZombieSpawnVolume* Volume = Tracked.Get();
		Volume->RemoveStaleZombies();

		double NearestPlayerDistanceSq = TNumericLimits<double>::Max();
		for (const FVector& PlayerLocation : PlayerLocations)
		{
			NearestPlayerDistanceSq = FMath::Min(NearestPlayerDistanceSq, Volume->GetDistanceSquaredToPoint(PlayerLocation));
		}

		if (NearestPlayerDistanceSq > DespawnRadiusSq)
		{
			DespawnVolume(*PoolSubsystem, *Volume, PlayerLocations);
			continue;
		}

		// Between ActivationRadius and DespawnRadius: keep what's there, add nothing.
		if (NearestPlayerDistanceSq > ActivationRadiusSq)
		{
			continue;
		}

		if (!Volume->IsPopulationCycleActive() && Volume->CanStartPopulationCycle(Now))
		{
			Volume->StartPopulationCycle();
		}

		if (Volume->GetPendingSpawnCount() > 0)
		{
			Candidates.Add({ Volume, NearestPlayerDistanceSq });
		}
	}

	// Nearest first, so when the pool/tier budget runs out it's the volumes closest to a player that got zombies.
	Candidates.Sort([](const FSpawnCandidate& A, const FSpawnCandidate& B)
	{
		return A.NearestPlayerDistanceSq < B.NearestPlayerDistanceSq;
	});

	for (const FSpawnCandidate& Candidate : Candidates)
	{
		if (!FillVolume(*PoolSubsystem, *Candidate.Volume, PlayerLocations))
		{
			break;
		}
	}
}

void UZomZombieSpawnDirector::ScheduleNextUpdate()
{
	const UZomZombiePoolSubsystem* PoolSubsystem = GetPoolSubsystem();
	UWorld* World = PoolSubsystem ? PoolSubsystem->GetWorld() : nullptr;
	if (!World)
	{
		return;
	}

	// One-shot and re-read every update (not a looping timer) so swapping ActiveDifficultyTier changes the cadence too.
	const float Interval = ActiveDifficultyTier ? ActiveDifficultyTier->CrowdSpawnInterval : FallbackSpawnInterval;
	World->GetTimerManager().SetTimer(UpdateTimer, this, &UZomZombieSpawnDirector::UpdateDirector, FMath::Max(0.1f, Interval), false);
}

bool UZomZombieSpawnDirector::FillVolume(UZomZombiePoolSubsystem& PoolSubsystem, AZomZombieSpawnVolume& Volume, const TArray<FVector>& PlayerLocations)
{
	// Spawn volumes return navmesh (floor-level) points; lift by the capsule so it doesn't start in the ground.
	const float CapsuleHalfHeight = PoolSubsystem.GetCrowdCapsuleHalfHeight();

	for (int32 PendingCount = Volume.GetPendingSpawnCount(); PendingCount > 0; --PendingCount)
	{
		FVector NavLocation;
		if (!Volume.FindSpawnLocation(PlayerLocations, MinSpawnDistanceFromPlayer, SpawnLocationAttempts, NavLocation))
		{
			// No usable point this update (no navmesh in the box, or every try was too close to a player). Budget
			// isn't the problem, so other volumes can still spawn.
			UE_LOG(LogZomAI, Verbose, TEXT("%s: no valid spawn location found this update."), *Volume.GetName());
			return true;
		}

		const FTransform SpawnTransform(FRotator(0.f, FMath::FRandRange(0.f, 360.f), 0.f), NavLocation + FVector(0.f, 0.f, CapsuleHalfHeight));
		AZomZombieBase* Zombie = RequestCrowdSpawn(&PoolSubsystem, SpawnTransform);
		if (!Zombie)
		{
			return false;
		}

		Volume.AddSpawnedZombie(Zombie);
	}

	return true;
}

void UZomZombieSpawnDirector::DespawnVolume(UZomZombiePoolSubsystem& PoolSubsystem, AZomZombieSpawnVolume& Volume, const TArray<FVector>& PlayerLocations)
{
	const double DespawnRadiusSq = FMath::Square(DespawnRadius);

	// Copy - RemoveZombieForDespawn edits the volume's list.
	const TArray<TWeakObjectPtr<AZomZombieBase>> Zombies = Volume.GetSpawnedZombies();
	for (const TWeakObjectPtr<AZomZombieBase>& Tracked : Zombies)
	{
		AZomZombieBase* Zombie = Tracked.Get();
		if (!Zombie)
		{
			continue;
		}

		// A zombie that followed a player away from its volume stays active while it's still near one.
		const FVector ZombieLocation = Zombie->GetActorLocation();
		const bool bNearPlayer = PlayerLocations.ContainsByPredicate([&ZombieLocation, DespawnRadiusSq](const FVector& PlayerLocation)
		{
			return FVector::DistSquared(PlayerLocation, ZombieLocation) <= DespawnRadiusSq;
		});
		if (bNearPlayer)
		{
			continue;
		}

		// Untrack first so the OnZombieReleased broadcast doesn't count it as a kill.
		Volume.RemoveZombieForDespawn(Zombie);
		PoolSubsystem.ReleaseZombie(Zombie);
	}
}

void UZomZombieSpawnDirector::HandleZombieReleased(AZomZombieBase* Zombie)
{
	for (const TWeakObjectPtr<AZomZombieSpawnVolume>& Tracked : SpawnVolumes)
	{
		AZomZombieSpawnVolume* Volume = Tracked.Get();
		if (Volume && Volume->HandleZombieReleased(Zombie))
		{
			return;
		}
	}
}

void UZomZombieSpawnDirector::GatherPlayerLocations(TArray<FVector>& OutLocations) const
{
	const UZomZombiePoolSubsystem* PoolSubsystem = GetPoolSubsystem();
	const UWorld* World = PoolSubsystem ? PoolSubsystem->GetWorld() : nullptr;
	if (!World)
	{
		return;
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		const APlayerController* PlayerController = It->Get();
		const APawn* PlayerPawn = PlayerController ? PlayerController->GetPawn() : nullptr;
		if (PlayerPawn)
		{
			OutLocations.Add(PlayerPawn->GetActorLocation());
		}
	}
}

UZomZombiePoolSubsystem* UZomZombieSpawnDirector::GetPoolSubsystem() const
{
	return GetTypedOuter<UZomZombiePoolSubsystem>();
}
