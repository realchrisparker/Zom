// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Game/ZomGameMode.h"
#include "Zom/Characters/ZomPlayerCharacter.h"
#include "Zom/Characters/ZomPlayerController.h"
#include "Zom/Game/ZomGameSession.h"
#include "Zom/Game/ZomGameState.h"
#include "Zom/Game/ZomHUD.h"
#include "Zom/Game/ZomPlayerState.h"
#include "Zom/Misc/ZomLogChannels.h"
#include "Engine/Engine.h"


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
