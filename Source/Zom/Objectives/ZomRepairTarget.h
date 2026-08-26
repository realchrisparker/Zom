// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ZomRepairTarget.generated.h"


/**
 * Interactable actor for the Repair objective (Section 2/8 of the dev doc). No generic interact-
 * input system exists yet (e.g. an "E to interact" prompt/trace on the player) - Interact() is the concrete
 * completion hook, callable once that system exists or from a Blueprint-side interaction trigger in the
 * meantime.
 */
UCLASS()
class ZOM_API AZomRepairTarget : public AActor
{
	GENERATED_BODY()

public:
	AZomRepairTarget();

	UFUNCTION(BlueprintCallable, Category = "Zom|Objectives")
	void Interact(AActor* Interactor);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zom", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> RepairTargetRoot;

	bool bRepaired = false;
};
