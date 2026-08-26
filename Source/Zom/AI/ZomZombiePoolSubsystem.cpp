// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/AI/ZomZombiePoolSubsystem.h"
#include "Zom/AI/ZomZombieSpawnDirector.h"
#include "Zom/AI/Controllers/ZomZombieAIController.h"
#include "Zom/Characters/ZomZombieBase.h"
#include "Engine/World.h"


void UZomZombiePoolSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	SpawnDirector = NewObject<UZomZombieSpawnDirector>(this);
}

void UZomZombiePoolSubsystem::PrewarmPool()
{
	if (bPoolPrewarmed)
	{
		return;
	}
	bPoolPrewarmed = true;

	SpawnPoolBatch(CrowdZombieClass, CrowdPoolSize, CrowdPool);
	SpawnPoolBatch(BloaterZombieClass, BloaterPoolSize, BloaterPool);
}

void UZomZombiePoolSubsystem::SpawnPoolBatch(TSubclassOf<AZomZombieBase> ZombieClass, int32 Count, TArray<TObjectPtr<AZomZombieBase>>& OutPool)
{
	UWorld* World = GetWorld();
	if (!ZombieClass || !World)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int32 Index = 0; Index < Count; ++Index)
	{
		if (AZomZombieBase* Zombie = World->SpawnActor<AZomZombieBase>(ZombieClass, FTransform::Identity, SpawnParams))
		{
			DeactivateZombie(Zombie);
			OutPool.Add(Zombie);
		}
	}
}

void UZomZombiePoolSubsystem::DeactivateZombie(AZomZombieBase* Zombie)
{
	if (!Zombie)
	{
		return;
	}

	// The AIController possesses once and is never destroyed/re-spawned alongside pooling - pause its State
	// Tree explicitly so a hidden zombie's brain doesn't keep running.
	if (AZomZombieAIController* AIController = Cast<AZomZombieAIController>(Zombie->GetController()))
	{
		AIController->PauseBrain();
	}

	Zombie->SetActorHiddenInGame(true);
	Zombie->SetActorEnableCollision(false);
	Zombie->SetActorTickEnabled(false);
}

AZomZombieBase* UZomZombiePoolSubsystem::AcquireZombie(EZomZombieCategory Category, UZombieTypeData* InTypeData, const FTransform& SpawnTransform)
{
	TArray<TObjectPtr<AZomZombieBase>>& Pool = (Category == EZomZombieCategory::Bloater) ? BloaterPool : CrowdPool;

	for (AZomZombieBase* Zombie : Pool)
	{
		if (Zombie && Zombie->IsHidden())
		{
			Zombie->SetActorTransform(SpawnTransform);
			Zombie->InitializeForType(InTypeData);
			Zombie->SetActorHiddenInGame(false);
			Zombie->SetActorEnableCollision(true);
			Zombie->SetActorTickEnabled(true);

			if (AZomZombieAIController* AIController = Cast<AZomZombieAIController>(Zombie->GetController()))
			{
				AIController->ResumeBrain();
			}

			return Zombie;
		}
	}

	// Budget for this category is exhausted (or the pool was never prewarmed).
	return nullptr;
}

void UZomZombiePoolSubsystem::ReleaseZombie(AZomZombieBase* Zombie)
{
	DeactivateZombie(Zombie);
}

int32 UZomZombiePoolSubsystem::GetActiveCrowdCount() const
{
	int32 ActiveCount = 0;
	for (const AZomZombieBase* Zombie : CrowdPool)
	{
		if (Zombie && !Zombie->IsHidden())
		{
			++ActiveCount;
		}
	}
	return ActiveCount;
}
