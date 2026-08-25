// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Characters/Animations/Base/ZomAnimInstanceBase.h"
#include "Zom/Characters/ZomPlayerCharacter.h"
#include "Zom/Characters/Components/ZomCharacterMovementComponent.h"


// Constructor
UZomAnimInstanceBase::UZomAnimInstanceBase()
{
}

// Called when the anim instance is created and its owning component/actor are valid; good place to cache references
void UZomAnimInstanceBase::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	// Cache the player character reference if the owning actor is a player character
	CachedPlayerCharacter = Cast<AZomPlayerCharacter>(TryGetPawnOwner());

	// Cache the character movement component reference if the player character is valid
	CachedCharacterMovementComponent = CachedPlayerCharacter.IsValid() ? Cast<UZomCharacterMovementComponent>(CachedPlayerCharacter->GetCharacterMovement()) : nullptr;

	bHasOwningActor = CachedPlayerCharacter.IsValid();
}

// Called when the anim instance is being uninitialized (e.g. anim class changing, owning component being destroyed); good place to clear cached references
void UZomAnimInstanceBase::NativeUninitializeAnimation()
{
	Super::NativeUninitializeAnimation();

	// Clear cached references to avoid dangling pointers

	CachedPlayerCharacter = nullptr;
	CachedCharacterMovementComponent = nullptr;

	bHasOwningActor = false;
}

// Called every frame on the game thread before the animation graph is updated
void UZomAnimInstanceBase::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	bHasOwningActor = CachedPlayerCharacter.IsValid();

	if (!bHasOwningActor || !CachedCharacterMovementComponent.IsValid())
	{
		return;
	}

	AZomPlayerCharacter* Character = CachedPlayerCharacter.Get();
	UZomCharacterMovementComponent* MovementComponent = CachedCharacterMovementComponent.Get();

	// Transforms
	CharacterTransform_LastFrame = CharacterTransform;
	CharacterTransform = Character->GetActorTransform();
	ActorTransform = CharacterTransform;

	if (USkeletalMeshComponent* SkelMeshComponent = GetSkelMeshComponent())
	{
		RootTransform = SkelMeshComponent->GetBoneTransform(0);
	}

	// Velocity / acceleration inputs
	Velocity_LastFrame = Velocity;
	Velocity = MovementComponent->Velocity;

	InputAcceleration = MovementComponent->GetCurrentAcceleration();
	CurrentMaxAcceleration = MovementComponent->GetMaxAcceleration();
	CurrentMaxDeceleration = MovementComponent->GetMaxBrakingDeceleration();

	// Movement state pass-through from the character (Gait) and movement component (everything else)
	Gait_LastFrame = Gait;
	Gait = Character->Gait;

	RotationMode_LastFrame = RotationMode;
	RotationMode = MovementComponent->RotationMode;

	Stance_LastFrame = Stance;
	Stance = MovementComponent->IsCrouching() ? EStance::Crouch : EStance::Stand;

	CachedNativeMovementMode = MovementComponent->MovementMode;

	// Landing state, sourced from the movement component (set precisely in ProcessLanded)
	bJustLanded = MovementComponent->bJustLanded;
	LandVelocity = MovementComponent->LandVelocity;

	// Aiming
	AimingRotation = Character->GetBaseAimRotation();

	// Ground info (only valid while walking; holds the last walkable floor result otherwise)
	if (MovementComponent->CurrentFloor.IsWalkableFloor())
	{
		GroundNormal = MovementComponent->CurrentFloor.HitResult.ImpactNormal;
		GroundLocation = MovementComponent->CurrentFloor.HitResult.ImpactPoint;
	}
}

