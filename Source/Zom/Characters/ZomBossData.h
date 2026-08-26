// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ZomBossData.generated.h"


class USoundBase;


/** A single Boss bark: the sound to play and the subtitle text UZomSubtitleWidget shows alongside it. */
USTRUCT(BlueprintType)
struct FZomBossDialogueBark
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom|Boss")
	TObjectPtr<USoundBase> Sound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom|Boss")
	FText SubtitleText;
};


/**
 * [Design] Health, damage, phase threshold, dialogue audio cues (Section 6 of the dev doc). Tuning only -
 * unlike crowd UZombieTypeData, the Boss is intentionally not fully data-driven; behavior stays in code.
 * Three barks minimum (encounter start, mid-fight taunt, death line).
 */
UCLASS(BlueprintType)
class ZOM_API UZomBossData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom|Boss")
	float Health = 1000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom|Boss")
	float AttackDamage = 30.f;

	// Health fraction (0-1) at which Zom.Boss.Phase2 is toggled (~50% per the dev doc).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom|Boss")
	float PhaseTwoHealthThreshold = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom|Boss")
	FZomBossDialogueBark EncounterStartBark;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom|Boss")
	FZomBossDialogueBark MidFightTauntBark;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom|Boss")
	FZomBossDialogueBark DeathBark;
};
