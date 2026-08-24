// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ZomGameMode.generated.h"

UCLASS(Blueprintable, meta=(DisplayName="Zom Game Mode"))
class ZOM_API AZomGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AZomGameMode();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};
