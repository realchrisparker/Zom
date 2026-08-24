// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ZomPlayerController.generated.h"


// Forward declarations

class AZomPlayerCharacter;


/**
 * 
 */
UCLASS(Blueprintable, meta=(DisplayName="Zom Player Controller"))
class ZOM_API AZomPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AZomPlayerController();

	// -------------
	// Functions
	// -------------

	// Returns the cached player character possessed by this controller
	UFUNCTION(BlueprintCallable, Category = "Zom", meta = (DisplayName = "Get Cached Player Character"))
	AZomPlayerCharacter* GetCachedPlayerCharacter() const { return CachedPlayerCharacter.Get(); }

	// -------------
	// Properties
	// -------------

protected:

	// -------------
	// Functions
	// -------------

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called to bind functionality to input
	virtual void SetupInputComponent() override;

	// Called when this controller possesses a pawn
	virtual void OnPossess(APawn* InPawn) override;

	// Called when this controller unpossesses a pawn
	virtual void OnUnPossess() override;

private:

	// -------------
	// Properties
	// -------------
	
	// Cached reference to the player character possessed by this controller
	TWeakObjectPtr<AZomPlayerCharacter> CachedPlayerCharacter;
};
