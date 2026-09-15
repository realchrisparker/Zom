// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/SubSystems/ZombiePoolSpawner/Volumes/ZomZombieSpawnVolume.h"
#include "Zom/SubSystems/ZombiePoolSpawner/ZomZombiePoolSubsystem.h"
#include "Zom/SubSystems/ZombiePoolSpawner/Director/ZomZombieSpawnDirector.h"
#include "Zom/Characters/ZomZombieBase.h"
#include "Zom/Characters/Data/ZombieTypeData.h"
#include "Zom/Misc/ZomLogChannels.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"
#include "NavigationSystem.h"
#include "Engine/World.h"


AZomZombieSpawnVolume::AZomZombieSpawnVolume()
{
	PrimaryActorTick.bCanEverTick = false;

	SpawnBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnBox"));
	SpawnBox->InitBoxExtent(FVector(500.f, 500.f, 200.f));
	SpawnBox->ShapeColor = FColor::Red;
	// No collision unless this is a PlayerEnter volume (enabled in BeginPlay) - Director volumes are range-checked
	// on the director's timer instead.
	SpawnBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SpawnBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	SpawnBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SetRootComponent(SpawnBox);
}

#if WITH_EDITOR
void AZomZombieSpawnVolume::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = PropertyChangedEvent.GetMemberPropertyName();
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AZomZombieSpawnVolume, SpawnMinimum))
	{
		SpawnMaximum = FMath::Max(SpawnMaximum, SpawnMinimum);
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(AZomZombieSpawnVolume, SpawnMaximum))
	{
		SpawnMinimum = FMath::Min(SpawnMinimum, SpawnMaximum);
	}
}
#endif

void AZomZombieSpawnVolume::BeginPlay()
{
	Super::BeginPlay();

	if (!HasZombieTypes())
	{
		UE_LOG(LogZomAI, Warning, TEXT("%s has no ZombieTypes assigned - it will never spawn zombies."), *GetName());
	}

	if (ActivationMode == EZomSpawnVolumeActivation::PlayerEnter)
	{
		SpawnBox->OnComponentBeginOverlap.AddDynamic(this, &AZomZombieSpawnVolume::OnSpawnBoxOverlap);
		SpawnBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}

	if (UZomZombieSpawnDirector* Director = GetSpawnDirector())
	{
		Director->RegisterSpawnVolume(this);
	}
}

void AZomZombieSpawnVolume::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UZomZombieSpawnDirector* Director = GetSpawnDirector())
	{
		// Only hand zombies back when the volume leaves a world that keeps running (streamed out / destroyed) - on
		// PIE end, quit or level transition the pool is being torn down alongside it.
		const bool bReleaseZombies = EndPlayReason == EEndPlayReason::Destroyed || EndPlayReason == EEndPlayReason::RemovedFromWorld;
		Director->UnregisterSpawnVolume(this, bReleaseZombies);
	}

	Super::EndPlay(EndPlayReason);
}

void AZomZombieSpawnVolume::OnSpawnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	const APawn* OtherPawn = Cast<APawn>(OtherActor);
	if (!OtherPawn || !OtherPawn->IsPlayerControlled())
	{
		return;
	}

	// Entering mid-cycle or during the repopulate cooldown does nothing - the player has to come back later.
	if (bCycleActive || bExhausted || GetWorld()->GetTimeSeconds() < RepopulateAllowedTime)
	{
		return;
	}

	bPlayerTriggered = true;

	if (UZomZombieSpawnDirector* Director = GetSpawnDirector())
	{
		Director->PopulateVolume(this);
	}
}

int32 AZomZombieSpawnVolume::GetAliveCount() const
{
	int32 AliveCount = 0;
	for (const TWeakObjectPtr<AZomZombieBase>& Tracked : SpawnedZombies)
	{
		const AZomZombieBase* Zombie = Tracked.Get();
		if (Zombie && !Zombie->IsHidden())
		{
			++AliveCount;
		}
	}
	return AliveCount;
}

bool AZomZombieSpawnVolume::HasZombieTypes() const
{
	return ZombieTypes.ContainsByPredicate([](const TObjectPtr<UZombieTypeData>& Type)
	{
		return Type != nullptr;
	});
}

bool AZomZombieSpawnVolume::CanStartPopulationCycle(double WorldTime) const
{
	if (bCycleActive || bExhausted || WorldTime < RepopulateAllowedTime)
	{
		return false;
	}

	return ActivationMode == EZomSpawnVolumeActivation::Director || bPlayerTriggered;
}

void AZomZombieSpawnVolume::StartPopulationCycle()
{
	const int32 Minimum = FMath::Max(0, SpawnMinimum);
	const int32 Maximum = FMath::Max(Minimum, SpawnMaximum);

	bCycleActive = true;
	CycleSpawnedCount = 0;
	CycleTargetCount = FMath::RandRange(Minimum, Maximum);

	TryEndPopulationCycle();
}

int32 AZomZombieSpawnVolume::GetPendingSpawnCount() const
{
	return bCycleActive ? FMath::Max(0, CycleTargetCount - CycleSpawnedCount) : 0;
}

