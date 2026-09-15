// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Zom/Characters/Enums/ZomCharacterEnums.h"
#include "ZomZombieAnimInstanceBase.generated.h"


// Forward declarations

class AZomCharacterBase;
class UZomCharacterMovementComponent;


/**
 * Animation instance base class for Zom Zombie characters. This class provides a foundation for character animation logic and can be extended to implement specific animation behaviors. Zombies of different types can have their own animation instances derived from this base class.
 */
UCLASS(Blueprintable, meta = (DisplayName = "Zom Zombie Anim Instance Base"))
class ZOM_API UZomZombieAnimInstanceBase : public UAnimInstance
{
    GENERATED_BODY()

public:

    // Constructor
    UZomZombieAnimInstanceBase();

    // -------------
    // General
    // -------------

    // Indicates whether the anim instance has a valid owning actor.
    UPROPERTY(BlueprintReadWrite, Category = "Zom|General", meta = (DisplayName = "Has Owning Actor"))
    bool bHasOwningActor = false;

    // Determines the type of animation to use for this zombie instance.
    UPROPERTY(BlueprintReadWrite, Category = "Zom|General", meta = (DisplayName = "Animation Type"))
    int32 AnimationType = 0;

    // -------------
    // Movement State
    // -------------

    UPROPERTY(BlueprintReadWrite, Category = "Zom|Movement State", meta = (DisplayName = "Speed 2D"))
    float Speed2D = 0.0f;

    // -------------
    // Locomotion
    // -------------

    UPROPERTY(BlueprintReadWrite, Category = "Zom|Locomotion", meta = (DisplayName = "Velocity"))
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Zom|Velocity", meta = (DisplayName = "Has Velocity"))
    bool bHasVelocity = false;

    UPROPERTY(BlueprintReadWrite, Category = "Zom|Velocity", meta = (DisplayName = "Velocity (Last Frame)"))
    FVector Velocity_LastFrame = FVector::ZeroVector;

    // -------------
    // Rotation / Direction
    // -------------

    // World rotation of the owning actor, sampled on the game thread each frame.
    UPROPERTY(BlueprintReadWrite, Category = "Zom|Rotation", meta = (DisplayName = "Actor Rotation"))
    FRotator ActorRotation = FRotator::ZeroRotator;

    // Movement direction relative to the actor's facing, in degrees [-180, 180]. 0 = forward, 90 = right, -90 = left, +/-180 = backward.
    UPROPERTY(BlueprintReadWrite, Category = "Zom|Locomotion", meta = (DisplayName = "Direction"))
    float Direction = 0.0f;

    // -------------
    // Acceleration
    // -------------

    UPROPERTY(BlueprintReadWrite, Category = "Zom|Acceleration", meta = (DisplayName = "Has Acceleration"))
    bool bHasAcceleration = false;

    UPROPERTY(BlueprintReadWrite, Category = "Zom|Acceleration", meta = (DisplayName = "Acceleration"))
    FVector Acceleration = FVector::ZeroVector;

    // -------------
    // Functions
    // -------------

    /**
     * Whether the character is moving: the current velocity and acceleration are both non-zero.
     */
    UFUNCTION(BlueprintPure, Category = "Zom|Locomotion", meta = (DisplayName = "Is Moving", BlueprintThreadSafe))
    bool IsMoving() const;

protected:
    // Called when the anim instance is created and its owning component/actor are valid; good place to cache references
    virtual void NativeInitializeAnimation() override;

    // Called every frame on the game thread before the animation graph is updated
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // Called every frame, potentially on a worker thread; only safe to read data here, not to modify UObjects
    virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

    // Called when the anim instance is being uninitialized (e.g. anim class changing, owning component being destroyed); good place to clear cached references
    virtual void NativeUninitializeAnimation() override;

private:

    // -------------
    // Properties
    // -------------

    // Cached reference to the owning character. Typed to the shared base (not AZomPlayerCharacter) so this
    // anim instance class can eventually be reused by other AZomCharacterBase subclasses (e.g. Boss).
    TWeakObjectPtr<AZomCharacterBase> CachedCharacter;

    // Cached reference to the player character's movement component possessed by this controller
    TWeakObjectPtr<UZomCharacterMovementComponent> CachedCharacterMovementComponent;

    // -------------
    // Functions
    // -------------

    // Returns a random integer in the inclusive range [0, Max]. A negative Max is clamped to 0.
    int32 GetRandomInteger(int32 Max) const;
};