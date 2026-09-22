// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "ZomCheatManager.generated.h"

UCLASS(Blueprintable, meta=(DisplayName="Zom Cheat Manager"))
class ZOM_API UZomCheatManager : public UCheatManager
{
	GENERATED_BODY()

public:
	UZomCheatManager();

	// Toggles the player's noise emission rings (the Zom.Debug.Noise console variable).
	// 0 off, 1 rings, 2 rings plus the per-step readout.
	UFUNCTION(exec)
	void ZomDebugNoise(int32 Enable = 1);
};
