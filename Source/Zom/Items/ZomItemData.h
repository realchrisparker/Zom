// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ZomItemData.generated.h"


class UZomGameplayAbility;
class UZomGameplayEffect;


/**
 * What an item's activation does. - the doc names weapons/consumables but not this split explicitly;
 * a single data asset class covering both (Section 7) needs some way to tell them apart at use-time.
 */
UENUM(BlueprintType)
enum class EZomItemType : uint8
{
	Weapon			UMETA(DisplayName = "Weapon"),
	Consumable		UMETA(DisplayName = "Consumable")
};


/**
 * Covers both weapons and consumables (Section 7 of the dev doc). Confirmed weapon list: Machete,
 * Fire Axe, Pistol, Pump Shotgun, Crossbow. Consumable list: Medicine, Pistol Ammo, Shotgun Shells,
 * Bandage.
 */
UCLASS(BlueprintType)
class ZOM_API UZomItemData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom|Item")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom|Item")
	EZomItemType ItemType = EZomItemType::Weapon;

	// Whether this item stacks (ammo/consumables) or is a single equippable instance (weapons).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom|Item")
	bool bStackable = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom|Item", meta = (EditCondition = "bStackable"))
	int32 MaxStackSize = 1;

	// -------------
	// Weapon (ItemType == Weapon)
	// -------------

	// Which ability slot this weapon maps to (Section 7's weapon list -> Section 4.2 abilities).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom|Item|Weapon", meta = (EditCondition = "ItemType == EZomItemType::Weapon"))
	TSubclassOf<UZomGameplayAbility> PrimaryAbilityClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom|Item|Weapon", meta = (EditCondition = "ItemType == EZomItemType::Weapon"))
	TSoftObjectPtr<USkeletalMesh> WeaponMesh;

	// -------------
	// Consumable (ItemType == Consumable)
	// -------------

	// Applied to the user's ASC when consumed (e.g. Medicine clears UZomGE_Infection, Bandage restores Health).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom|Item|Consumable", meta = (EditCondition = "ItemType == EZomItemType::Consumable"))
	TSubclassOf<UZomGameplayEffect> ConsumeEffectClass;

	// If set, consuming this item removes all active instances of this effect class instead of (or in addition
	// to) applying ConsumeEffectClass - Medicine's "clears UZomGE_Infection" behavior.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom|Item|Consumable", meta = (EditCondition = "ItemType == EZomItemType::Consumable"))
	TSubclassOf<UZomGameplayEffect> RemoveEffectClass;
};
