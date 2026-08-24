// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameSession.h"
#include "ZomGameSession.generated.h"

UCLASS(Blueprintable, meta=(DisplayName="Zom Game Session"))
class ZOM_API AZomGameSession : public AGameSession
{
	GENERATED_BODY()

public:
	AZomGameSession();

};