bool AZomZombieSpawnVolume::FindSpawnLocation(const TArray<FVector>& PlayerLocations, int32 Attempts, FVector& OutNavLocation) const
{
	UWorld* World = GetWorld();
	const UNavigationSystemV1* NavSystem = World ? FNavigationSystem::GetCurrent<UNavigationSystemV1>(World) : nullptr;
	if (!NavSystem)
	{
		return false;
	}

	const FTransform BoxTransform = SpawnBox->GetComponentTransform();
	const FVector LocalExtent = SpawnBox->GetUnscaledBoxExtent();
	// Search the box's full height so points on slopes/stairs inside it still find navmesh.
	const FVector QueryExtent(50.f, 50.f, SpawnBox->GetScaledBoxExtent().Z);
	// 0 (the default) never rejects a point.
	const double MinDistanceSq = FMath::Square(MinSpawnDistanceFromPlayer);

	for (int32 Attempt = 0; Attempt < Attempts; ++Attempt)
	{
		const FVector LocalPoint(FMath::FRandRange(-LocalExtent.X, LocalExtent.X), FMath::FRandRange(-LocalExtent.Y, LocalExtent.Y), 0.f);

		FNavLocation NavLocation;
		if (!NavSystem->ProjectPointToNavigation(BoxTransform.TransformPosition(LocalPoint), NavLocation, QueryExtent))
		{
			continue;
		}

		// Projection can land just outside a rotated or thin box - only keep points still inside it.
		const FVector LocalNavPoint = BoxTransform.InverseTransformPosition(NavLocation.Location);
		if (FMath::Abs(LocalNavPoint.X) > LocalExtent.X || FMath::Abs(LocalNavPoint.Y) > LocalExtent.Y || FMath::Abs(LocalNavPoint.Z) > LocalExtent.Z)
		{
			continue;
		}

		const bool bTooCloseToPlayer = PlayerLocations.ContainsByPredicate([&NavLocation, MinDistanceSq](const FVector& PlayerLocation)
		{
			return FVector::DistSquared(PlayerLocation, NavLocation.Location) < MinDistanceSq;
		});
		if (bTooCloseToPlayer)
		{
			continue;
		}

		OutNavLocation = NavLocation.Location;
		return true;
	}

	return false;
}

double AZomZombieSpawnVolume::GetDistanceSquaredToPoint(const FVector& Point) const
{
	return SpawnBox->Bounds.GetBox().ComputeSquaredDistanceToPoint(Point);
}

void AZomZombieSpawnVolume::AddSpawnedZombie(AZomZombieBase* Zombie)
{
	if (Zombie)
	{
		SpawnedZombies.Add(Zombie);
		++CycleSpawnedCount;
	}
}

bool AZomZombieSpawnVolume::HandleZombieReleased(const AZomZombieBase* Zombie)
{
	const int32 Index = SpawnedZombies.IndexOfByPredicate([Zombie](const TWeakObjectPtr<AZomZombieBase>& Tracked)
	{
		return Tracked.Get() == Zombie;
	});
	if (Index == INDEX_NONE)
	{
		return false;
	}

	// Killed, not despawned - it stays counted against this cycle.
	SpawnedZombies.RemoveAtSwap(Index);
	TryEndPopulationCycle();
	return true;
}

void AZomZombieSpawnVolume::RemoveZombieForDespawn(const AZomZombieBase* Zombie)
{
	const int32 Index = SpawnedZombies.IndexOfByPredicate([Zombie](const TWeakObjectPtr<AZomZombieBase>& Tracked)
	{
		return Tracked.Get() == Zombie;
	});
	if (Index == INDEX_NONE)
	{
		return;
	}

	// Not killed - give the slot back so it's respawned when a player returns.
	SpawnedZombies.RemoveAtSwap(Index);
	CycleSpawnedCount = FMath::Max(0, CycleSpawnedCount - 1);
}

void AZomZombieSpawnVolume::RemoveStaleZombies()
{
	const int32 RemovedCount = SpawnedZombies.RemoveAllSwap([](const TWeakObjectPtr<AZomZombieBase>& Tracked)
	{
		return !Tracked.IsValid();
	});

	if (RemovedCount > 0)
	{
		TryEndPopulationCycle();
	}
}

UZomZombieSpawnDirector* AZomZombieSpawnVolume::GetSpawnDirector() const
{
	const UWorld* World = GetWorld();
	const UZomZombiePoolSubsystem* PoolSubsystem = World ? World->GetSubsystem<UZomZombiePoolSubsystem>() : nullptr;
	return PoolSubsystem ? PoolSubsystem->SpawnDirector : nullptr;
}

void AZomZombieSpawnVolume::TryEndPopulationCycle()
{
	if (bCycleActive && CycleSpawnedCount >= CycleTargetCount && SpawnedZombies.IsEmpty())
	{
		EndPopulationCycle();
	}
}

void AZomZombieSpawnVolume::EndPopulationCycle()
{
	bCycleActive = false;
	bPlayerTriggered = false;
	bExhausted = bSpawnOnce;
	CycleTargetCount = 0;
	CycleSpawnedCount = 0;
	RepopulateAllowedTime = GetWorld()->GetTimeSeconds() + RepopulateDelay;
}
