// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Game/ZomMenuPlayerController.h"
#include "EnhancedInputSubsystems.h"


// Called to bind functionality to input
void AZomMenuPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Same place AZomPlayerController adds IMC_InGame, so the two controllers swap contexts symmetrically.
	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (MenuMappingContext)
			{
				InputSubsystem->AddMappingContext(MenuMappingContext, 0);
			}
		}
	}
}
