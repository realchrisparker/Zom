// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ZomHUD.generated.h"

UCLASS(Blueprintable, meta=(DisplayName="Zom HUD"))
class ZOM_API AZomHUD : public AHUD
{
	GENERATED_BODY()

public:
	AZomHUD();

	virtual void DrawHUD() override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};
