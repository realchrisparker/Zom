// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimNodeReference.h"
#include "Animation/TrajectoryTypes.h"
#include "Animation/AnimClassInterface.h"
#include "Animation/AnimSubsystem_Tag.h"
#include "AnimationWarpingTypes.h"
#include "BoneControllers/AnimNode_FootPlacement.h"
#include "BoneControllers/AnimNode_OffsetRootBone.h"
#include "BoneControllers/AnimNode_OrientationWarping.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PoseSearch/PoseSearchTrajectoryLibrary.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "PoseSearch/PoseSearchLibrary.h"
#include "Zom/Characters/Enums/ZomCharacterEnums.h"
#include "ZomAnimInstanceBase.generated.h"


// Forward declarations

class AZomCharacterBase;
class UZomCharacterMovementComponent;


/**
 * Animation instance base class for Zom characters. This class provides a foundation for character animation logic and can be extended to implement specific animation behaviors.
 */
UCLASS(Blueprintable, meta = (DisplayName = "Zom Anim Instance Base"))
class ZOM_API UZomAnimInstanceBase : public UAnimInstance
{
	GENERATED_BODY()

public:

	// Constructor
	UZomAnimInstanceBase();

	// -------------
	// General
	// -------------

	UPROPERTY(BlueprintReadWrite, Category = "Zom|General", meta = (DisplayName = "Has Owning Actor"))
	bool bHasOwningActor = false;

	// -------------
	// Movement State
	// -------------

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Movement State", meta = (DisplayName = "Speed 2D"))
	float Speed2D = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Movement State", meta = (DisplayName = "Movement Mode"))
	ECharacterMovementMode MovementMode = ECharacterMovementMode::OnGround;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Movement State", meta = (DisplayName = "Movement Mode (Last Frame)"))
	ECharacterMovementMode MovementMode_LastFrame = ECharacterMovementMode::OnGround;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Movement State", meta = (DisplayName = "Rotation Mode"))
	ERotationMode RotationMode = ERotationMode::OrientToMovement;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Movement State", meta = (DisplayName = "Rotation Mode (Last Frame)"))
	ERotationMode RotationMode_LastFrame = ERotationMode::OrientToMovement;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Movement State", meta = (DisplayName = "Movement State"))
	EMovementState MovementState = EMovementState::Idle;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Movement State", meta = (DisplayName = "Movement State (Last Frame)"))
	EMovementState MovementState_LastFrame = EMovementState::Idle;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Movement State", meta = (DisplayName = "Gait"))
	EGait Gait = EGait::Walk;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Movement State", meta = (DisplayName = "Gait (Last Frame)"))
	EGait Gait_LastFrame = EGait::Walk;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Movement State", meta = (DisplayName = "Stance"))
	EStance Stance = EStance::Stand;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Movement State", meta = (DisplayName = "Stance (Last Frame)"))
	EStance Stance_LastFrame = EStance::Stand;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Movement State", meta = (DisplayName = "Movement Direction"))
	EMovementDirection MovementDirection = EMovementDirection::F;

	// Current combat state. Unarmed is the default state; other states are set manually (by AZomPlayerController, in response to combat input actions).
	UPROPERTY(BlueprintReadWrite, Category = "Zom|Combat", meta = (DisplayName = "Combat State"))
	ECombatState CombatState = ECombatState::Unarmed;

	// -------------
	// Locomotion
	// -------------

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Locomotion", meta = (DisplayName = "Actor Transform"))
	FTransform ActorTransform;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Locomotion", meta = (DisplayName = "Velocity"))
	FVector Velocity = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Locomotion", meta = (DisplayName = "Input Acceleration"))
	FVector InputAcceleration = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Locomotion", meta = (DisplayName = "Current Max Acceleration"))
	float CurrentMaxAcceleration = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Locomotion", meta = (DisplayName = "Current Max Deceleration"))
	float CurrentMaxDeceleration = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Locomotion", meta = (DisplayName = "Orientation Intent"))
	FRotator OrientationIntent = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Locomotion", meta = (DisplayName = "Aiming Rotation"))
	FRotator AimingRotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Locomotion", meta = (DisplayName = "Steering Time"))
	float SteeringTime = 0.0f;

	// -------------
	// Landing
	// -------------

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Landing", meta = (DisplayName = "Just Landed"))
	bool bJustLanded = false;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Landing", meta = (DisplayName = "Land Velocity"))
	FVector LandVelocity = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Landing", meta = (DisplayName = "Heavy Land Speed Threshold"))
	float HeavyLandSpeedThreshold = 700.0f;

