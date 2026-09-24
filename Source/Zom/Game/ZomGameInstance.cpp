// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Game/ZomGameInstance.h"
#include "Zom/Game/ZomGameMode.h"
#include "Zom/Game/ZomSaveGame.h"
#include "Zom/Characters/Base/ZomCharacterBase.h"
#include "Zom/Abilities/AttributeSets/ZomPlayerAttributeSet.h"
#include "Zom/Objectives/ZomObjectiveSubsystem.h"
#include "Zom/Settings/ZomGameUserSettings.h"
#include "Zom/Misc/ZomLogChannels.h"
#include "AbilitySystemComponent.h"
#include "AudioModulationStatics.h"
#include "SoundControlBus.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"


const FString UZomGameInstance::SaveSlotName = TEXT("ZomSaveSlot");

UZomGameInstance::UZomGameInstance()
{

}

void UZomGameInstance::Init()
{
	Super::Init();

}

void UZomGameInstance::Shutdown()
{
	Super::Shutdown();

}

void UZomGameInstance::OnStart()
{
	Super::OnStart();

	// Needs a world for the audio device, which Init() doesn't have yet. Global bus mix values persist on the
	// audio device across level travel, so once here plus on Apply is enough.
	ApplyAudioSettings();
}

// =========================================
// Save
// =========================================

bool UZomGameInstance::HasSave() const
{
	return LoadedSave || UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0);
}

UZomSaveGame* UZomGameInstance::GetLoadedSave()
{
	if (!LoadedSave && UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
	{
		LoadedSave = Cast<UZomSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
	}

	return LoadedSave;
}

bool UZomGameInstance::SaveCurrentState()
{
	const UWorld* World = GetWorld();
	const AZomGameMode* GameMode = World ? World->GetAuthGameMode<AZomGameMode>() : nullptr;
	if (!GameMode)
	{
		UE_LOG(LogZomGame, Warning, TEXT("SaveCurrentState called outside a gameplay map (no AZomGameMode) - nothing saved."));
		return false;
	}

	UZomSaveGame* SaveGame = Cast<UZomSaveGame>(UGameplayStatics::CreateSaveGameObject(UZomSaveGame::StaticClass()));
	SaveGame->CheckpointID = GameMode->GetCurrentCheckpointID();

	const AZomCharacterBase* PlayerCharacter = Cast<AZomCharacterBase>(UGameplayStatics::GetPlayerCharacter(this, 0));
	if (const UAbilitySystemComponent* ASC = PlayerCharacter ? PlayerCharacter->GetAbilitySystemComponent() : nullptr)
	{
		SaveGame->SavedHealth = ASC->GetNumericAttribute(UZomAttributeSetBase::GetHealthAttribute());
		SaveGame->SavedStamina = ASC->GetNumericAttribute(UZomPlayerAttributeSet::GetStaminaAttribute());
	}

	if (const UZomObjectiveSubsystem* ObjectiveSubsystem = GetSubsystem<UZomObjectiveSubsystem>())
	{
		SaveGame->ObjectiveStep = ObjectiveSubsystem->GetCurrentStep();
	}

	if (!UGameplayStatics::SaveGameToSlot(SaveGame, SaveSlotName, 0))
	{
		UE_LOG(LogZomGame, Error, TEXT("Failed to write save slot '%s'."), *SaveSlotName);
		return false;
	}

	LoadedSave = SaveGame;
	return true;
}

void UZomGameInstance::DeleteSave()
{
	LoadedSave = nullptr;

	if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
	{
		UGameplayStatics::DeleteGameInSlot(SaveSlotName, 0);
	}
}

// =========================================
// Menu flow
// =========================================

void UZomGameInstance::StartNewGame()
{
	DeleteSave();

	if (UZomObjectiveSubsystem* ObjectiveSubsystem = GetSubsystem<UZomObjectiveSubsystem>())
	{
		ObjectiveSubsystem->ResetObjectives();
	}

	OpenMap(GameplayMap);
}

void UZomGameInstance::ContinueGame()
{
	OpenMap(GameplayMap);
}

void UZomGameInstance::ReturnToMainMenu()
{
	OpenMap(MainMenuMap);
}

void UZomGameInstance::QuitGame()
{
	UKismetSystemLibrary::QuitGame(this, GetFirstLocalPlayerController(), EQuitPreference::Quit, false);
}

void UZomGameInstance::OpenMap(const TSoftObjectPtr<UWorld>& Map)
{
	if (Map.IsNull())
	{
		UE_LOG(LogZomGame, Error, TEXT("Menu travel target not set on the game instance (MainMenuMap/GameplayMap)."));
		return;
	}

	UGameplayStatics::OpenLevelBySoftObjectPtr(this, Map);
}

// =========================================
// Settings
// =========================================

void UZomGameInstance::ApplyAudioSettings()
{
	const UZomGameUserSettings* Settings = UZomGameUserSettings::Get();
	if (!Settings || !GetWorld())
	{
		return;
	}

	auto ApplyVolume = [this](USoundControlBus* Bus, float Volume)
	{
		if (Bus)
		{
			UAudioModulationStatics::SetGlobalBusMixValue(this, Bus, Volume);
		}
	};

	ApplyVolume(MasterVolumeBus, Settings->MasterVolume);
	ApplyVolume(MusicVolumeBus, Settings->MusicVolume);
	ApplyVolume(SFXVolumeBus, Settings->SFXVolume);
	ApplyVolume(DialogueVolumeBus, Settings->DialogueVolume);
}
