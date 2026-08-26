// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ZomFetchItem.generated.h"


class USphereComponent;


/**
 * [Proposed] Pickup actor for the Fetch objective (Section 2/8 of the dev doc). Completes
 * EZomObjectiveStep::Fetch on player overlap.
 */
UCLASS()
class ZOM_API AZomFetchItem : public AActor
{
	GENERATED_BODY()

public:
	AZomFetchItem();

protected:
	UFUNCTION()
	void OnPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zom", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> PickupSphere;
};