	// -------------
	// Velocity
	// -------------

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Velocity", meta = (DisplayName = "Has Velocity"))
	bool bHasVelocity = false;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Velocity", meta = (DisplayName = "Velocity (Last Frame)"))
	FVector Velocity_LastFrame = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Velocity", meta = (DisplayName = "Last Non Zero Velocity"))
	FVector LastNonZeroVelocity = FVector::ZeroVector;

	// -------------
	// Acceleration
	// -------------

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Acceleration", meta = (DisplayName = "Has Acceleration"))
	bool bHasAcceleration = false;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Acceleration", meta = (DisplayName = "Acceleration"))
	FVector Acceleration = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Acceleration", meta = (DisplayName = "Acceleration (Last Frame)"))
	FVector Acceleration_LastFrame = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Acceleration", meta = (DisplayName = "Acceleration Amount"))
	float AccelerationAmount = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Acceleration", meta = (DisplayName = "Relative Acceleration"))
	FVector RelativeAcceleration = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Acceleration", meta = (DisplayName = "Velocity Acceleration"))
	FVector VelocityAcceleration = FVector::ZeroVector;

	// -------------
	// Transforms
	// -------------

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Transforms", meta = (DisplayName = "Character Transform"))
	FTransform CharacterTransform;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Transforms", meta = (DisplayName = "Character Transform (Last Frame)"))
	FTransform CharacterTransform_LastFrame;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Transforms", meta = (DisplayName = "Root Transform"))
	FTransform RootTransform;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Transforms", meta = (DisplayName = "Interaction Transform"))
	FTransform InteractionTransform;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Transforms", meta = (DisplayName = "Offset Root Bone Enabled"))
	bool bOffsetRootBoneEnabled = false;

	// -------------
	// Ground
	// -------------

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Ground", meta = (DisplayName = "Ground Normal"))
	FVector GroundNormal = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Ground", meta = (DisplayName = "Ground Location"))
	FVector GroundLocation = FVector::ZeroVector;

	// -------------
	// Based Movement
	// -------------

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Based Movement", meta = (DisplayName = "Based Movement Delta"))
	FTransform BasedMovementDelta;

	// -------------
	// Trajectory
	// -------------

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Trajectory", meta = (DisplayName = "Trajectory Generation Data (Idle)"))
	FPoseSearchTrajectoryData TrajectoryGenerationData_Idle;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Trajectory", meta = (DisplayName = "Trajectory Generation Data (Moving)"))
	FPoseSearchTrajectoryData TrajectoryGenerationData_Moving;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Trajectory", meta = (DisplayName = "Trajectory"))
	FTransformTrajectory Trajectory;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Trajectory", meta = (DisplayName = "Trajectory Collision"))
	FPoseSearchTrajectory_WorldCollisionResults TrajectoryCollision;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Trajectory", meta = (DisplayName = "Previous Desired Controller Yaw"))
	float PreviousDesiredControllerYaw = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Trajectory", meta = (DisplayName = "Trajectory Past Velocity"))
	FVector Trj_PastVelocity = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Trajectory", meta = (DisplayName = "Trajectory Current Velocity"))
	FVector Trj_CurrentVelocity = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Trajectory", meta = (DisplayName = "Trajectory Future Velocity"))
	FVector Trj_FutureVelocity = FVector::ZeroVector;

	// -------------
	// Root Offset
	// -------------

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Root Offset", meta = (DisplayName = "Offset Root Translation Radius"))
	float OffsetRootTranslationRadius = 0.0f;

	// -------------
	// Foot Placement
	// -------------

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Foot Placement", meta = (DisplayName = "Plant Settings (Default)"))
	FFootPlacementPlantSettings PlantSettings_Default;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Foot Placement", meta = (DisplayName = "Plant Settings (Stops)"))
	FFootPlacementPlantSettings PlantSettings_Stops;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Foot Placement", meta = (DisplayName = "Interpolation Settings (Default)"))
	FFootPlacementInterpolationSettings InterpolationSettings_Default;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Foot Placement", meta = (DisplayName = "Interpolation Settings (Stops)"))
	FFootPlacementInterpolationSettings InterpolationSettings_Stops;

