// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ZomDefendVolume.generated.h"


class UBoxComponent;
class AZomZombieBase;


/**
 * [Proposed] Trigger volume that starts the Defend wave (Section 2/8 of the dev doc). On player entry, spawns
 * WaveSize zombies via UZomZombiePoolSubsystem/UZomZombieSpawnDirector at SpawnPoints, then polls (on a
 * low-frequency repeating timer, not per-frame Tick, per Section 15) whether they've all died/been released
 * back to the pool, completing EZomObjectiveStep::Defend once the wave is cleared.
 */
UCLASS()
class ZOM_API AZomDefendVolume : public AActor
{
	GENERATED_BODY()

public:
	AZomDefendVolume();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnVolumeOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void CheckWaveCleared();

	// -------------
	// Properties
	// -------------

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zom", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> TriggerVolume;

	// World-space points to spawn the wave's zombies at.
	UPROPERTY(EditAnywhere, Category = "Zom|Defend")
	TArray<FTransform> SpawnPoints;

	UPROPERTY(EditAnywhere, Category = "Zom|Defend")
	int32 WaveSize = 8;

	UPROPERTY(EditAnywhere, Category = "Zom|Defend")
	float WaveClearCheckInterval = 1.f;

private:
	TArray<TWeakObjectPtr<AZomZombieBase>> ActiveWaveZombies;
	FTimerHandle WaveClearCheckTimer;
	bool bWaveStarted = false;
};
