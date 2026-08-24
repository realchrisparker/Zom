// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "ZomPlayerState.generated.h"


UCLASS(Blueprintable, meta=(DisplayName="Zom Player State"))
class ZOM_API AZomPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AZomPlayerState();

};
