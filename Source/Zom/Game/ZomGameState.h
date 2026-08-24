// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "ZomGameState.generated.h"

UCLASS(Blueprintable, meta=(DisplayName="Zom Game State"))
class ZOM_API AZomGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AZomGameState();

};
