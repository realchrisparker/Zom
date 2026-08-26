// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ZomGameMode.generated.h"


class UZomSaveGame;


UCLASS(Blueprintable, meta=(DisplayName="Zom Game Mode"))
class ZOM_API AZomGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AZomGameMode();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Loads the save slot before ChoosePlayerStart_Implementation runs, since checkpoint selection depends on it.
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	// Selects the AZomCheckpoint matching the loaded save's CheckpointID (Entry if no save exists).
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	// Reapplies saved Health/Stamina (via a Gameplay Effect, not a direct attribute write) and resumes
	// UZomObjectiveSubsystem from the saved step, once the player's pawn/ASC exist (Section 11).
	virtual void PostLogin(APlayerController* NewPlayer) override;

private:
	static const FString SaveSlotName;

	UPROPERTY()
	TObjectPtr<UZomSaveGame> LoadedSaveGame;
};
