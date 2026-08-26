// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Game/ZomHUD.h"
#include "Zom/UI/ZomHUDWidget.h"
#include "Blueprint/UserWidget.h"

AZomHUD::AZomHUD()
{

}

// Called when the game starts or when spawned
void AZomHUD::BeginPlay()
{
	Super::BeginPlay();

	if (HUDWidgetClass)
	{
		HUDWidgetInstance = CreateWidget<UZomHUDWidget>(GetOwningPlayerController(), HUDWidgetClass);
		if (HUDWidgetInstance)
		{
			HUDWidgetInstance->AddToViewport();
		}
	}
}

void AZomHUD::DrawHUD()
{
	Super::DrawHUD();

}
