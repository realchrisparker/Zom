// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/UI/Menus/ZomUIRootWidget.h"
#include "Zom/UI/Menus/ZomMenuWidget.h"
#include "CommonInputModeTypes.h"
#include "Input/UIActionBindingHandle.h"
#include "Widgets/CommonActivatableWidgetContainer.h"


UZomUIRootWidget::UZomUIRootWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bAutoActivate = true;
}

TOptional<FUIInputConfig> UZomUIRootWidget::GetDesiredInputConfig() const
{
	// Only applies while no menu is active - an active menu is further down the tree and its config wins.
	return FUIInputConfig(ECommonInputMode::Game, EMouseCaptureMode::CapturePermanently_IncludingInitialMouseDown);
}

UZomMenuWidget* UZomUIRootWidget::PushMenu(TSubclassOf<UZomMenuWidget> MenuClass)
{
	if (!MenuStack || !MenuClass)
	{
		return nullptr;
	}

	return MenuStack->AddWidget<UZomMenuWidget>(MenuClass);
}
