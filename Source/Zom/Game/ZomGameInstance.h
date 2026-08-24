// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "ZomGameInstance.generated.h"

UCLASS(Blueprintable, meta=(DisplayName="Zom Game Instance"))
class ZOM_API UZomGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UZomGameInstance();

	virtual void Init() override;
	virtual void Shutdown() override;

};
