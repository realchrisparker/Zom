// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Characters/ZomPlayerController.h"
#include "Zom/Characters/ZomPlayerCharacter.h"
#include "Zom/Characters/Enums/ZomCharacterEnums.h"
#include "Zom/Abilities/ZomGameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "MotionCombatSystem/Structs/MCS_AttackSituation.h"
#include "MotionCombatSystem/Structs/MCS_AttackEntry.h"
#include "MotionCombatSystem/Components/MCS_CombatCoreComponent.h"
#include "MotionCombatSystem/Components/MCS_CombatHitboxComponent.h"
#include "MotionCombatSystem/Components/MCS_CombatHitReactionComponent.h"
#include "MotionCombatSystem/Components/MCS_CombatDefenseComponent.h"


AZomPlayerController::AZomPlayerController()
{

}

// Called when the game starts or when spawned
void AZomPlayerController::BeginPlay()
{
    Super::BeginPlay();

}

// Called to bind functionality to input
void AZomPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
    {
        if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
        {
            if (DefaultMappingContext)
            {
                InputSubsystem->AddMappingContext(DefaultMappingContext, 0);
            }
        }
    }

    UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
    if (!EnhancedInputComponent)
    {
        return;
    }

    if (IA_Move)
    {
        EnhancedInputComponent->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AZomPlayerController::Input_Move);
    }
    if (IA_Look)
    {
        EnhancedInputComponent->BindAction(IA_Look, ETriggerEvent::Triggered, this, &AZomPlayerController::Input_Look);
    }
    if (IA_Jump)
    {
        EnhancedInputComponent->BindAction(IA_Jump, ETriggerEvent::Started, this, &AZomPlayerController::Input_JumpStarted);
        EnhancedInputComponent->BindAction(IA_Jump, ETriggerEvent::Completed, this, &AZomPlayerController::Input_JumpCompleted);
    }
    if (IA_Sprint)
    {
        EnhancedInputComponent->BindAction(IA_Sprint, ETriggerEvent::Started, this, &AZomPlayerController::Input_SprintStarted);
        EnhancedInputComponent->BindAction(IA_Sprint, ETriggerEvent::Completed, this, &AZomPlayerController::Input_SprintCompleted);
    }
    if (IA_Crouch)
    {
        EnhancedInputComponent->BindAction(IA_Crouch, ETriggerEvent::Started, this, &AZomPlayerController::Input_CrouchStarted);
    }
    if (IA_WalkRun)
    {
        EnhancedInputComponent->BindAction(IA_WalkRun, ETriggerEvent::Started, this, &AZomPlayerController::Input_WalkRunStarted);
    }
    if (IA_LightAttack)
    {
        EnhancedInputComponent->BindAction(IA_LightAttack, ETriggerEvent::Started, this, &AZomPlayerController::Input_LightAttack);
    }
    if (IA_HeavyAttack)
    {
        EnhancedInputComponent->BindAction(IA_HeavyAttack, ETriggerEvent::Started, this, &AZomPlayerController::Input_HeavyAttack);
    }
    if (IA_RangedShoot)
    {
        EnhancedInputComponent->BindAction(IA_RangedShoot, ETriggerEvent::Started, this, &AZomPlayerController::ActivateAbilityByClass, RangedShootAbilityClass);
    }
    if (IA_Reload)
    {
        EnhancedInputComponent->BindAction(IA_Reload, ETriggerEvent::Started, this, &AZomPlayerController::ActivateAbilityByClass, ReloadAbilityClass);
    }
    if (IA_Dodge)
    {
        EnhancedInputComponent->BindAction(IA_Dodge, ETriggerEvent::Started, this, &AZomPlayerController::ActivateAbilityByClass, DodgeAbilityClass);
    }
    if (IA_Shove)
    {
        EnhancedInputComponent->BindAction(IA_Shove, ETriggerEvent::Started, this, &AZomPlayerController::ActivateAbilityByClass, ShoveAbilityClass);
    }
}

void AZomPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    // Cache the player character reference if the possessed pawn is a player character
    CachedPlayerCharacter = Cast<AZomPlayerCharacter>(InPawn);

    if (AZomPlayerCharacter* PlayerCharacter = CachedPlayerCharacter.Get())
    {
        if (UMCS_CombatCoreComponent* CombatCore = PlayerCharacter->GetCombatCoreComponent())
        {
            // Bind the HandleAttackResolved function to the CombatCore's OnAttackResolved delegate
            CombatCore->OnAttackResolved.AddDynamic(this, &AZomPlayerController::HandleAttackResolved);
        }
    }
}

void AZomPlayerController::OnUnPossess()
{
    if (AZomPlayerCharacter* PlayerCharacter = CachedPlayerCharacter.Get())
    {
        if (UMCS_CombatCoreComponent* CombatCore = PlayerCharacter->GetCombatCoreComponent())
        {
            // Unbind the HandleAttackResolved function from the CombatCore's OnAttackResolved delegate
            CombatCore->OnAttackResolved.RemoveDynamic(this, &AZomPlayerController::HandleAttackResolved);
        }
    }

    Super::OnUnPossess();

    // Clear the cached player character reference
    CachedPlayerCharacter = nullptr;
}

void AZomPlayerController::Input_Move(const FInputActionValue& Value)
{
    AZomPlayerCharacter* PlayerCharacter = CachedPlayerCharacter.Get();
    if (!PlayerCharacter)
    {
        return;
    }

    const FVector2D MovementVector = Value.Get<FVector2D>();

    const FRotator ViewRotation = GetControlRotation();
    const FRotator YawRotation(0, ViewRotation.Yaw, 0);

    const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
    const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

    PlayerCharacter->AddMovementInput(ForwardDirection, MovementVector.Y);
    PlayerCharacter->AddMovementInput(RightDirection, MovementVector.X);
}

