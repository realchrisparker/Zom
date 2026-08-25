// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Zom/Characters/Enums/ZomCharacterEnums.h"
#include "ZomCharacterMovementComponent.generated.h"


// Forward declaration

class AZomPlayerCharacter;


/**
 * Custom character movement component for handling player-specific locomotion and movement logic.
 */
UCLASS()
class ZOM_API UZomCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UZomCharacterMovementComponent();

	// -------------
	// Functions
	// -------------

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Returns the max speed for the current movement state
	virtual float GetMaxSpeed() const override;

	// Called before the character's movement is processed each tick
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;

	// Called after the character's movement is processed each tick
	virtual void UpdateCharacterStateAfterMovement(float DeltaSeconds) override;

	// Called when the movement mode changes (e.g. walking, falling, custom modes)
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

	// Handles any custom movement modes
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;

	// Called when the character lands on a walkable surface after falling
	virtual void ProcessLanded(const FHitResult& Hit, float remainingTime, int32 Iterations) override;

	// Returns whether the character can crouch in its current state
	virtual bool CanCrouchInCurrentState() const override;

	// Called when the owning character is possessed by a controller
	void OnCharacterPossessed(AController* NewController);

	// Called when the owning character is unpossessed by its controller
	void OnCharacterUnPossessed();

	// -------------
	// Properties
	// -------------

	// Current rotation mode, used to drive how the component orients the character while moving
	UPROPERTY(BlueprintReadWrite, Category = "Zom|Movement State", meta = (DisplayName = "Rotation Mode"))
	ERotationMode RotationMode = ERotationMode::OrientToMovement;

	// Max speed while Gait is Walk
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Zom|Movement State", meta = (DisplayName = "Walk Speed"))
	float WalkSpeed = 175.0f;

	// Max speed while Gait is Run
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Zom|Movement State", meta = (DisplayName = "Run Speed"))
	float RunSpeed = 375.0f;

	// Max speed while Gait is Sprint
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Zom|Movement State", meta = (DisplayName = "Sprint Speed"))
	float SprintSpeed = 600.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Movement", meta = (DisplayName = "Just Landed"))
	bool bJustLanded = false;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Movement", meta = (DisplayName = "Land Velocity"))
	FVector LandVelocity = FVector::ZeroVector;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:

	// Owning character of this movement component.
	UPROPERTY()
	TObjectPtr<AZomPlayerCharacter> OwningCharacter;
};
