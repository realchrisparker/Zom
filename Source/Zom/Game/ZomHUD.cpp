// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Game/ZomHUD.h"
#include "Zom/UI/ZomHUDWidget.h"
#include "Zom/UI/Menus/ZomConfirmDialogWidget.h"
#include "Zom/UI/Menus/ZomMenuWidget.h"
#include "Zom/UI/Menus/ZomUIRootWidget.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

AZomHUD::AZomHUD()
{

}

// Called when the game starts or when spawned
void AZomHUD::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PlayerController = GetOwningPlayerController();

	if (HUDWidgetClass)
	{
		HUDWidgetInstance = CreateWidget<UZomHUDWidget>(PlayerController, HUDWidgetClass);
		if (HUDWidgetInstance)
		{
			HUDWidgetInstance->AddToViewport();
		}
	}

	if (UIRootClass)
	{
		UIRootInstance = CreateWidget<UZomUIRootWidget>(PlayerController, UIRootClass);
		if (UIRootInstance)
		{
			// Above the HUD widget so menus draw over it.
			UIRootInstance->AddToViewport(10);
			UIRootInstance->GetMenuStack()->OnDisplayedWidgetChanged().AddUObject(this, &AZomHUD::HandleDisplayedMenuChanged);
		}
	}

	if (StartupMenuClass)
	{
		PushMenu(StartupMenuClass);
	}
}

void AZomHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UIRootInstance)
	{
		UIRootInstance->GetMenuStack()->OnDisplayedWidgetChanged().RemoveAll(this);
	}

	Super::EndPlay(EndPlayReason);
}

void AZomHUD::DrawHUD()
{
	Super::DrawHUD();

}

UZomMenuWidget* AZomHUD::PushMenu(TSubclassOf<UZomMenuWidget> MenuClass)
{
	return UIRootInstance ? UIRootInstance->PushMenu(MenuClass) : nullptr;
}

void AZomHUD::ShowPauseMenu()
{
	// Guard against stacking a second pause menu if the key is pressed again before input mode flips.
	if (!IsMenuOpen())
	{
		PushMenu(PauseMenuClass);
	}
}

void AZomHUD::ShowSettingsMenu()
{
	PushMenu(SettingsMenuClass);
}

void AZomHUD::ShowConfirmDialog(const FText& Title, const FText& Body, TFunction<void()> OnConfirmed)
{
	if (UZomConfirmDialogWidget* Dialog = Cast<UZomConfirmDialogWidget>(PushMenu(ConfirmDialogClass)))
	{
		Dialog->Setup(Title, Body, MoveTemp(OnConfirmed));
	}
}

bool AZomHUD::IsMenuOpen() const
{
	return UIRootInstance && UIRootInstance->GetMenuStack()->GetActiveWidget() != nullptr;
}

void AZomHUD::HandleDisplayedMenuChanged(UCommonActivatableWidget* DisplayedWidget)
{
	const bool bMenuOpen = DisplayedWidget != nullptr;

	if (HUDWidgetInstance)
	{
		// The HUD is display-only, so HitTestInvisible when shown rather than restoring some earlier value.
		HUDWidgetInstance->SetVisibility(bMenuOpen ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}

	if (bPauseWhileMenuOpen)
	{
		if (APlayerController* PlayerController = GetOwningPlayerController())
		{
			PlayerController->SetPause(bMenuOpen);
		}
	}
}
