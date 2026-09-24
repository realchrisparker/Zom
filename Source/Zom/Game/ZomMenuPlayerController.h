// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ZomMenuPlayerController.generated.h"


class UInputMappingContext;


/**
 * Player controller for front-end levels (L_MainMenu), set on BP_GameMode_Menu. Possesses no pawn and binds no
 * gameplay actions - it only installs MenuMappingContext in place of AZomPlayerController's IMC_InGame.
 * Menu screens themselves still come from AZomHUD (BP_HUD_Menu).
 */
UCLASS(Blueprintable, meta=(DisplayName="Zom Menu Player Controller"))
class ZOM_API AZomMenuPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	// Called to bind functionality to input
	virtual void SetupInputComponent() override;

	// Enhanced Input mapping context for the front end (e.g. IMC_Menu).
	UPROPERTY(EditDefaultsOnly, Category = "Zom|Input")
	TObjectPtr<UInputMappingContext> MenuMappingContext;
};
