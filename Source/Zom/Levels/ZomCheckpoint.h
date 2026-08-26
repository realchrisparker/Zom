// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "Zom/Game/ZomCheckpointEnums.h"
#include "ZomCheckpoint.generated.h"


/** Carries EZomCheckpointID (Section 11 of the dev doc). */
UCLASS()
class ZOM_API AZomCheckpoint : public APlayerStart
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zom")
	EZomCheckpointID CheckpointID = EZomCheckpointID::Entry;
};