	// -------------
	// Motion Matching
	// -------------

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Motion Matching", meta = (DisplayName = "MM Database LOD"))
	int32 MMDatabaseLOD = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Motion Matching", meta = (DisplayName = "Current Selected Anim"))
	TObjectPtr<UObject> CurrentSelectedAnim = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Motion Matching", meta = (DisplayName = "Current Selected Database"))
	TObjectPtr<const UPoseSearchDatabase> CurrentSelectedDatabase = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Motion Matching", meta = (DisplayName = "Valid Databases"))
	TArray<TObjectPtr<const UPoseSearchDatabase>> ValidDatabases;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Motion Matching", meta = (DisplayName = "MM Search Cost"))
	float MMSearchCost = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Motion Matching", meta = (DisplayName = "Current Database Tags"))
	TArray<FName> CurrentDatabaseTags;

	// -------------
	// Debug
	// -------------

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Debug", meta = (DisplayName = "Transition History"))
	TArray<FString> TransitionHistory;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Debug", meta = (DisplayName = "Pawn Speed History"))
	TArray<float> PawnSpeedHistory;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Debug", meta = (DisplayName = "Move Data Speed History"))
	TArray<float> MoveData_Speed_History;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Debug", meta = (DisplayName = "Phase History"))
	TArray<float> Phase_History;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Debug", meta = (DisplayName = "Contact L History"))
	TArray<float> Contact_L_History;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Debug", meta = (DisplayName = "Contact R History"))
	TArray<float> Contact_R_History;

	UPROPERTY(BlueprintReadWrite, Category = "Zom|Debug", meta = (DisplayName = "Enable Warping History"))
	TArray<float> Enable_Warping_History;

	// -------------
	// Functions
	// -------------

	/**
	 * Whether the character is moving: the current velocity and acceleration are both non-zero.
	 */
	UFUNCTION(BlueprintPure, Category = "Zom|Locomotion", meta = (DisplayName = "Is Moving", BlueprintThreadSafe))
	bool IsMoving() const;

	/**
	 * Whether the character is starting to move: the predicted future trajectory speed is significantly higher
	 * than the current speed, and the currently selected database isn't already a pivot/transition database.
	 */
	UFUNCTION(BlueprintPure, Category = "Zom|Motion Matching", meta = (DisplayName = "Is Starting", BlueprintThreadSafe))
	bool IsStarting() const;

	/**
	 * Whether the character is pivoting: the character is moving, and its trajectory turn angle exceeds a
	 * threshold that depends on the current rotation mode (looser while orienting to movement, tightest while aiming).
	 */
	UFUNCTION(BlueprintPure, Category = "Zom|Motion Matching", meta = (DisplayName = "Is Pivoting", BlueprintThreadSafe))
	bool IsPivoting() const;

	/**
	 * Whether the character's rotation has diverged enough from the animation root's rotation, at high enough
	 * speed, to warrant a spin transition, and the currently selected database isn't already a pivot/transition database.
	 */
	UFUNCTION(BlueprintPure, Category = "Zom|Motion Matching", meta = (DisplayName = "Should Spin Transition", BlueprintThreadSafe))
	bool ShouldSpinTransition() const;

	/**
	 * Whether the character should turn in place: the orientation intent has diverged enough from the animation
	 * root's rotation, and the character either wants to aim or has just come to a stop from moving.
	 */
	UFUNCTION(BlueprintPure, Category = "Zom|Motion Matching", meta = (DisplayName = "Should Turn In Place", BlueprintThreadSafe))
	bool ShouldTurnInPlace() const;

	/**
	 * The yaw angle between the direction the character is accelerating towards and its current direction of travel.
	 */
	UFUNCTION(BlueprintPure, Category = "Zom|Motion Matching", meta = (DisplayName = "Get Trajectory Turn Angle", BlueprintThreadSafe))
	float GetTrajectoryTurnAngle() const;

	/**
	 * The 2D lean amount used to drive lean/tilt animations: X is lateral lean, driven by the lateral (Y) component
	 * of the relative acceleration amount and scaled up as speed ramps from 165 to 375. Y is currently unused.
	 */
	UFUNCTION(BlueprintPure, Category = "Zom|Locomotion", meta = (DisplayName = "Get Lean Amount", BlueprintThreadSafe))
	FVector2D GetLeanAmount() const;

	/**
	 * The aim offset value: X is the yaw delta and Y is the pitch delta between the aiming rotation and the root
	 * bone's rotation, blended out to zero as the "Disable_AO" curve activates.
	 */
	UFUNCTION(BlueprintPure, Category = "Zom|Locomotion", meta = (DisplayName = "Get AO Value", BlueprintThreadSafe))
	FVector2D GetAOValue() const;

