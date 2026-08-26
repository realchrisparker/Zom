// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Zom/Characters/Structs/ZomInventorySlot.h"
#include "ZomInventoryComponent.generated.h"

// Forward declarations

class UZomItemData;


// Delegates

// Broadcasts when the inventory changes (Section 7 of the dev doc). UI widgets bind to this delegate to update themselves when the inventory changes, avoiding per-frame polling.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnZomInventoryChanged);


/**
 * Attached to AZomPlayerCharacter (Section 7 of the dev doc). Delegate-driven, no per-frame polling
 * per Section 15's performance rules - UI widgets bind to OnInventoryChanged.
 */
UCLASS(ClassGroup = (Zom), meta = (BlueprintSpawnableComponent, DisplayName = "Zom Inventory Component"))
class ZOM_API UZomInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UZomInventoryComponent();

	UPROPERTY(BlueprintAssignable, Category = "Zom|Inventory")
	FOnZomInventoryChanged OnInventoryChanged;

	// Adds Count of ItemData, stacking into an existing slot if bStackable, otherwise adding a new slot per unit.
	UFUNCTION(BlueprintCallable, Category = "Zom|Inventory")
	bool AddItem(UZomItemData* ItemData, int32 Count = 1);

	// Removes up to Count of ItemData. Returns true if at least one was removed.
	UFUNCTION(BlueprintCallable, Category = "Zom|Inventory")
	bool RemoveItem(UZomItemData* ItemData, int32 Count = 1);

	// Consumes one of ItemData: applies ConsumeEffectClass and/or removes active RemoveEffectClass instances
	// on the owning actor's ASC, then removes one from the stack. Only meaningful for Consumable items -
	// equipping a Weapon item isn't part of this pass (no weapon-attach system exists yet).
	UFUNCTION(BlueprintCallable, Category = "Zom|Inventory")
	bool UseItem(UZomItemData* ItemData);

	UFUNCTION(BlueprintCallable, Category = "Zom|Inventory")
	int32 GetItemCount(const UZomItemData* ItemData) const;

	UFUNCTION(BlueprintCallable, Category = "Zom|Inventory")
	const TArray<FZomInventorySlot>& GetSlots() const { return Slots; }

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Zom|Inventory")
	TArray<FZomInventorySlot> Slots;
};