// Called every frame, potentially on a worker thread; only safe to read data here, not to modify UObjects
void UZomAnimInstanceBase::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	if (!bHasOwningActor)
	{
		return;
	}

	// Speed / velocity state
	Speed2D = Velocity.Size2D();
	bHasVelocity = !Velocity.IsNearlyZero();

	if (bHasVelocity)
	{
		LastNonZeroVelocity = Velocity;
	}

	MovementState_LastFrame = MovementState;
	MovementState = bHasVelocity ? EMovementState::Moving : EMovementState::Idle;

	// High level movement mode, derived from the native mode gathered on the game thread
	MovementMode_LastFrame = MovementMode;
	MovementMode = MapNativeMovementMode(CachedNativeMovementMode);

	// Acceleration derived from the change in velocity over time
	Acceleration_LastFrame = Acceleration;
	Acceleration = (DeltaSeconds > KINDA_SMALL_NUMBER) ? (Velocity - Velocity_LastFrame) / DeltaSeconds : FVector::ZeroVector;

	bHasAcceleration = !Acceleration.IsNearlyZero();
	AccelerationAmount = Acceleration.Size();
	RelativeAcceleration = CharacterTransform.InverseTransformVectorNoScale(Acceleration);
	VelocityAcceleration = bHasVelocity ? Acceleration.ProjectOnToNormal(Velocity.GetSafeNormal()) : FVector::ZeroVector;

	// Directional facing of movement relative to the character's forward direction
	if (bHasVelocity)
	{
		const float Angle = FMath::FindDeltaAngleDegrees(CharacterTransform.Rotator().Yaw, Velocity.Rotation().Yaw);

		if (Angle >= -30.0f && Angle <= 30.0f)
		{
			MovementDirection = EMovementDirection::F;
		}
		else if (Angle < -150.0f || Angle > 150.0f)
		{
			MovementDirection = EMovementDirection::B;
		}
		else if (Angle < -90.0f)
		{
			MovementDirection = EMovementDirection::LL;
		}
		else if (Angle < 0.0f)
		{
			MovementDirection = EMovementDirection::LR;
		}
		else if (Angle < 90.0f)
		{
			MovementDirection = EMovementDirection::RL;
		}
		else
		{
			MovementDirection = EMovementDirection::RR;
		}
	}

	// Orientation intent: face the aim direction while strafing/aiming, otherwise face the movement direction
	if (RotationMode == ERotationMode::Strafe || RotationMode == ERotationMode::Aim)
	{
		OrientationIntent = FRotator(0.0f, AimingRotation.Yaw, 0.0f);
	}
	else
	{
		OrientationIntent = bHasVelocity ? FRotator(0.0f, Velocity.Rotation().Yaw, 0.0f) : FRotator(0.0f, CharacterTransform.Rotator().Yaw, 0.0f);
	}
}

ECharacterMovementMode UZomAnimInstanceBase::MapNativeMovementMode(TEnumAsByte<EMovementMode> NativeMode) const
{
	switch (NativeMode)
	{
	case MOVE_None:
	case MOVE_Walking:
	case MOVE_NavWalking:
	case MOVE_Flying:
		return ECharacterMovementMode::OnGround;

	case MOVE_Falling:
	case MOVE_Swimming:
		return ECharacterMovementMode::InAir;

	case MOVE_Custom:
	default:
		// Sliding, Traversing and Ragdoll are driven by custom movement modes that aren't implemented yet, so leave the current value unchanged
		return MovementMode;
	}
}

bool UZomAnimInstanceBase::IsMoving() const
{
	if (Velocity.IsNearlyZero() || Acceleration.IsNearlyZero()) return false;

	return true;
}

bool UZomAnimInstanceBase::IsStarting() const
{
	constexpr float StartingVelocityDeltaThreshold = 100.0f;

	const bool bAccelerating = Trj_FutureVelocity.Size2D() >= (Velocity.Size2D() + StartingVelocityDeltaThreshold);
	const bool bHasPivotsTag = CurrentDatabaseTags.Contains(FName("Pivots"));

	return IsMoving() && bAccelerating && !bHasPivotsTag;
}

bool UZomAnimInstanceBase::ShouldSpinTransition() const
{
	constexpr float SpinYawThreshold = 130.0f;
	constexpr float SpinSpeedThreshold = 150.0f;

	const float YawDelta = FMath::FindDeltaAngleDegrees(RootTransform.Rotator().Yaw, CharacterTransform.Rotator().Yaw);
	const bool bHasPivotsTag = CurrentDatabaseTags.Contains(FName("Pivots"));

	return (FMath::Abs(YawDelta) >= SpinYawThreshold) && (Speed2D >= SpinSpeedThreshold) && !bHasPivotsTag;
}

bool UZomAnimInstanceBase::JustLanded_Light() const
{
	return bJustLanded && (FMath::Abs(LandVelocity.Z) < FMath::Abs(HeavyLandSpeedThreshold));
}

bool UZomAnimInstanceBase::JustLanded_Heavy() const
{
	return bJustLanded && (FMath::Abs(LandVelocity.Z) >= FMath::Abs(HeavyLandSpeedThreshold));
}