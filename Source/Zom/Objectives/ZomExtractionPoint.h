// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Zom/Objectives/ZomObjectiveEnums.h"
#include "ZomExtractionPoint.generated.h"


class UBoxComponent;


/**
 * [Proposed] Locked-until-objectives-complete exit trigger (Section 2/8 of the dev doc). Listens to
 * UZomObjectiveSubsystem's completion delegate and unlocks on EZomObjectiveStep::Boss, completing
 * EZomObjectiveStep::Extracted on player overlap once unlocked.
 */
UCLASS()
class ZOM_API AZomExtractionPoint : public AActor
{
	GENERATED_BODY()

public:
	AZomExtractionPoint();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleObjectiveStepCompleted(EZomObjectiveStep CompletedStep);

	UFUNCTION()
	void OnExtractionOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zom", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> ExtractionVolume;

	bool bUnlocked = false;
};
