// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Characters/Animations/Base/ZomAnimInstanceBase.h"
#include "Zom/Characters/Base/ZomCharacterBase.h"
#include "Zom/Characters/Components/ZomCharacterMovementComponent.h"
#include "BlendStack/BlendStackAnimNodeLibrary.h"


// Constructor
UZomAnimInstanceBase::UZomAnimInstanceBase()
{
	// Plant Settings (Default)
	PlantSettings_Default.SpeedThreshold = 1.0f;
	PlantSettings_Default.DistanceToGround = 10.0f;
	PlantSettings_Default.LockType = EFootPlacementLockType::PivotAroundBall;
	PlantSettings_Default.UnplantRadius = 20.0f;
	PlantSettings_Default.ReplantRadiusRatio = 0.2f;
	PlantSettings_Default.UnplantAngle = 60.0f;
	PlantSettings_Default.ReplantAngleRatio = 0.2f;
	PlantSettings_Default.MaxExtensionRatio = 0.5f;
	PlantSettings_Default.MinExtensionRatio = 0.2f;
	PlantSettings_Default.SeparatingDistance = 0.0f;
	PlantSettings_Default.UnalignmentSpeedThreshold = 100.0f;
	PlantSettings_Default.AnkleTwistReduction = 0.75f;
	PlantSettings_Default.bReconstructWorldPlantFromVelocity = false;
	PlantSettings_Default.bAdjustHeelBeforePlanting = false;

	// Interpolation Settings (Default)
	InterpolationSettings_Default.UnplantLinearStiffness = 100.0f;
	InterpolationSettings_Default.UnplantLinearDamping = 1.0f;
	InterpolationSettings_Default.UnplantAngularStiffness = 450.0f;
	InterpolationSettings_Default.UnplantAngularDamping = 1.0f;
	InterpolationSettings_Default.bEnableFloorInterpolation = true;
	InterpolationSettings_Default.FloorLinearStiffness = 1000.0f;
	InterpolationSettings_Default.FloorLinearDamping = 1.0f;
	InterpolationSettings_Default.FloorAngularStiffness = 450.0f;
	InterpolationSettings_Default.FloorAngularDamping = 1.0f;
	InterpolationSettings_Default.bSmoothRootBone = false;
	InterpolationSettings_Default.bEnableSeparationInterpolation = true;
	InterpolationSettings_Default.SeparationStiffness = 1000.0f;
	InterpolationSettings_Default.SeparationDamping = 1.0f;
}

// Called when the anim instance is created and its owning component/actor are valid; good place to cache references
void UZomAnimInstanceBase::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	// Cache the character reference if the owning actor is a Zom character
	CachedPlayerCharacter = Cast<AZomCharacterBase>(TryGetPawnOwner());

	// Cache the character movement component reference if the player character is valid
	CachedCharacterMovementComponent = CachedPlayerCharacter.IsValid() ? Cast<UZomCharacterMovementComponent>(CachedPlayerCharacter->GetCharacterMovement()) : nullptr;

	bHasOwningActor = CachedPlayerCharacter.IsValid();

	// Cache the AnimGraph's Offset Root Bone node, tagged "OffsetRoot", so NativeUpdateAnimation can read its simulated transform every frame without a tag lookup
	CachedOffsetRootBoneNode = nullptr;
	if (const IAnimClassInterface* AnimClassInterface = IAnimClassInterface::GetFromClass(GetClass()))
	{
		if (const FAnimSubsystem_Tag* TagSubsystem = AnimClassInterface->FindSubsystem<FAnimSubsystem_Tag>())
		{
			CachedOffsetRootBoneNode = TagSubsystem->FindNodeByTag<FAnimNode_OffsetRootBone>(FName("OffsetRoot"), this);
		}
	}
}