	/**
	 * The aim offset yaw: the aim offset value's yaw delta while strafing, otherwise 0 (orienting to movement or
	 * aiming both keep the character facing the aim direction already, so there's no yaw offset left to apply).
	 */
	UFUNCTION(BlueprintPure, Category = "Zom|Locomotion", meta = (DisplayName = "Get AO Yaw", BlueprintThreadSafe))
	float GetAOYaw() const;

	/**
	 * Whether aim offset should be enabled: the character is strafing, the aim offset yaw is within the threshold
	 * for the current movement state (tighter while idle than while moving), and no montage is significantly
	 * weighted in on the default slot (aim offset would fight a montage that's driving its own upper body pose).
	 */
	UFUNCTION(BlueprintPure, Category = "Zom|Locomotion", meta = (DisplayName = "Enable AO", BlueprintThreadSafe))
	bool EnableAO() const;

	// Whether the character just landed with a vertical land speed below the heavy land threshold
	UFUNCTION(BlueprintPure, Category = "Zom|Landing", meta = (DisplayName = "Just Landed (Light)", BlueprintThreadSafe))
	bool JustLanded_Light() const;

	// Whether the character just landed with a vertical land speed at or above the heavy land threshold
	UFUNCTION(BlueprintPure, Category = "Zom|Landing", meta = (DisplayName = "Just Landed (Heavy)", BlueprintThreadSafe))
	bool JustLanded_Heavy() const;

	/**
	 * Whether the character just traversed: no traversal montage is currently playing on the traversal slot, the
	 * "MovingTraversal" curve is active, and the trajectory turn angle is shallow enough to still be moving forward.
	 */
	UFUNCTION(BlueprintPure, Category = "Zom|Traversal", meta = (DisplayName = "Just Traversed", BlueprintThreadSafe))
	bool JustTraversed() const;

	/**
	 * Whether root motion can currently be steered: the character is moving or in the air (steering idle animations
	 * can cause them to slide), and the blend stack's currently active anim (given by Node) is active.
	 */
	UFUNCTION(BlueprintPure, Category = "Zom|Motion Matching", meta = (DisplayName = "Enable Steering", BlueprintThreadSafe))
	bool EnableSteering(const FAnimNodeReference& Node) const;

	/**
	 * The steering node's target rotation: the predicted trajectory's facing half a second into the future, so
	 * steering rotates towards where the character is headed rather than lagging behind its current rotation.
	 */
	UFUNCTION(BlueprintPure, Category = "Zom|Motion Matching", meta = (DisplayName = "Get Desired Facing", BlueprintThreadSafe))
	FQuat GetDesiredFacing() const;

	/**
	 * The warping space Orientation Warping should use: the root bone transform when Offset Root Bone is enabled
	 * (since that lets the root bone and component transforms diverge), otherwise the component transform.
	 */
	UFUNCTION(BlueprintPure, Category = "Zom|Motion Matching", meta = (DisplayName = "Get Orientation Warping Warping Space", BlueprintThreadSafe))
	EOrientationWarpingSpace GetOrientationWarpingWarpingSpace() const;

	/**
	 * The Motion Matching node's blend time: shorter right after landing (OnGround, was InAir) so land animations
	 * blend in quickly, very short right after jumping (InAir, moving up fast) for a snappy jump start, 0.5s otherwise.
	 */
	UFUNCTION(BlueprintPure, Category = "Zom|Motion Matching", meta = (DisplayName = "Get MM Blend Time", BlueprintThreadSafe))
	float GetMMBlendTime() const;

	/**
	 * The Motion Matching node's notify recency time out, by Gait. Must stay larger than the time between footstep
	 * notifies for that gait, otherwise notifies get filtered out as stale.
	 */
	UFUNCTION(BlueprintPure, Category = "Zom|Motion Matching", meta = (DisplayName = "Get MM Notify Recency Time Out", BlueprintThreadSafe))
	float GetMMNotifyRecencyTimeOut() const;

	/**
	 * The Motion Matching node's continuing-pose interrupt mode: interrupts the database search (allowing a
	 * database change) when the character's high level state has meaningfully changed since last frame.
	 */
	UFUNCTION(BlueprintPure, Category = "Zom|Motion Matching", meta = (DisplayName = "Get MM Interrupt Mode", BlueprintThreadSafe))
	EPoseSearchInterruptMode GetMMInterruptMode() const;

