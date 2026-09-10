// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Characters/ZomPlayerCharacter.h"
#include "Zom/Characters/Components/ZomCharacterMovementComponent.h"
#include "MotionWarpingComponent.h"
#include "GameFramework/GameplayCameraComponent.h"
#include "GameFramework/PlayerState.h"
#include "ZomPlayerController.h"
#include "Zom/Misc/ZomLogChannels.h"
#include "Zom/Misc/ZomGameplayTags.h"
#include "Zom/Abilities/GA/ZomGA_LightAttack.h"
#include "Zom/Abilities/GA/ZomGA_HeavyAttack.h"
#include "Zom/Abilities/Effects/ZomGE_StaminaRegen.h"
#include "MotionCombatSystem/Components/MCS_CombatCoreComponent.h"
#include "MotionCombatSystem/Components/MCS_CombatHitboxComponent.h"
#include "AbilitySystemComponent.h"
#include "Zom/Abilities/AttributeSets/ZomPlayerAttributeSet.h"


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
	GameplayCamera->SetupAttachment(GetMesh());

	CurrentCamera = FGameplayTagContainer(TAG_Zom_Camera_State_Default.GetTag());

	// Starting ability - granted via DefaultAbilities (base class, GrantDefaultAbilitiesAndEffects) once the ASC
	// is initialized. Set here as a C++ default; override per-Blueprint if needed.
	DefaultAbilities.Add(UZomGA_LightAttack::StaticClass());
	DefaultAbilities.Add(UZomGA_HeavyAttack::StaticClass());

	// Passive Stamina regen - same DefaultGameplayEffects pipeline as DefaultAbilities above, applied once via
	// AddEffect once the ASC initializes. Infinite duration, so it just keeps running from then on.
	DefaultGameplayEffects.Add(UZomGE_StaminaRegen::StaticClass());
}

// Called when the game starts or when spawned
void AZomPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	BindCombatCoreEvents();
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
	// Also grants DefaultAbilities/DefaultGameplayEffects (see AZomCharacterBase::GrantDefaultAbilitiesAndEffects).
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

// Mirrors AZomCharacterBase::GetHealth - returns 0 before the ASC is initialized.
float AZomPlayerCharacter::GetStamina() const
{
	const UZomPlayerAttributeSet* AttributeSet = AbilitySystemComponent ? AbilitySystemComponent->GetSet<UZomPlayerAttributeSet>() : nullptr;
	return AttributeSet ? AttributeSet->GetStamina() : 0.f;
}

// Mirrors AZomCharacterBase::GetMaxHealth - returns 0 before the ASC is initialized.
float AZomPlayerCharacter::GetMaxStamina() const
{
	const UZomPlayerAttributeSet* AttributeSet = AbilitySystemComponent ? AbilitySystemComponent->GetSet<UZomPlayerAttributeSet>() : nullptr;
	return AttributeSet ? AttributeSet->GetMaxStamina() : 0.f;
}

// Binds the Handle* functions to GetCombatCoreComponent()'s delegates. RemoveDynamic before AddDynamic guards
// against a double bind if BeginPlay ever runs more than once for the same component (matches the idempotency
// pattern used for OnAttackResolved/Health in AZomCharacterBase::InitializeAbilitySystem).
void AZomPlayerCharacter::BindCombatCoreEvents()
{
	UMCS_CombatCoreComponent* CombatCore = GetCombatCoreComponent();
	if (!CombatCore)
	{
		return;
	}

	CombatCore->OnAttackStart.RemoveDynamic(this, &AZomPlayerCharacter::HandleCombatAttackStart);
	CombatCore->OnAttackStart.AddDynamic(this, &AZomPlayerCharacter::HandleCombatAttackStart);

	CombatCore->OnAttackEnd.RemoveDynamic(this, &AZomPlayerCharacter::HandleCombatAttackEnd);
	CombatCore->OnAttackEnd.AddDynamic(this, &AZomPlayerCharacter::HandleCombatAttackEnd);

	CombatCore->OnHitboxWindowBegin.RemoveDynamic(this, &AZomPlayerCharacter::HandleCombatHitboxWindowBegin);
	CombatCore->OnHitboxWindowBegin.AddDynamic(this, &AZomPlayerCharacter::HandleCombatHitboxWindowBegin);

	CombatCore->OnHitboxWindowEnd.RemoveDynamic(this, &AZomPlayerCharacter::HandleCombatHitboxWindowEnd);
	CombatCore->OnHitboxWindowEnd.AddDynamic(this, &AZomPlayerCharacter::HandleCombatHitboxWindowEnd);

	CombatCore->OnCameraControl.RemoveDynamic(this, &AZomPlayerCharacter::HandleCombatCameraControl);
	CombatCore->OnCameraControl.AddDynamic(this, &AZomPlayerCharacter::HandleCombatCameraControl);

	CombatCore->OnComboWindowBegin.RemoveDynamic(this, &AZomPlayerCharacter::HandleCombatComboWindowBegin);
	CombatCore->OnComboWindowBegin.AddDynamic(this, &AZomPlayerCharacter::HandleCombatComboWindowBegin);

	CombatCore->OnComboWindowEnd.RemoveDynamic(this, &AZomPlayerCharacter::HandleCombatComboWindowEnd);
	CombatCore->OnComboWindowEnd.AddDynamic(this, &AZomPlayerCharacter::HandleCombatComboWindowEnd);

	CombatCore->OnParryWindowBegin.RemoveDynamic(this, &AZomPlayerCharacter::HandleCombatParryWindowBegin);
	CombatCore->OnParryWindowBegin.AddDynamic(this, &AZomPlayerCharacter::HandleCombatParryWindowBegin);

	CombatCore->OnParryWindowEnd.RemoveDynamic(this, &AZomPlayerCharacter::HandleCombatParryWindowEnd);
	CombatCore->OnParryWindowEnd.AddDynamic(this, &AZomPlayerCharacter::HandleCombatParryWindowEnd);

	CombatCore->OnDefenseWindowBegin.RemoveDynamic(this, &AZomPlayerCharacter::HandleCombatDefenseWindowBegin);
	CombatCore->OnDefenseWindowBegin.AddDynamic(this, &AZomPlayerCharacter::HandleCombatDefenseWindowBegin);

	CombatCore->OnDefenseWindowEnd.RemoveDynamic(this, &AZomPlayerCharacter::HandleCombatDefenseWindowEnd);
	CombatCore->OnDefenseWindowEnd.AddDynamic(this, &AZomPlayerCharacter::HandleCombatDefenseWindowEnd);
}

