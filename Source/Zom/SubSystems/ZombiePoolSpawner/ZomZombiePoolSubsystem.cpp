// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/SubSystems/ZombiePoolSpawner/ZomZombiePoolSubsystem.h"
#include "Zom/SubSystems/ZombiePoolSpawner/Director/ZomZombieSpawnDirector.h"
#include "Zom/SubSystems/ZombiePoolSpawner/Settings/ZomZombieSpawnSettings.h"
#include "Zom/AI/Controllers/ZomZombieAIController.h"
#include "Zom/Characters/ZomZombieBase.h"
#include "Zom/Characters/Data/ZombieTypeData.h"
#include "Zom/Misc/ZomLogChannels.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"


void UZomZombiePoolSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	SpawnDirector = NewObject<UZomZombieSpawnDirector>(this);
}

void UZomZombiePoolSubsystem::Deinitialize()
{
	if (SpawnDirector)
	{
		SpawnDirector->StopDirector();
	}

	Super::Deinitialize();
}

void UZomZombiePoolSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	if (SpawnDirector)
	{
		SpawnDirector->ApplySettings(*GetDefault<UZomZombieSpawnSettings>());
	}

	InWorld.GetTimerManager().SetTimerForNextTick(this, &UZomZombiePoolSubsystem::StartSpawning);
}

bool UZomZombiePoolSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UZomZombiePoolSubsystem::StartSpawning()
{
	bSpawningStarted = true;

	for (UZombieTypeData* Type : PendingTypes)
	{
		PrewarmType(Type);
	}
	PendingTypes.Empty();

	if (SpawnDirector)
	{
		SpawnDirector->StartDirector();
	}
}

void UZomZombiePoolSubsystem::RegisterZombieType(UZombieTypeData* InTypeData)
{
	if (!InTypeData)
	{
		return;
	}

	if (!bSpawningStarted)
	{
		PendingTypes.AddUnique(InTypeData);
		return;
	}

	PrewarmType(InTypeData);
}

void UZomZombiePoolSubsystem::PrewarmType(UZombieTypeData* InTypeData)
{
	UWorld* World = GetWorld();
	if (!InTypeData || !World)
	{
		return;
	}

	UClass* ZombieClass = InTypeData->ZombieClass.LoadSynchronous();
	if (!ZombieClass)
	{
		UE_LOG(LogZomAI, Warning, TEXT("%s has no Zombie Class set - zombies of this type can't spawn."), *InTypeData->GetName());
		return;
	}

	// Types sharing a class top its one pool up to the largest PoolSize instead of each adding a batch.
	FZomZombieClassPool& ClassPool = ClassPools.FindOrAdd(ZombieClass);
	const int32 MissingCount = InTypeData->PoolSize - ClassPool.Zombies.Num();
	if (MissingCount <= 0)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int32 Index = 0; Index < MissingCount; ++Index)
	{
		if (AZomZombieBase* Zombie = World->SpawnActor<AZomZombieBase>(ZombieClass, FTransform::Identity, SpawnParams))
		{
			DeactivateZombie(Zombie);
			ClassPool.Zombies.Add(Zombie);
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

	// SetActorTickEnabled doesn't stop component ticks - without this a hidden zombie with collision off can keep
	// falling (e.g. prewarmed at the origin over no floor) until it passes KillZ and is destroyed out from under the pool.
	if (UCharacterMovementComponent* MovementComponent = Zombie->GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
		MovementComponent->DisableMovement();
		MovementComponent->SetComponentTickEnabled(false);
	}

	Zombie->SetActorHiddenInGame(true);
	Zombie->SetActorEnableCollision(false);
	Zombie->SetActorTickEnabled(false);
}

bool UZomZombiePoolSubsystem::HasAvailableZombie(const UZombieTypeData* InTypeData) const
{
	UClass* ZombieClass = InTypeData ? InTypeData->ZombieClass.Get() : nullptr;
	const FZomZombieClassPool* ClassPool = ZombieClass ? ClassPools.Find(ZombieClass) : nullptr;
	return ClassPool && ClassPool->Zombies.ContainsByPredicate([](const TObjectPtr<AZomZombieBase>& Zombie)
	{
		return Zombie && Zombie->IsHidden();
	});
}

AZomZombieBase* UZomZombiePoolSubsystem::AcquireZombie(UZombieTypeData* InTypeData, const FTransform& SpawnTransform)
{
	UClass* ZombieClass = InTypeData ? InTypeData->ZombieClass.Get() : nullptr;
	FZomZombieClassPool* ClassPool = ZombieClass ? ClassPools.Find(ZombieClass) : nullptr;
	if (!ClassPool)
	{
		return nullptr;
	}

	for (AZomZombieBase* Zombie : ClassPool->Zombies)
	{
		if (Zombie && Zombie->IsHidden())
		{
			// Collision back on before placement - FindTeleportSpot tests this actor's own capsule against the world.
			Zombie->SetActorEnableCollision(true);

			FVector SpawnLocation = SpawnTransform.GetLocation();
			const FRotator SpawnRotation = SpawnTransform.Rotator();
			// If no free spot is found SpawnLocation is left as requested - still better than refusing the spawn.
			GetWorld()->FindTeleportSpot(Zombie, SpawnLocation, SpawnRotation);
			Zombie->TeleportTo(SpawnLocation, SpawnRotation, false, true);

			if (UCharacterMovementComponent* MovementComponent = Zombie->GetCharacterMovement())
			{
				MovementComponent->SetComponentTickEnabled(true);
				MovementComponent->StopMovementImmediately();
				MovementComponent->SetDefaultMovementMode();
			}

			Zombie->InitializeForType(InTypeData);
			Zombie->SetActorHiddenInGame(false);
			Zombie->SetActorTickEnabled(true);

			if (AZomZombieAIController* AIController = Cast<AZomZombieAIController>(Zombie->GetController()))
			{
				AIController->ResumeBrain();
			}

			return Zombie;
		}
	}

	// Every zombie of this class is already active.
	return nullptr;
}

void UZomZombiePoolSubsystem::ReleaseZombie(AZomZombieBase* Zombie)
{
	// Already pooled - don't broadcast a second release for the same zombie.
	if (!Zombie || Zombie->IsHidden())
	{
		return;
	}

	DeactivateZombie(Zombie);
	OnZombieReleased.Broadcast(Zombie);
}

int32 UZomZombiePoolSubsystem::GetActiveCrowdCount() const
{
	int32 ActiveCount = 0;
	for (const TPair<TObjectPtr<UClass>, FZomZombieClassPool>& Pair : ClassPools)
	{
		for (const AZomZombieBase* Zombie : Pair.Value.Zombies)
		{
			const UZombieTypeData* TypeData = (Zombie && !Zombie->IsHidden()) ? Zombie->GetZombieTypeData() : nullptr;
			if (TypeData && TypeData->Category == EZomZombieCategory::Crowd)
			{
				++ActiveCount;
			}
		}
	}
	return ActiveCount;
}

float UZomZombiePoolSubsystem::GetCapsuleHalfHeight(const UZombieTypeData* InTypeData) const
{
	UClass* ZombieClass = InTypeData ? InTypeData->ZombieClass.Get() : nullptr;
	const AZomZombieBase* ZombieDefaults = ZombieClass ? Cast<AZomZombieBase>(ZombieClass->GetDefaultObject()) : nullptr;
	const UCapsuleComponent* Capsule = ZombieDefaults ? ZombieDefaults->GetCapsuleComponent() : nullptr;
	return Capsule ? Capsule->GetScaledCapsuleHalfHeight() : 0.f;
}
