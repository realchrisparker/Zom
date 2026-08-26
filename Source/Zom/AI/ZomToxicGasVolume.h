// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ZomToxicGasVolume.generated.h"


class USphereComponent;
class UZomGameplayEffect;


/**
 * Radial trigger volume, spawned at Bloater death, persists 3s, applies an infection effect on
 * overlap (Section 5.3 of the dev doc). Not pooled - spawned fresh since it's rare/transient, unlike the
 * crowd/Bloater zombies themselves.
 */
UCLASS()
class ZOM_API AZomToxicGasVolume : public AActor
{
	GENERATED_BODY()

public:
	AZomToxicGasVolume();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnGasOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// -------------
	// Properties
	// -------------

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zom", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> GasSphere;

	// How long the volume persists before self-destructing.
	UPROPERTY(EditDefaultsOnly, Category = "Zom|ToxicGas")
	float LifeSpanSeconds = 3.f;

	// The infection effect applied on overlap - assign a Blueprint child of UZomGE_Infection configured with
	// DurationPolicy = HasDuration (the gas case extends rather than stacks; see UZomGE_Infection's header).
	UPROPERTY(EditDefaultsOnly, Category = "Zom|ToxicGas")
	TSubclassOf<UZomGameplayEffect> InfectionEffectClass;

	// Value supplied for UZomGE_Infection's Zom.SetByCaller.Duration.
	UPROPERTY(EditDefaultsOnly, Category = "Zom|ToxicGas")
	float InfectionDuration = 5.f;
};
