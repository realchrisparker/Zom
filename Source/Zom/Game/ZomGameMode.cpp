// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Game/ZomGameMode.h"
#include "Zom/Characters/ZomPlayerCharacter.h"
#include "Zom/Characters/ZomPlayerController.h"
#include "Zom/Levels/ZomCheckpoint.h"
#include "Zom/Game/ZomGameSession.h"
#include "Zom/Game/ZomGameState.h"
#include "Zom/Game/ZomHUD.h"
#include "Zom/Game/ZomPlayerState.h"
#include "Zom/Game/ZomSaveGame.h"
#include "Zom/Objectives/ZomObjectiveSubsystem.h"
#include "Zom/Abilities/Effects/ZomGE_RestoreFromSave.h"
#include "Zom/Misc/ZomGameplayTags.h"
#include "Zom/Misc/ZomLogChannels.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemComponent.h"


const FString AZomGameMode::SaveSlotName = TEXT("ZomSaveSlot");

AZomGameMode::AZomGameMode()
{
	DefaultPawnClass = AZomPlayerCharacter::StaticClass();
	PlayerControllerClass = AZomPlayerController::StaticClass();
	PlayerStateClass = AZomPlayerState::StaticClass();
	GameStateClass = AZomGameState::StaticClass();
	GameSessionClass = AZomGameSession::StaticClass();
	HUDClass = AZomHUD::StaticClass();
}

// Called when the game starts or when spawned
void AZomGameMode::BeginPlay()
{
	Super::BeginPlay();
}

void AZomGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
	{
		LoadedSaveGame = Cast<UZomSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
	}
}

AActor* AZomGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	const EZomCheckpointID TargetCheckpointID = LoadedSaveGame ? LoadedSaveGame->CheckpointID : EZomCheckpointID::Entry;

	for (TActorIterator<AZomCheckpoint> It(GetWorld()); It; ++It)
	{
		if (It->CheckpointID == TargetCheckpointID)
		{
			return *It;
		}
	}

	// No checkpoint placed for the target ID yet (e.g. level under construction) - fall back to the engine default.
	return Super::ChoosePlayerStart_Implementation(Player);
}

void AZomGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (!LoadedSaveGame)
	{
		return;
	}

	const AZomPlayerCharacterBase* PlayerCharacter = NewPlayer ? Cast<AZomPlayerCharacterBase>(NewPlayer->GetPawn()) : nullptr;
	UAbilitySystemComponent* ASC = PlayerCharacter ? PlayerCharacter->GetAbilitySystemComponent() : nullptr;

	if (ASC)
	{
		FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
		const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(UZomGE_RestoreFromSave::StaticClass(), 1.f, EffectContext);
		if (SpecHandle.IsValid())
		{
			SpecHandle.Data->SetSetByCallerMagnitude(TAG_Zom_SetByCaller_RestoreHealth.GetTag(), LoadedSaveGame->SavedHealth);
			SpecHandle.Data->SetSetByCallerMagnitude(TAG_Zom_SetByCaller_RestoreStamina.GetTag(), LoadedSaveGame->SavedStamina);
			ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
	else
	{
		UE_LOG(LogZomGame, Warning, TEXT("Loaded save but the player's ASC wasn't available yet in PostLogin - Health/Stamina not restored."));
	}

	if (UZomObjectiveSubsystem* ObjectiveSubsystem = GetGameInstance()->GetSubsystem<UZomObjectiveSubsystem>())
	{
		// Read explicitly every time rather than trusting in-memory state, per Section 11.
		ObjectiveSubsystem->RestoreStep(LoadedSaveGame->ObjectiveStep);
	}
}
