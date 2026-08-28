// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Characters/Animations/Base/ZomAnimInstanceBase.h"
#include "Zom/Characters/ZomPlayerCharacter.h"
#include "Zom/Characters/Components/ZomCharacterMovementComponent.h"
#include "BlendStack/BlendStackAnimNodeLibrary.h"


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

	UpdateTrajectory(DeltaSeconds);

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

	CombatState = Character->CombatState;

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

	// Orientation intent: simply the character's current facing
	OrientationIntent = CharacterTransform.Rotator();
}

// Updates the predicted trajectory used by motion matching. Must run first in NativeUpdateAnimation, before anything else consumes this frame's trajectory.
void UZomAnimInstanceBase::UpdateTrajectory(float DeltaSeconds)
{
	// Use the Moving trajectory model once there's any current speed, otherwise Idle
	const FPoseSearchTrajectoryData& CurrentTrajectoryData = (Speed2D > 0.0f) ? TrajectoryGenerationData_Moving : TrajectoryGenerationData_Idle;

	// Update trajectory history and generate a fresh history + prediction based on current character intent.
	// Trajectory (history) and PreviousDesiredControllerYaw are updated in place; GeneratedTrajectory is the full result.
	FTransformTrajectory GeneratedTrajectory;
	UPoseSearchTrajectoryLibrary::PoseSearchGenerateTransformTrajectory(
		this,
		CurrentTrajectoryData,
		DeltaSeconds,
		Trajectory,
		PreviousDesiredControllerYaw,
		GeneratedTrajectory,
		/*InHistorySamplingInterval*/ -1.0f,
		/*InTrajectoryHistoryCount*/ 30,
		/*InPredictionSamplingInterval*/ 0.1f,
		/*InTrajectoryPredictionCount*/ 15);

	// Apply gravity and floor/obstacle collision to the predicted trajectory (mainly relevant while in the air)
	FTransformTrajectory CollisionAdjustedTrajectory;
	UPoseSearchTrajectoryLibrary::HandleTransformTrajectoryWorldCollisions(
		this,
		this,
		GeneratedTrajectory,
		/*bApplyGravity*/ true,
		/*FloorCollisionsOffset*/ 0.01f,
		CollisionAdjustedTrajectory,
		TrajectoryCollision,
		UEngineTypes::ConvertToTraceType(ECC_Visibility),
		/*bTraceComplex*/ false,
		TArray<AActor*>(),
		EDrawDebugTrace::None,
		/*bIgnoreSelf*/ true,
		/*MaxObstacleHeight*/ 150.0f);

	Trajectory = CollisionAdjustedTrajectory;

	// Sample predicted velocities at fixed points along the trajectory
	UPoseSearchTrajectoryLibrary::GetTransformTrajectoryVelocity(Trajectory, -0.3f, -0.2f, Trj_PastVelocity);
	UPoseSearchTrajectoryLibrary::GetTransformTrajectoryVelocity(Trajectory, 0.0f, 0.2f, Trj_CurrentVelocity);
	UPoseSearchTrajectoryLibrary::GetTransformTrajectoryVelocity(Trajectory, 0.4f, 0.5f, Trj_FutureVelocity);
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

/**
 * Whether the character is moving: the current velocity and acceleration are both non-zero.
 */
bool UZomAnimInstanceBase::IsMoving() const
{
	if (Velocity.IsNearlyZero() || Acceleration.IsNearlyZero()) return false;

	return true;
}

/**
 * Whether the character is starting to move: the predicted future trajectory speed is significantly higher
 * than the current speed, and the currently selected database isn't already a pivot/transition database.
 */
bool UZomAnimInstanceBase::IsStarting() const
{
	constexpr float StartingVelocityDeltaThreshold = 100.0f;

	const bool bAccelerating = Trj_FutureVelocity.Size2D() >= (Velocity.Size2D() + StartingVelocityDeltaThreshold);
	const bool bHasPivotsTag = CurrentDatabaseTags.Contains(FName("Pivots"));

	return IsMoving() && bAccelerating && !bHasPivotsTag;
}

bool UZomAnimInstanceBase::IsPivoting() const
{
	float PivotTurnAngleThreshold;

	switch (RotationMode)
	{
	case ERotationMode::Strafe:
		PivotTurnAngleThreshold = 30.0f;
		break;
	case ERotationMode::Aim:
		PivotTurnAngleThreshold = 0.0f;
		break;
	case ERotationMode::OrientToMovement:
	default:
		PivotTurnAngleThreshold = 45.0f;
		break;
	}

	// GetTrajectoryTurnAngle() compares Velocity against InputAcceleration; with no input, InputAcceleration is
	// zero and FVector::Rotation() on a zero vector returns Yaw 0, producing a bogus angle rather than "no turn".
	const bool bHasInputAcceleration = !InputAcceleration.IsNearlyZero();
	const bool bPivoting = bHasInputAcceleration && (FMath::Abs(GetTrajectoryTurnAngle()) >= PivotTurnAngleThreshold);

	return bPivoting && IsMoving();
}

/**
 * Whether the character's rotation has diverged enough from the animation root's rotation, at high enough
 * speed, to warrant a spin transition, and the currently selected database isn't already a pivot/transition database.
 */
bool UZomAnimInstanceBase::ShouldSpinTransition() const
{
	constexpr float SpinYawThreshold = 130.0f;
	constexpr float SpinSpeedThreshold = 150.0f;

	const float YawDelta = FMath::FindDeltaAngleDegrees(RootTransform.Rotator().Yaw, CharacterTransform.Rotator().Yaw);
	const bool bHasPivotsTag = CurrentDatabaseTags.Contains(FName("Pivots"));

	return (FMath::Abs(YawDelta) >= SpinYawThreshold) && (Speed2D >= SpinSpeedThreshold) && !bHasPivotsTag;
}

/**
 * Whether the character should turn in place: the orientation intent has diverged enough from the animation
 * root's rotation, and the character either wants to aim or has just come to a stop from moving.
 */
bool UZomAnimInstanceBase::ShouldTurnInPlace() const
{
	constexpr float TurnInPlaceYawThreshold = 50.0f;

	const float YawDelta = FMath::FindDeltaAngleDegrees(RootTransform.Rotator().Yaw, OrientationIntent.Yaw);

	const bool bWantsToAim = RotationMode == ERotationMode::Aim;
	const bool bJustBecameIdle = (MovementState == EMovementState::Idle) && (MovementState_LastFrame == EMovementState::Moving);

	return (FMath::Abs(YawDelta) >= TurnInPlaceYawThreshold) && (bWantsToAim || bJustBecameIdle);
}

/**
 * The yaw angle between the direction the character is accelerating towards and its current direction of travel.
 */
float UZomAnimInstanceBase::GetTrajectoryTurnAngle() const
{
	return FMath::FindDeltaAngleDegrees(Velocity.Rotation().Yaw, InputAcceleration.Rotation().Yaw);
}

// Whether the character just landed with a vertical land speed below the heavy land threshold
bool UZomAnimInstanceBase::JustLanded_Light() const
{
	return bJustLanded && (FMath::Abs(LandVelocity.Z) < FMath::Abs(HeavyLandSpeedThreshold));
}

// Whether the character just landed with a vertical land speed at or above the heavy land threshold
bool UZomAnimInstanceBase::JustLanded_Heavy() const
{
	return bJustLanded && (FMath::Abs(LandVelocity.Z) >= FMath::Abs(HeavyLandSpeedThreshold));
}

/**
 * Whether the character just traversed: no traversal montage is currently playing on the traversal slot, the
 * "MovingTraversal" curve is active, and the trajectory turn angle is shallow enough to still be moving forward.
 */
bool UZomAnimInstanceBase::JustTraversed() const
{
	constexpr float TraversalTurnAngleThreshold = 50.0f;

	const bool bSlotInactive = !IsSlotActive(FName("DefaultSlot"));
	const bool bMovingTraversalActive = GetCurveValue(FName("MovingTraversal")) > 0.0f;
	const bool bTurnAngleShallow = FMath::Abs(GetTrajectoryTurnAngle()) <= TraversalTurnAngleThreshold;

	return bSlotInactive && bMovingTraversalActive && bTurnAngleShallow;
}

/**
 * Whether root motion can currently be steered: the character is moving or in the air (steering idle animations
 * can cause them to slide), and the blend stack's currently active anim (given by Node) is active.
 */
bool UZomAnimInstanceBase::EnableSteering(const FAnimNodeReference& Node) const
{
	const bool bMovingOrInAir = (MovementState != EMovementState::Idle) || (MovementMode != ECharacterMovementMode::OnGround);

	return bMovingOrInAir && UBlendStackAnimNodeLibrary::GetCurrentBlendStackAnimIsActive(Node);
}

/**
 * The steering node's target rotation: the predicted trajectory's facing half a second into the future, so
 * steering rotates towards where the character is headed rather than lagging behind its current rotation.
 */
FQuat UZomAnimInstanceBase::GetDesiredFacing() const
{
	return Trajectory.GetSampleAtTime(0.5f).Facing;
}

/**
 * The warping space Orientation Warping should use: the root bone transform when Offset Root Bone is enabled
 * (since that lets the root bone and component transforms diverge), otherwise the component transform.
 */
EOrientationWarpingSpace UZomAnimInstanceBase::GetOrientationWarpingWarpingSpace() const
{
	return bOffsetRootBoneEnabled ? EOrientationWarpingSpace::RootBoneTransform : EOrientationWarpingSpace::ComponentTransform;
}

/**
 * The Motion Matching node's blend time: shorter right after landing (OnGround, was InAir) so land animations
 * blend in quickly, very short right after jumping (InAir, moving up fast) for a snappy jump start, 0.5s otherwise.
 */
float UZomAnimInstanceBase::GetMMBlendTime() const
{
	switch (MovementMode)
	{
	case ECharacterMovementMode::OnGround:
		switch (MovementMode_LastFrame)
		{
		case ECharacterMovementMode::OnGround:
			return 0.5f;
		case ECharacterMovementMode::InAir:
			// Just landed: blend the land animation in faster
			return 0.2f;
		default:
			break;
		}
		break;

	case ECharacterMovementMode::InAir:
		// Moving up quickly means we just jumped: blend the jump animation in very fast
		return (Velocity.Z > 100.0f) ? 0.15f : 0.5f;

	default:
		break;
	}

	return 0.2f;
}

/**
 * The Motion Matching node's notify recency time out, by Gait. Must stay larger than the time between footstep
 * notifies for that gait, otherwise notifies get filtered out as stale.
 */
float UZomAnimInstanceBase::GetMMNotifyRecencyTimeOut() const
{
	switch (Gait)
	{
	case EGait::Walk:
		return 0.2f;
	case EGait::Run:
		return 0.2f;
	case EGait::Sprint:
		return 0.16f;
	}

	return 0.2f;
}

/**
 * The Motion Matching node's continuing-pose interrupt mode: interrupts the database search (allowing a
 * database change) when the character's high level state has meaningfully changed since last frame.
 */
EPoseSearchInterruptMode UZomAnimInstanceBase::GetMMInterruptMode() const
{
	// Always interrupt if the movement mode has changed, since that usually means the character is jumping or landing
	const bool bMovementModeChanged = (MovementMode != MovementMode_LastFrame);
	if (bMovementModeChanged)
	{
		return EPoseSearchInterruptMode::InterruptOnDatabaseChange; //Quick out if the movement mode changed.
	}

	// ------
	// From GASP. This is the check from the tripple OR.
	// ------

	// 1.A Movement state changed (Idle <-> Moving)
	const bool bMovementStateChangedFinal = (MovementState != MovementState_LastFrame);

	// 1.B Gait changed while moving (Idle <-> Moving doesn't count)
	const bool bGaitChanged = (Gait != Gait_LastFrame);
	const bool bMovementStateIsMoving = (MovementState == EMovementState::Moving);
	const bool bGaitChangedWhileMovingFinal = bGaitChanged && bMovementStateIsMoving;

	// 1.C Stance changed (Stand <-> Crouch)
	const bool bStanceChangedFinal = (Stance != Stance_LastFrame);

	// 1.D FINAL
	const bool bStateOrGaitChangedFinal = bMovementStateChangedFinal || bGaitChangedWhileMovingFinal || bStanceChangedFinal;

	// ------
	// From GASP. This is the check from the first AND.
	// ------

	// 2.A Movement mode changed (OnGround <-> InAir)
	const bool bMovementModeIsOnGround = (MovementMode == ECharacterMovementMode::OnGround);

	// 2.B FINAL
	const bool bMovementModeOnGroundTripleOrFinal = (bStateOrGaitChangedFinal && bMovementModeIsOnGround);

	// 3.A FINAL. This is the check from the last OR.
	if (bMovementModeChanged || bMovementModeOnGroundTripleOrFinal) return EPoseSearchInterruptMode::InterruptOnDatabaseChange;

	// Default to not interrupt if none of the above conditions are met
	return EPoseSearchInterruptMode::DoNotInterrupt;
}