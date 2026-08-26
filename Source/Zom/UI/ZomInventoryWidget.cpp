// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/UI/ZomInventoryWidget.h"
#include "Zom/Characters/Components/ZomInventoryComponent.h"
#include "GameFramework/Pawn.h"


void UZomInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (APawn* OwningPawn = GetOwningPlayerPawn())
	{
		CachedInventory = OwningPawn->FindComponentByClass<UZomInventoryComponent>();
	}

	if (CachedInventory)
	{
		CachedInventory->OnInventoryChanged.AddDynamic(this, &UZomInventoryWidget::HandleInventoryChanged);
		OnInventoryUpdated();
	}
}

void UZomInventoryWidget::NativeDestruct()
{
	if (CachedInventory)
	{
		CachedInventory->OnInventoryChanged.RemoveDynamic(this, &UZomInventoryWidget::HandleInventoryChanged);
	}

	Super::NativeDestruct();
}

void UZomInventoryWidget::HandleInventoryChanged()
{
	OnInventoryUpdated();
}
