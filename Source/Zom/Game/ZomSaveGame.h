// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Zom/Game/ZomCheckpointEnums.h"
#include "Zom/Objectives/ZomObjectiveEnums.h"
#include "ZomSaveGame.generated.h"


class UZomItemData;


USTRUCT(BlueprintType)
struct FZomSavedInventorySlot
{
	GENERATED_BODY()

	// Soft reference - a save file shouldn't hard-keep item data assets loaded.
	UPROPERTY(BlueprintReadWrite, Category = "Zom|Save")
	TSoftObjectPtr<UZomItemData> ItemData;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Save")
	int32 Count = 0;
};


/**
 * Checkpoint ID, player attribute snapshot, objective step, resource/inventory flags (Section 11 of
 * the dev doc).
 */
UCLASS()
class ZOM_API UZomSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "Zom|Save")
	EZomCheckpointID CheckpointID = EZomCheckpointID::Entry;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Save")
	float SavedHealth = 0.f;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Save")
	float SavedStamina = 0.f;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Save")
	EZomObjectiveStep ObjectiveStep = EZomObjectiveStep::Fetch;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Save")
	TArray<FZomSavedInventorySlot> InventorySlots;
};
