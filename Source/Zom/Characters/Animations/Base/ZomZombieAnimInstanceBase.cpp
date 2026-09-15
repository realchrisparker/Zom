// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Characters/Animations/Base/ZomZombieAnimInstanceBase.h"
#include "Zom/Characters/Base/ZomCharacterBase.h"
#include "Zom/Characters/Components/ZomCharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"


// Constructor
UZomZombieAnimInstanceBase::UZomZombieAnimInstanceBase()
{
    AnimationType = GetRandomInteger(2);
}

// Called when the anim instance is created and its owning component/actor are valid; good place to cache references
void UZomZombieAnimInstanceBase::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    // Cache the character reference if the owning actor is a Zom character
    CachedCharacter = Cast<AZomCharacterBase>(TryGetPawnOwner());

    // Cache the character movement component reference if the player character is valid
    CachedCharacterMovementComponent = CachedCharacter.IsValid() ? Cast<UZomCharacterMovementComponent>(CachedCharacter->GetCharacterMovement()) : nullptr;

    bHasOwningActor = CachedCharacter.IsValid();
}

// Called when the anim instance is being uninitialized (e.g. anim class changing, owning component being destroyed); good place to clear cached references
void UZomZombieAnimInstanceBase::NativeUninitializeAnimation()
{
    Super::NativeUninitializeAnimation();

    // Clear cached references to avoid dangling pointers

    CachedCharacter = nullptr;
    CachedCharacterMovementComponent = nullptr;

    bHasOwningActor = false;
}

// Called every frame on the game thread before the animation graph is updated
void UZomZombieAnimInstanceBase::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    bHasOwningActor = CachedCharacter.IsValid();

    if (!bHasOwningActor || !CachedCharacterMovementComponent.IsValid())
    {
        return;
    }

    AZomCharacterBase* Character = CachedCharacter.Get();
    UZomCharacterMovementComponent* MovementComponent = CachedCharacterMovementComponent.Get();

    // Velocity
    Velocity_LastFrame = Velocity;
    Velocity = MovementComponent->Velocity;

    // Rotation
    ActorRotation = Character->GetActorRotation();
}

// Called every frame, potentially on a worker thread; only safe to read data here, not to modify UObjects
void UZomZombieAnimInstanceBase::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

    if (!bHasOwningActor)
    {
        return;
    }

    // Speed / velocity state
    Speed2D = Velocity.Size2D();
    bHasVelocity = !Velocity.IsNearlyZero();

    // Movement direction relative to the actor's facing
    Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, ActorRotation);

    // Acceleration derived from the change in velocity over time
    Acceleration = (DeltaSeconds > KINDA_SMALL_NUMBER) ? (Velocity - Velocity_LastFrame) / DeltaSeconds : FVector::ZeroVector;

    bHasAcceleration = !Acceleration.IsNearlyZero();
}

/**
 * Whether the character is moving: the current velocity and acceleration are both non-zero.
 */
bool UZomZombieAnimInstanceBase::IsMoving() const
{
    if (Velocity.IsNearlyZero() || Acceleration.IsNearlyZero()) return false;

    return true;
}

/**
 * Returns a random integer in the inclusive range [0, Max]. A negative Max is clamped to 0.
 */
int32 UZomZombieAnimInstanceBase::GetRandomInteger(int32 Max) const
{
    return FMath::RandRange(0, FMath::Max(Max, 0));
}