	/**
	 * The Motion Matching node's offset root translation radius: the distance from the root bone to the component
	 * transform at which the offset root bone is disabled, so the root bone and component transform don't diverge
	 * too far apart.
	 */
	UFUNCTION(BlueprintPure, Category = "Zom|Motion Matching", meta = (DisplayName = "Get Offset Root Translation Radius", BlueprintThreadSafe))
	float GetOffsetRootTranslationRadius() const { return OffsetRootTranslationRadius; }

	/**
	 * The Offset Root Bone node's rotation mode: released while the default slot is playing a montage (so the
	 * montage's own rotation drives the root), otherwise accumulated as normal.
	 */
	UFUNCTION(BlueprintPure, Category = "Zom|Motion Matching", meta = (DisplayName = "Get Offset Root Rotation Mode", BlueprintThreadSafe))
	EOffsetRootBoneMode GetOffsetRootRotationMode() const;

	/**
	 * The Offset Root Bone node's translation mode: released while the default slot is playing a montage, released
	 * while on the ground and stationary (no motion to offset), interpolated while on the ground and moving so the
	 * translation offset catches up smoothly, and released in every other movement mode.
	 */
	UFUNCTION(BlueprintPure, Category = "Zom|Motion Matching", meta = (DisplayName = "Get Offset Root Translation Mode", BlueprintThreadSafe))
	EOffsetRootBoneMode GetOffsetRootTranslationMode() const;

	/**
	 * The Offset Root Bone node's translation half life: shorter while idle so the offset catches up quickly,
	 * longer while moving so it blends out more gradually.
	 */
	UFUNCTION(BlueprintPure, Category = "Zom|Motion Matching", meta = (DisplayName = "Get Offset Root Translation Half Life", BlueprintThreadSafe))
	float GetOffsetRootTranslationHalfLife() const;

	// The Foot Placement node's plant settings: the Stops variant while the current database is tagged "Stops", otherwise the Default variant.
	UFUNCTION(BlueprintPure, Category = "Zom|Foot Placement", meta = (DisplayName = "Get Foot Placement Plant Settings", BlueprintThreadSafe))
	FFootPlacementPlantSettings GetFootPlacementPlantSettings() const;

	// The Foot Placement node's interpolation settings: the Stops variant while the current database is tagged "Stops", otherwise the Default variant.
	UFUNCTION(BlueprintPure, Category = "Zom|Foot Placement", meta = (DisplayName = "Get Foot Placement Interpolation Settings", BlueprintThreadSafe))
	FFootPlacementInterpolationSettings GetFootPlacementInterpolationSettings() const;

protected:
	// Called when the anim instance is created and its owning component/actor are valid; good place to cache references
	virtual void NativeInitializeAnimation() override;

	// Called every frame on the game thread before the animation graph is updated
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	// Called every frame, potentially on a worker thread; only safe to read data here, not to modify UObjects
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

	// Called when the anim instance is being uninitialized (e.g. anim class changing, owning component being destroyed); good place to clear cached references
	virtual void NativeUninitializeAnimation() override;

	// Maps the movement component's native movement mode to our high level ECharacterMovementMode. Custom modes (Sliding, Traversing, Ragdoll) are not detectable yet and are left unchanged.
	ECharacterMovementMode MapNativeMovementMode(TEnumAsByte<EMovementMode> NativeMode) const;

	// Updates the predicted trajectory used by motion matching. Must run first in NativeUpdateAnimation, before anything else consumes this frame's trajectory.
	void UpdateTrajectory(float DeltaSeconds);

	// The character's current acceleration relative to its facing, normalized by the max acceleration/deceleration for whichever is currently relevant.
	FVector CalculateRelativeAccelerationAmount() const;

private:

	// -------------
	// Properties
	// -------------

	// Cached reference to the owning character. Typed to the shared base (not AZomPlayerCharacter) so this
	// anim instance class can eventually be reused by other AZomCharacterBase subclasses (e.g. Boss).
	TWeakObjectPtr<AZomCharacterBase> CachedPlayerCharacter;

	// Cached reference to the player character's movement component possessed by this controller
	TWeakObjectPtr<UZomCharacterMovementComponent> CachedCharacterMovementComponent;

	// Native movement mode gathered on the game thread in NativeUpdateAnimation, consumed by NativeThreadSafeUpdateAnimation
	TEnumAsByte<EMovementMode> CachedNativeMovementMode = MOVE_None;

	// Cached pointer to the AnimGraph's Offset Root Bone node, tagged "OffsetRoot", resolved once via FAnimSubsystem_Tag. Used every frame to read its simulated root transform.
	FAnimNode_OffsetRootBone* CachedOffsetRootBoneNode = nullptr;
};
