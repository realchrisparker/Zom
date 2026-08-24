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

};