void AZomPlayerCharacter::HandleCombatAttackStart()
{
	OnAttackStart();
}

void AZomPlayerCharacter::HandleCombatAttackEnd()
{
	OnAttackEnd(); // Forward the combat core attack end event to the Blueprint event
}

void AZomPlayerCharacter::HandleCombatHitboxWindowBegin(AActor* Attacker, const FMCS_AttackEntry& Attack, const TArray<FMCS_AttackHitbox>& Hitboxes)
{
	OnHitboxWindowBegin(Attacker, Attack, Hitboxes); // Forward the combat core hitbox window begin event to the Blueprint event

	// Initialize and start multi-hit detection for the combat hitbox component if it exists.
	if (UMCS_CombatHitboxComponent* HitboxComponent = GetCombatHitboxComponent())
	{
		HitboxComponent->ResetAlreadyHit();
		HitboxComponent->StartMultiHitDetection(GetCombatCoreComponent()->GetCurrentAttack(), Hitboxes);
	}
}

void AZomPlayerCharacter::HandleCombatHitboxWindowEnd(AActor* Attacker)
{
	OnHitboxWindowEnd(Attacker); // Forward the combat core hitbox window end event to the Blueprint event
}

// CameraTag drives CurrentCamera directly (rather than forwarding to a Blueprint event) - see CurrentCamera's
// comment: it's read every RunCameraDirector tick by CHT_CurrentCamera to pick the active camera rig, and
// AnimNotify_CameraControl -> CombatCore->OnCameraControl is exactly the "gameplay system" that comment refers to.
void AZomPlayerCharacter::HandleCombatCameraControl(FGameplayTag CameraTag)
{
	CurrentCamera = FGameplayTagContainer(CameraTag);
}

void AZomPlayerCharacter::HandleCombatComboWindowBegin()
{
	OnComboWindowBegin(); // Forward the combat core combo window begin event to the Blueprint event
}

void AZomPlayerCharacter::HandleCombatComboWindowEnd()
{
	OnComboWindowEnd(); // Forward the combat core combo window end event to the Blueprint event
}

void AZomPlayerCharacter::HandleCombatParryWindowBegin(AActor* Attacker)
{
	OnParryWindowBegin(Attacker); // Forward the combat core parry window begin event to the Blueprint event
}

void AZomPlayerCharacter::HandleCombatParryWindowEnd(AActor* Attacker)
{
	OnParryWindowEnd(Attacker); // Forward the combat core parry window end event to the Blueprint event
}

void AZomPlayerCharacter::HandleCombatDefenseWindowBegin(AActor* Defender)
{
	OnDefenseWindowBegin(Defender); // Forward the combat core defense window begin event to the Blueprint event
}

void AZomPlayerCharacter::HandleCombatDefenseWindowEnd(AActor* Defender)
{
	OnDefenseWindowEnd(Defender); // Forward the combat core defense window end event to the Blueprint event
}

FGameplayTag AZomPlayerCharacter::GetFactionTag_Implementation() const
{
	return TAG_Zom_Character_Player.GetTag();
}