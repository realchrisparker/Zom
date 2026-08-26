// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Characters/ZomPlayerCharacter.h"
#include "Zom/Characters/Components/ZomCharacterMovementComponent.h"
#include "MotionWarpingComponent.h"
#include "GameFramework/GameplayCameraComponent.h"
#include "GameFramework/PlayerState.h"
#include "ZomPlayerController.h"
#include "Zom/Misc/ZomLogChannels.h"
#include "Zom/Misc/ZomGameplayTags.h"
#include "Zom/Characters/Components/ZomInventoryComponent.h"


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

	// Create the inventory component
	Inventory = CreateDefaultSubobject<UZomInventoryComponent>(TEXT("Inventory"));

	CurrentCamera = TAG_Zom_Camera_State_Default.GetTag();
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

	// GetPlayerState(), not NewController->PlayerState: by the time PossessedBy runs server-side, the pawn's
	// own PlayerState is already valid, avoiding the null-timing window NewController->PlayerState can hit.
	InitializeAbilitySystem(GetPlayerState(), this);
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

// Called on clients when PlayerState is replicated; mirrors PossessedBy's ASC initialization
void AZomPlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	InitializeAbilitySystem(GetPlayerState(), this);
}

// Fires once Health reaches zero. Log-only for now - the real save/respawn flow lands with Section 11's save system.
void AZomPlayerCharacter::HandleDeath()
{
	UE_LOG(LogZomCharacter, Log, TEXT("%s died (respawn flow not implemented yet)."), *GetName());
}
