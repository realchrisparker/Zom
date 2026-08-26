// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Objectives/ZomDefendVolume.h"
#include "Zom/Objectives/ZomObjectiveSubsystem.h"
#include "Zom/AI/ZomZombiePoolSubsystem.h"
#include "Zom/AI/ZomZombieSpawnDirector.h"
#include "Zom/Characters/ZomZombieBase.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"


AZomDefendVolume::AZomDefendVolume()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
	TriggerVolume->InitBoxExtent(FVector(200.f, 200.f, 100.f));
	TriggerVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SetRootComponent(TriggerVolume);

	TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AZomDefendVolume::OnVolumeOverlap);
}

void AZomDefendVolume::BeginPlay()
{
	Super::BeginPlay();
}

void AZomDefendVolume::OnVolumeOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bWaveStarted)
	{
		return;
	}

	const APawn* OtherPawn = Cast<APawn>(OtherActor);
	if (!OtherPawn || !OtherPawn->IsPlayerControlled() || SpawnPoints.Num() == 0)
	{
		return;
	}

	UZomZombiePoolSubsystem* PoolSubsystem = GetWorld()->GetSubsystem<UZomZombiePoolSubsystem>();
	if (!PoolSubsystem || !PoolSubsystem->SpawnDirector)
	{
		return;
	}

	bWaveStarted = true;

	for (int32 Index = 0; Index < WaveSize; ++Index)
	{
		const FTransform& SpawnTransform = SpawnPoints[Index % SpawnPoints.Num()];
		if (AZomZombieBase* Zombie = PoolSubsystem->SpawnDirector->RequestCrowdSpawn(PoolSubsystem, SpawnTransform))
		{
			ActiveWaveZombies.Add(Zombie);
		}
	}

	GetWorldTimerManager().SetTimer(WaveClearCheckTimer, this, &AZomDefendVolume::CheckWaveCleared, WaveClearCheckInterval, true);
}

void AZomDefendVolume::CheckWaveCleared()
{
	for (const TWeakObjectPtr<AZomZombieBase>& WeakZombie : ActiveWaveZombies)
	{
		const AZomZombieBase* Zombie = WeakZombie.Get();
		// Still alive: valid and not yet returned to the pool (pooled zombies are hidden on release).
		if (Zombie && !Zombie->IsHidden())
		{
			return;
		}
	}

	GetWorldTimerManager().ClearTimer(WaveClearCheckTimer);

	if (UZomObjectiveSubsystem* ObjectiveSubsystem = GetGameInstance()->GetSubsystem<UZomObjectiveSubsystem>())
	{
		ObjectiveSubsystem->CompleteStep(EZomObjectiveStep::Defend);
	}
}