void AZomPlayerController::Input_Look(const FInputActionValue& Value)
{
    AZomPlayerCharacter* PlayerCharacter = CachedPlayerCharacter.Get();
    if (!PlayerCharacter)
    {
        return;
    }

    const FVector2D LookAxisVector = Value.Get<FVector2D>();

    PlayerCharacter->AddControllerYawInput(LookAxisVector.X);
    PlayerCharacter->AddControllerPitchInput(LookAxisVector.Y);
}

void AZomPlayerController::Input_JumpStarted()
{
    if (AZomPlayerCharacter* PlayerCharacter = CachedPlayerCharacter.Get())
    {
        PlayerCharacter->Jump();
    }
}

void AZomPlayerController::Input_JumpCompleted()
{
    if (AZomPlayerCharacter* PlayerCharacter = CachedPlayerCharacter.Get())
    {
        PlayerCharacter->StopJumping();
    }
}

void AZomPlayerController::Input_SprintStarted()
{
    if (AZomPlayerCharacter* PlayerCharacter = CachedPlayerCharacter.Get())
    {
        PlayerCharacter->Gait = EGait::Sprint;
    }
}

void AZomPlayerController::Input_SprintCompleted()
{
    // Not Sprint any more; restore whichever of Walk/Run the player last toggled with IA_WalkRun.
    if (AZomPlayerCharacter* PlayerCharacter = CachedPlayerCharacter.Get())
    {
        PlayerCharacter->Gait = bWantsToRun ? EGait::Run : EGait::Walk;
    }
}

void AZomPlayerController::Input_CrouchStarted()
{
    // Toggle: tap once to crouch, tap again to stand - not a hold-to-crouch input.
    if (AZomPlayerCharacter* PlayerCharacter = CachedPlayerCharacter.Get())
    {
        if (PlayerCharacter->bIsCrouched)
        {
            PlayerCharacter->UnCrouch();
        }
        else
        {
            PlayerCharacter->Crouch();
        }
    }
}

void AZomPlayerController::Input_WalkRunStarted()
{
    // Toggle: tap once to switch Gait to Run, tap again to drop back to Walk. Doesn't touch Gait while Sprint
    // (a separate hold-based override) is active - it just updates what Sprint should restore on release.
    bWantsToRun = !bWantsToRun;

    if (AZomPlayerCharacter* PlayerCharacter = CachedPlayerCharacter.Get())
    {
        if (PlayerCharacter->Gait != EGait::Sprint)
        {
            PlayerCharacter->Gait = bWantsToRun ? EGait::Run : EGait::Walk;
        }
    }
}

void AZomPlayerController::Input_LightAttack()
{
    if(CachedPlayerCharacter.Get())
    {
        // Get the combat core component from the player character.
        UMCS_CombatCoreComponent* CombatCore = CachedPlayerCharacter.Get()->GetCombatCoreComponent();

        // Get the current attack situation from the player character.
        FMCS_AttackSituation CurrentAttackSituation = CachedPlayerCharacter.Get()->GetCurrentAttackSituation();

        // Perform a light attack using the combat core component.
        CombatCore->PerformAttack(EMCS_AttackType::Light, EMCS_AttackDirection::Forward, CurrentAttackSituation);
    }
}

void AZomPlayerController::Input_HeavyAttack()
{
    if(CachedPlayerCharacter.Get())
    {
        // Get the combat core component from the player character.
        UMCS_CombatCoreComponent* CombatCore = CachedPlayerCharacter.Get()->GetCombatCoreComponent();

        // Get the current attack situation from the player character.
        FMCS_AttackSituation CurrentAttackSituation = CachedPlayerCharacter.Get()->GetCurrentAttackSituation();

        // Perform a heavy attack using the combat core component.
        CombatCore->PerformAttack(EMCS_AttackType::Heavy, EMCS_AttackDirection::Forward, CurrentAttackSituation);
    }
}

void AZomPlayerController::ActivateAbilityByClass(TSubclassOf<UZomGameplayAbility> AbilityClass)
{
    if (!AbilityClass)
    {
        return;
    }

    AZomPlayerCharacter* PlayerCharacter = CachedPlayerCharacter.Get();
    if (!PlayerCharacter)
    {
        return;
    }

    if (UAbilitySystemComponent* ASC = PlayerCharacter->GetAbilitySystemComponent())
    {
        ASC->TryActivateAbilityByClass(AbilityClass);
    }
}

// =========================================
// MCS Related Functions/Events
// =========================================

void AZomPlayerController::HandleAttackResolved(const FMCS_AttackEntry& ResolvedAttack)
{
    UE_LOG(LogTemp, Log, TEXT("AZomPlayerController::HandleAttackResolved called with AttackTag: %s"), *ResolvedAttack.AttackTag.ToString());

    // Per OnAttackResolved's contract: this only fires on the GAS path (AttackTag valid) or the
    // Blueprint-only path (bAutoPlayMontage false, AttackTag empty). Nothing to activate in the latter case.
    if (!ResolvedAttack.AttackTag.IsValid())
    {
        return;
    }

    AZomPlayerCharacter* PlayerCharacter = CachedPlayerCharacter.Get();
    if (!PlayerCharacter)
    {
        return;
    }

    if (UAbilitySystemComponent* ASC = PlayerCharacter->GetAbilitySystemComponent())
    {
        UE_LOG(LogTemp, Log, TEXT("AZomPlayerController::HandleAttackResolved activating ability with AttackTag: %s"), *ResolvedAttack.AttackTag.ToString());
        ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(ResolvedAttack.AttackTag));
    }
}
