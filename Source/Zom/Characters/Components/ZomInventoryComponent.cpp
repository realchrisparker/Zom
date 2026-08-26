// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Characters/Components/ZomInventoryComponent.h"
#include "Zom/Items/ZomItemData.h"
#include "Zom/Abilities/ZomGameplayEffect.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"


UZomInventoryComponent::UZomInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UZomInventoryComponent::AddItem(UZomItemData* ItemData, int32 Count)
{
	if (!ItemData || Count <= 0)
	{
		return false;
	}

	if (ItemData->bStackable)
	{
		for (FZomInventorySlot& Slot : Slots)
		{
			if (Slot.ItemData == ItemData)
			{
				Slot.Count = FMath::Min(Slot.Count + Count, ItemData->MaxStackSize);
				OnInventoryChanged.Broadcast();
				return true;
			}
		}
	}

	FZomInventorySlot NewSlot;
	NewSlot.ItemData = ItemData;
	NewSlot.Count = ItemData->bStackable ? FMath::Min(Count, ItemData->MaxStackSize) : Count;
	Slots.Add(NewSlot);

	OnInventoryChanged.Broadcast();
	return true;
}

bool UZomInventoryComponent::RemoveItem(UZomItemData* ItemData, int32 Count)
{
	if (!ItemData || Count <= 0)
	{
		return false;
	}

	for (int32 Index = 0; Index < Slots.Num(); ++Index)
	{
		if (Slots[Index].ItemData == ItemData)
		{
			Slots[Index].Count -= Count;

			if (Slots[Index].Count <= 0)
			{
				Slots.RemoveAt(Index);
			}

			OnInventoryChanged.Broadcast();
			return true;
		}
	}

	return false;
}

bool UZomInventoryComponent::UseItem(UZomItemData* ItemData)
{
	if (!ItemData || ItemData->ItemType != EZomItemType::Consumable || GetItemCount(ItemData) <= 0)
	{
		return false;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	if (!ASC)
	{
		return false;
	}

	if (ItemData->RemoveEffectClass)
	{
		FGameplayEffectQuery Query;
		Query.EffectDefinition = ItemData->RemoveEffectClass;
		ASC->RemoveActiveEffects(Query);
	}

	if (ItemData->ConsumeEffectClass)
	{
		FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
		EffectContext.AddSourceObject(GetOwner());

		const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(ItemData->ConsumeEffectClass, 1.f, EffectContext);
		if (SpecHandle.IsValid())
		{
			ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}

	return RemoveItem(ItemData, 1);
}

int32 UZomInventoryComponent::GetItemCount(const UZomItemData* ItemData) const
{
	for (const FZomInventorySlot& Slot : Slots)
	{
		if (Slot.ItemData == ItemData)
		{
			return Slot.Count;
		}
	}

	return 0;
}
