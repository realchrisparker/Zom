// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/SubSystems/ZombiePoolSpawner/ZomZombiePoolSubsystem.h"
#include "Zom/SubSystems/ZombiePoolSpawner/Director/ZomZombieSpawnDirector.h"
#include "Zom/SubSystems/ZombiePoolSpawner/Settings/ZomZombieSpawnSettings.h"
#include "Zom/AI/Controllers/ZomZombieAIController.h"
#include "Zom/Characters/ZomZombieBase.h"
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

	const UZomZombieSpawnSettings* Settings = GetDefault<UZomZombieSpawnSettings>();
	CrowdZombieClass = Settings->CrowdZombieClass.LoadSynchronous();
	BloaterZombieClass = Settings->BloaterZombieClass.LoadSynchronous();
	CrowdPoolSize = Settings->CrowdPoolSize;
	BloaterPoolSize = Settings->BloaterPoolSize;

	if (SpawnDirector)
	{
		SpawnDirector->ApplySettings(*Settings);
	}

	InWorld.GetTimerManager().SetTimerForNextTick(this, &UZomZombiePoolSubsystem::StartSpawning);
}

bool UZomZombiePoolSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UZomZombiePoolSubsystem::StartSpawning()
{
	PrewarmPool();

	if (SpawnDirector)
	{
		SpawnDirector->StartDirector();
	}
}

void UZomZombiePoolSubsystem::PrewarmPool()
{
	if (bPoolPrewarmed)
	{
		return;
	}
	bPoolPrewarmed = true;

	if (!CrowdZombieClass)
	{
		UE_LOG(LogZomAI, Warning, TEXT("No CrowdZombieClass set in Project Settings > Game > Zom Zombie Spawning - the crowd pool is empty."));
	}

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

AZomZombieBase* UZomZombiePoolSubsystem::AcquireZombie(EZomZombieCategory Category, UZombieTypeData* InTypeData, const FTransform& SpawnTransform)
{
	TArray<TObjectPtr<AZomZombieBase>>& Pool = (Category == EZomZombieCategory::Bloater) ? BloaterPool : CrowdPool;

	for (AZomZombieBase* Zombie : Pool)
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

	// Budget for this category is exhausted (or the pool was never prewarmed).
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
	for (const AZomZombieBase* Zombie : CrowdPool)
	{
		if (Zombie && !Zombie->IsHidden())
		{
			++ActiveCount;
		}
	}
	return ActiveCount;
}

float UZomZombiePoolSubsystem::GetCrowdCapsuleHalfHeight() const
{
	const AZomZombieBase* ZombieDefaults = CrowdZombieClass ? CrowdZombieClass->GetDefaultObject<AZomZombieBase>() : nullptr;
	const UCapsuleComponent* Capsule = ZombieDefaults ? ZombieDefaults->GetCapsuleComponent() : nullptr;
	return Capsule ? Capsule->GetScaledCapsuleHalfHeight() : 0.f;
}
