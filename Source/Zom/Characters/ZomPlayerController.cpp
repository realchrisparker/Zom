// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Characters/ZomPlayerController.h"
#include "Zom/Characters/ZomPlayerCharacter.h"


AZomPlayerController::AZomPlayerController()
{

}

// Called when the game starts or when spawned
void AZomPlayerController::BeginPlay()
{
    Super::BeginPlay();

}

// Called to bind functionality to input
void AZomPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

}

void AZomPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    // Cache the player character reference if the possessed pawn is a player character
    CachedPlayerCharacter = Cast<AZomPlayerCharacter>(InPawn);
}

void AZomPlayerController::OnUnPossess()
{
    Super::OnUnPossess();

    // Clear the cached player character reference
    CachedPlayerCharacter = nullptr;
}