// Called when the anim instance is being uninitialized (e.g. anim class changing, owning component being destroyed); good place to clear cached references
void UZomAnimInstanceBase::NativeUninitializeAnimation()
{
	Super::NativeUninitializeAnimation();

	// Clear cached references to avoid dangling pointers

	CachedPlayerCharacter = nullptr;
	CachedCharacterMovementComponent = nullptr;
	CachedOffsetRootBoneNode = nullptr;

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

	AZomCharacterBase* Character = CachedPlayerCharacter.Get();
	UZomCharacterMovementComponent* MovementComponent = CachedCharacterMovementComponent.Get();

	UpdateTrajectory(DeltaSeconds);

	// Transforms
	CharacterTransform_LastFrame = CharacterTransform;
	CharacterTransform = Character->GetActorTransform();
	ActorTransform = CharacterTransform;

	// Root transform comes from the "OffsetRoot" Offset Root Bone node's simulated transform, carried over from the
	// previous frame's AnimGraph evaluation, with a 90 degree yaw correction for this skeleton's root bone orientation.
	// Falls back to the character transform if the tagged node couldn't be resolved (e.g. AnimGraph not yet initialized).
	if (CachedOffsetRootBoneNode)
	{
		FTransform OffsetRootTransform;
		CachedOffsetRootBoneNode->GetOffsetRootTransform(OffsetRootTransform);

		FRotator OffsetRootRotation = OffsetRootTransform.Rotator();
		OffsetRootRotation.Yaw += 90.0f;

		RootTransform = FTransform(OffsetRootRotation, OffsetRootTransform.GetLocation(), FVector::OneVector);
	}
	else
	{
		RootTransform = CharacterTransform;
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
	Stance = Character->Stance;

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

	// Tracks how long it's been since Speed2D last met SpinSpeedThreshold, so ShouldSpinTransition() can use a
	// short grace window instead of an instantaneous speed check (see its comment for why).
	TimeSinceHighSpeed = (Speed2D >= SpinSpeedThreshold) ? 0.0f : (TimeSinceHighSpeed + DeltaSeconds);

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
 * Whether the character's rotation has diverged enough from the animation root's rotation, having recently been
 * moving fast enough, to warrant a spin transition, and the currently selected database isn't already a
 * pivot/transition database.
 */
bool UZomAnimInstanceBase::ShouldSpinTransition() const
{
	constexpr float SpinYawThreshold = 130.0f;

	// A sharp turn that's also decelerating to a stop can drop Speed2D below SpinSpeedThreshold before the yaw
	// delta below climbs past SpinYawThreshold. Without this grace window that race would let the divergence go
	// uncaught here, growing unnoticed until MovementState finally goes Idle and ShouldTurnInPlace() catches it -
	// visible as the legs snapping through a near-180 degree turn-in-place well after the character stopped.
	constexpr float HighSpeedGraceWindow = 0.3f;

	const float YawDelta = FMath::FindDeltaAngleDegrees(RootTransform.Rotator().Yaw, CharacterTransform.Rotator().Yaw);
	const bool bHasPivotsTag = CurrentDatabaseTags.Contains(FName("Pivots"));
	const bool bWasRecentlyFastEnough = TimeSinceHighSpeed <= HighSpeedGraceWindow;

	return (FMath::Abs(YawDelta) >= SpinYawThreshold) && bWasRecentlyFastEnough && !bHasPivotsTag;
}

/**
 * Whether the character should turn in place: the orientation intent has diverged enough from the animation
 * root's rotation, and the character is idle while strafing (e.g. the aim offset has been rotated past its
 * clamped range and can no longer absorb the difference on its own).
 */
bool UZomAnimInstanceBase::ShouldTurnInPlace() const
{
	constexpr float TurnInPlaceYawThreshold = 50.0f;

	const float YawDelta = FMath::FindDeltaAngleDegrees(RootTransform.Rotator().Yaw, OrientationIntent.Yaw);

	// Matches EnableAO()'s bIsStrafing check: the aim offset (and therefore the yaw delta it can't absorb) is
	// only ever produced while strafing, so that's the mode this needs to catch, not Aim.
	const bool bIsStrafing = RotationMode == ERotationMode::Strafe;
	const bool bIsIdle = MovementState == EMovementState::Idle;

	return (FMath::Abs(YawDelta) >= TurnInPlaceYawThreshold) && bIsStrafing && bIsIdle;
}

/**
 * The yaw angle between the direction the character is accelerating towards and its current direction of travel.
 */
float UZomAnimInstanceBase::GetTrajectoryTurnAngle() const
{
	return FMath::FindDeltaAngleDegrees(Velocity.Rotation().Yaw, InputAcceleration.Rotation().Yaw);
}

// The character's current acceleration relative to its facing, normalized by the max acceleration/deceleration for whichever is currently relevant.
FVector UZomAnimInstanceBase::CalculateRelativeAccelerationAmount() const
{
	if (FVector::DotProduct(Acceleration, Velocity) > 0.0f)
	{
		const float ClampedMaxAcceleration = FMath::Max(CurrentMaxAcceleration, 1.0f);
		return CharacterTransform.InverseTransformVectorNoScale(Acceleration.GetClampedToMaxSize(ClampedMaxAcceleration)) / ClampedMaxAcceleration;
	}

	const float ClampedMaxDeceleration = FMath::Max(CurrentMaxDeceleration, 1.0f);
	return CharacterTransform.InverseTransformVectorNoScale(Acceleration.GetClampedToMaxSize(ClampedMaxDeceleration)) / ClampedMaxDeceleration;
}

/**
 * The 2D lean amount used to drive lean/tilt animations: X is lateral lean, driven by the lateral (Y) component
 * of the relative acceleration amount and scaled up as speed ramps from 165 to 375. Y is currently unused.
 */
FVector2D UZomAnimInstanceBase::GetLeanAmount() const
{
	const float SpeedScale = FMath::GetMappedRangeValueClamped(FVector2D(165.0f, 375.0f), FVector2D(0.5f, 1.0f), Speed2D);

	return FVector2D(CalculateRelativeAccelerationAmount().Y * SpeedScale, 0.0f);
}

/**
 * The aim offset value: X is the yaw delta and Y is the pitch delta between the aiming rotation and the root
 * bone's rotation, blended out to zero as the "Disable_AO" curve activates.
 */
FVector2D UZomAnimInstanceBase::GetAOValue() const
{
	const FRotator RootRotation = RootTransform.Rotator();

	const float YawDelta = FMath::FindDeltaAngleDegrees(RootRotation.Yaw, AimingRotation.Yaw);
	const float PitchDelta = FMath::FindDeltaAngleDegrees(RootRotation.Pitch, AimingRotation.Pitch);

	const float DisableAOAlpha = GetCurveValue(FName("Disable_AO"));
	const FVector BlendedAO = FMath::Lerp(FVector(YawDelta, PitchDelta, 0.0f), FVector::ZeroVector, DisableAOAlpha);

	return FVector2D(BlendedAO.X, BlendedAO.Y);
}

/**
 * The aim offset yaw: the aim offset value's yaw delta while strafing, otherwise 0 (orienting to movement or
 * aiming both keep the character facing the aim direction already, so there's no yaw offset left to apply).
 */
float UZomAnimInstanceBase::GetAOYaw() const
{
	switch (RotationMode)
	{
	case ERotationMode::Strafe:
		return GetAOValue().X;

	case ERotationMode::OrientToMovement:
	case ERotationMode::Aim:
	default:
		return 0.0f;
	}
}

/**
 * Whether aim offset should be enabled: the character is strafing, the aim offset yaw is within the threshold
 * for the current movement state (tighter while idle than while moving), and no montage is significantly
 * weighted in on the default slot (aim offset would fight a montage that's driving its own upper body pose).
 */
bool UZomAnimInstanceBase::EnableAO() const
{
	const float MaxYawThreshold = (MovementState == EMovementState::Idle) ? 115.0f : 180.0f;

	const bool bYawWithinThreshold = FMath::Abs(GetAOValue().X) <= MaxYawThreshold;
	const bool bIsStrafing = (RotationMode == ERotationMode::Strafe);
	const bool bSlotWeightLow = Blueprint_GetSlotMontageLocalWeight(FName("DefaultSlot")) < 0.5f;

	return bYawWithinThreshold && bIsStrafing && bSlotWeightLow;
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

/**
 * The Offset Root Bone node's rotation mode: released while the default slot is playing a montage (so the
 * montage's own rotation drives the root), otherwise accumulated as normal.
 */
EOffsetRootBoneMode UZomAnimInstanceBase::GetOffsetRootRotationMode() const
{
	return IsSlotActive(FName("DefaultSlot")) ? EOffsetRootBoneMode::Release : EOffsetRootBoneMode::Accumulate;
}

/**
 * The Offset Root Bone node's translation mode: released while the default slot is playing a montage, released
 * while on the ground and stationary (no motion to offset), interpolated while on the ground and moving so the
 * translation offset catches up smoothly, and released in every other movement mode.
 */
EOffsetRootBoneMode UZomAnimInstanceBase::GetOffsetRootTranslationMode() const
{
	if (IsSlotActive(FName("DefaultSlot")))
	{
		return EOffsetRootBoneMode::Release;
	}

	switch (MovementMode)
	{
	case ECharacterMovementMode::OnGround:
		return IsMoving() ? EOffsetRootBoneMode::Interpolate : EOffsetRootBoneMode::Release;

	default:
		return EOffsetRootBoneMode::Release;
	}
}

/**
 * The Offset Root Bone node's translation half life: shorter while idle so the offset catches up quickly,
 * longer while moving so it blends out more gradually.
 */
float UZomAnimInstanceBase::GetOffsetRootTranslationHalfLife() const
{
	switch (MovementState)
	{
	case EMovementState::Idle:
		return 0.1f;

	case EMovementState::Moving:
	default:
		return 0.3f;
	}
}

// The Foot Placement node's plant settings: the Stops variant while the current database is tagged "Stops", otherwise the Default variant.
FFootPlacementPlantSettings UZomAnimInstanceBase::GetFootPlacementPlantSettings() const
{
	return CurrentDatabaseTags.Contains(FName("Stops")) ? PlantSettings_Stops : PlantSettings_Default;
}

// The Foot Placement node's interpolation settings: the Stops variant while the current database is tagged "Stops", otherwise the Default variant.
FFootPlacementInterpolationSettings UZomAnimInstanceBase::GetFootPlacementInterpolationSettings() const
{
	return CurrentDatabaseTags.Contains(FName("Stops")) ? InterpolationSettings_Stops : InterpolationSettings_Default;
}