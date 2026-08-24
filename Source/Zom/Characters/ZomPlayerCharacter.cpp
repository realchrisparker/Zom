// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Characters/ZomPlayerCharacter.h"
#include "Zom/Characters/Components/ZomCharacterMovementComponent.h"
#include "MotionWarpingComponent.h"
#include "GameFramework/GameplayCameraComponent.h"
#include "ZomPlayerController.h"


// Sets default values
AZomPlayerCharacter::AZomPlayerCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UZomCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create the motion warping component
	MotionWarping = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarping"));

	// Create the gameplay camera component and attach it to the character's mesh
	GameplayCamera = CreateDefaultSubobject<UGameplayCameraComponent>(TEXT("GameplayCamera"));
}

// Called when the game starts or when spawned
void AZomPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AZomPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AZomPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

// Called when this pawn is possessed by a controller
void AZomPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Notify the movement component that the character has been possessed
	if (UZomCharacterMovementComponent* ZomMovementComponent = Cast<UZomCharacterMovementComponent>(GetCharacterMovement()))
	{
		ZomMovementComponent->OnCharacterPossessed(NewController);
	}

	// Cache the player controller reference if the new controller is a player controller
	CachedPlayerController = Cast<AZomPlayerController>(NewController);
}

// Called when this pawn is unpossessed by its controller
void AZomPlayerCharacter::UnPossessed()
{
	Super::UnPossessed();

	// Notify the movement component that the character has been unpossessed
	if (UZomCharacterMovementComponent* ZomMovementComponent = Cast<UZomCharacterMovementComponent>(GetCharacterMovement()))
	{
		ZomMovementComponent->OnCharacterUnPossessed();
	}

	// Clear the cached player controller reference
	CachedPlayerController = nullptr;
}
