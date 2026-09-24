// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "ZomMenuWidget.generated.h"


class AZomHUD;
class UZomGameInstance;


/**
 * Base for every menu screen pushed onto UZomUIRootWidget's MenuStack. Requests Menu input mode, so CommonUI
 * handles the cursor, UI-only input and gamepad focus while it's the top of the stack - never call SetInputMode
 * by hand. Back (IA_UI_Back) closes it by default.
 */
UCLASS(Abstract)
class ZOM_API UZomMenuWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UZomMenuWidget(const FObjectInitializer& ObjectInitializer);

	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;

protected:
	AZomHUD* GetZomHUD() const;
	UZomGameInstance* GetZomGameInstance() const;
};
