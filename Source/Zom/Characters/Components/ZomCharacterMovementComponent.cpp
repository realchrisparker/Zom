// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Characters/Components/ZomCharacterMovementComponent.h"
#include "Zom/Characters/Base/ZomCharacterBase.h"


// Sets default values for this component's properties
UZomCharacterMovementComponent::UZomCharacterMovementComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UZomCharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void UZomCharacterMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	// bJustLanded should only be true for the tick in which ProcessLanded fires below
	bJustLanded = false;

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

// Called when the owning character is possessed by a controller
void UZomCharacterMovementComponent::OnCharacterPossessed(AController* NewController)
{
	if (!OwningCharacter)
	{
		OwningCharacter = Cast<AZomCharacterBase>(GetOwner());
	}
}

// Called when the owning character is unpossessed by its controller
void UZomCharacterMovementComponent::OnCharacterUnPossessed()
{
	// Clear the reference to the owning character when unpossessed
	OwningCharacter = nullptr;
}

// Returns the max speed for the current movement state
float UZomCharacterMovementComponent::GetMaxSpeed() const
{
	if (MovementMode == MOVE_Walking || MovementMode == MOVE_NavWalking)
	{
		if (IsCrouching())
		{
			return MaxWalkSpeedCrouched;
		}

		const AZomCharacterBase* ZomCharacterOwner = Cast<AZomCharacterBase>(GetCharacterOwner());

		if (ZomCharacterOwner)
		{
			switch (ZomCharacterOwner->Gait)
			{
			case EGait::Walk:
				return WalkSpeed;
			case EGait::Run:
				return RunSpeed;
			case EGait::Sprint:
				return SprintSpeed;
			}
		}
	}

	return Super::GetMaxSpeed();
}

// Called before the character's movement is processed each tick
void UZomCharacterMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);

	switch (RotationMode)
	{
	case ERotationMode::OrientToMovement:
		bOrientRotationToMovement = true;
		bUseControllerDesiredRotation = false;
		break;
	case ERotationMode::Strafe:
	case ERotationMode::Aim:
		bOrientRotationToMovement = false;
		bUseControllerDesiredRotation = true;
		break;
	}
}

// Called when the movement mode changes (e.g. walking, falling, custom modes)
void UZomCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

}

// Handles any custom movement modes
void UZomCharacterMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	Super::PhysCustom(deltaTime, Iterations);

}

// Called when the character lands on a walkable surface after falling
void UZomCharacterMovementComponent::ProcessLanded(const FHitResult& Hit, float remainingTime, int32 Iterations)
{
	// Capture the impact velocity before Super changes movement mode / modifies Velocity
	bJustLanded = true;
	LandVelocity = Velocity;

	Super::ProcessLanded(Hit, remainingTime, Iterations);
}

// Returns whether the character can crouch in its current state
bool UZomCharacterMovementComponent::CanCrouchInCurrentState() const
{
	if (!Super::CanCrouchInCurrentState())
	{
		return false;
	}

	// The speed check below only gates *entering* crouch: 20 uu/s matches the "stopped" threshold the motion
	// matching Chooser tables use to select stop animations, so crouch only starts once the character has a
	// stopped pose to transition from. This same function is also polled every tick by
	// UpdateCharacterStateBeforeMovement to decide whether to auto-uncrouch, so once already crouched we must
	// skip the speed check - otherwise crouch-walking above the threshold would immediately force an uncrouch
	// the tick after crouch starts.
	return IsCrouching() || Velocity.Size2D() < 20.0f;
}
