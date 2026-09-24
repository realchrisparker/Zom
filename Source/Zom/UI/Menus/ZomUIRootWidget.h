// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "ZomUIRootWidget.generated.h"


class UCommonActivatableWidgetStack;
class UZomMenuWidget;


/**
 * The one full-screen UI root per local player, created by AZomHUD. Holds a single MenuStack that every menu
 * (main, pause, settings, confirm dialogs) is pushed onto; only the top entry is shown and active.
 *
 * Itself activatable (auto-activated) so that when the stack is empty CommonUI falls back to this widget's Game
 * input config - otherwise CommonUI leaves the last menu's mouse/input mode applied after it closes.
 */
UCLASS(Abstract)
class ZOM_API UZomUIRootWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UZomUIRootWidget(const FObjectInitializer& ObjectInitializer);

	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;

	UZomMenuWidget* PushMenu(TSubclassOf<UZomMenuWidget> MenuClass);

	UCommonActivatableWidgetStack* GetMenuStack() const { return MenuStack; }

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonActivatableWidgetStack> MenuStack;
};
