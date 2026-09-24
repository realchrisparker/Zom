// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/UI/Menus/ZomMenuWidget.h"
#include "Zom/Game/ZomGameInstance.h"
#include "Zom/Game/ZomHUD.h"
#include "CommonInputModeTypes.h"
#include "Input/UIActionBindingHandle.h"


UZomMenuWidget::UZomMenuWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bIsBackHandler = true;
}

TOptional<FUIInputConfig> UZomMenuWidget::GetDesiredInputConfig() const
{
	return FUIInputConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture);
}

AZomHUD* UZomMenuWidget::GetZomHUD() const
{
	const APlayerController* PlayerController = GetOwningPlayer();
	return PlayerController ? PlayerController->GetHUD<AZomHUD>() : nullptr;
}

UZomGameInstance* UZomMenuWidget::GetZomGameInstance() const
{
	return GetGameInstance<UZomGameInstance>();
}
