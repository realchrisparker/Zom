// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ZomPlayerCharacterBase.generated.h"


UCLASS(Blueprintable, meta=(DisplayName="Zom Player Character Base"))
class ZOM_API AZomPlayerCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:

	// Sets default values for this character's properties
	AZomPlayerCharacterBase();

	// -------------
	// Functions
	// -------------

	// -------------
	// Properties
	// -------------

protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	// Add any private member variables or functions here
};
