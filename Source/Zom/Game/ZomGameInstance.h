// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "ZomGameInstance.generated.h"


class UZomSaveGame;
class USoundControlBus;
class UUserWidget;


/**
 * Owns the single save slot and menu-driven level travel (New Game / Continue / Main Menu / Quit), since both
 * outlive any one level. Save handling moves to its own subsystem if multiple slots or save-anywhere arrive.
 */
UCLASS(Blueprintable, meta=(DisplayName="Zom Game Instance"))
class ZOM_API UZomGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UZomGameInstance();

	virtual void Init() override;
	virtual void Shutdown() override;

	// -------------
	// Save
	// -------------

	static const FString SaveSlotName;

	UFUNCTION(BlueprintPure, Category = "Zom|Save")
	bool HasSave() const;

	// Returns the save slot's contents, loading it from disk on first call. Null if no save exists.
	UFUNCTION(BlueprintCallable, Category = "Zom|Save")
	UZomSaveGame* GetLoadedSave();

	// Snapshots the current checkpoint (AZomGameMode::CurrentCheckpointID), the player's Health/Stamina and the
	// objective step into the slot. Loading it resumes at that checkpoint - this is not a save-anywhere.
	UFUNCTION(BlueprintCallable, Category = "Zom|Save")
	bool SaveCurrentState();

	UFUNCTION(BlueprintCallable, Category = "Zom|Save")
	void DeleteSave();

	// -------------
	// Menu flow
	// -------------

	// Deletes the save, resets objectives to the first step and opens GameplayMap at the Entry checkpoint.
	UFUNCTION(BlueprintCallable, Category = "Zom|Menu")
	void StartNewGame();

	// Opens GameplayMap; AZomGameMode restores from the save slot.
	UFUNCTION(BlueprintCallable, Category = "Zom|Menu")
	void ContinueGame();

	UFUNCTION(BlueprintCallable, Category = "Zom|Menu")
	void ReturnToMainMenu();

	UFUNCTION(BlueprintCallable, Category = "Zom|Menu")
	void QuitGame();

	// -------------
	// Settings
	// -------------

	// Pushes UZomGameUserSettings' volumes onto the control buses below. Called on start and by the settings
	// menu's Apply.
	UFUNCTION(BlueprintCallable, Category = "Zom|Settings")
	void ApplyAudioSettings();

	// -------------
	// Loading screen (read by UZomLoadingScreenSubsystem)
	// -------------

	TSubclassOf<UUserWidget> GetLoadingScreenClass() const { return LoadingScreenClass; }
	float GetLoadingScreenMinDisplayTime() const { return LoadingScreenMinDisplayTime; }
	float GetLoadingScreenTimeout() const { return LoadingScreenTimeout; }

protected:
	virtual void OnStart() override;

	UPROPERTY(EditDefaultsOnly, Category = "Zom|Menu")
	TSoftObjectPtr<UWorld> MainMenuMap;

	UPROPERTY(EditDefaultsOnly, Category = "Zom|Menu")
	TSoftObjectPtr<UWorld> GameplayMap;

	// AudioModulation control buses (CB_*) the settings volumes drive. Any left unset are skipped.
	UPROPERTY(EditDefaultsOnly, Category = "Zom|Settings")
	TObjectPtr<USoundControlBus> MasterVolumeBus;

	UPROPERTY(EditDefaultsOnly, Category = "Zom|Settings")
	TObjectPtr<USoundControlBus> MusicVolumeBus;

	UPROPERTY(EditDefaultsOnly, Category = "Zom|Settings")
	TObjectPtr<USoundControlBus> SFXVolumeBus;

	UPROPERTY(EditDefaultsOnly, Category = "Zom|Settings")
	TObjectPtr<USoundControlBus> DialogueVolumeBus;

	// Shown for every level load. Leave empty to disable the loading screen.
	UPROPERTY(EditDefaultsOnly, Category = "Zom|Loading")
	TSubclassOf<UUserWidget> LoadingScreenClass;

	// Seconds the loading screen stays up after the level loads, even if everything is already ready, so it
	// doesn't flash.
	UPROPERTY(EditDefaultsOnly, Category = "Zom|Loading", meta = (ClampMin = "0"))
	float LoadingScreenMinDisplayTime = 1.f;

	// Hard cap on waiting for shaders/PSOs/texture streaming after the level loads, so a resource that never
	// settles (e.g. texture pool over budget) can't hold the loading screen forever.
	UPROPERTY(EditDefaultsOnly, Category = "Zom|Loading", meta = (ClampMin = "1"))
	float LoadingScreenTimeout = 30.f;

private:
	void OpenMap(const TSoftObjectPtr<UWorld>& Map);

	UPROPERTY()
	TObjectPtr<UZomSaveGame> LoadedSave;
};
