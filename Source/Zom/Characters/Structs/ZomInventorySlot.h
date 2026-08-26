// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZomInventorySlot.generated.h"

// Forward declarations

class UZomItemData;

/**
 * Represents a single slot in the inventory, containing an item and its count.
 */
USTRUCT(BlueprintType)
struct FZomInventorySlot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Zom|Inventory")
	TObjectPtr<UZomItemData> ItemData;

	UPROPERTY(BlueprintReadOnly, Category = "Zom|Inventory")
	int32 Count = 0;
};
