// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ZomPlayerController.generated.h"


// Forward declarations

class AZomPlayerCharacter;
class UInputMappingContext;
class UInputAction;
class UZomGameplayAbility;
struct FInputActionValue;


/**
 * 
 */
UCLASS(Blueprintable, meta=(DisplayName="Zom Player Controller"))
class ZOM_API AZomPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AZomPlayerController();

	// -------------
	// Functions
	// -------------

	// Returns the cached player character possessed by this controller
	UFUNCTION(BlueprintCallable, Category = "Zom", meta = (DisplayName = "Get Cached Player Character"))
	AZomPlayerCharacter* GetCachedPlayerCharacter() const { return CachedPlayerCharacter.Get(); }

	// -------------
	// Properties
	// -------------

protected:

	// -------------
	// Functions
	// -------------

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called to bind functionality to input
	virtual void SetupInputComponent() override;

	// Called when this controller possesses a pawn
	virtual void OnPossess(APawn* InPawn) override;

	// Called when this controller unpossesses a pawn
	virtual void OnUnPossess() override;

	// -------------
	// Properties
	// -------------

	// Enhanced Input mapping context added to the local player's input subsystem in SetupInputComponent.
	// Requires a project-owned IMC asset assigned in the editor (none exists yet - see Content/_Game/Input/).
	UPROPERTY(EditDefaultsOnly, Category = "Zom|Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	// -------------
	// Input Actions
	// -------------
	// Requires project-owned IA_*/IMC_Default assets assigned in the editor (Content/_Game/Input/, not yet authored).

	UPROPERTY(EditDefaultsOnly, Category = "Zom|Input")
	TObjectPtr<UInputAction> IA_Move;

	UPROPERTY(EditDefaultsOnly, Category = "Zom|Input")
	TObjectPtr<UInputAction> IA_Look;

	UPROPERTY(EditDefaultsOnly, Category = "Zom|Input")
	TObjectPtr<UInputAction> IA_Jump;

	UPROPERTY(EditDefaultsOnly, Category = "Zom|Input")
	TObjectPtr<UInputAction> IA_Sprint;

	UPROPERTY(EditDefaultsOnly, Category = "Zom|Input")
	TObjectPtr<UInputAction> IA_Crouch;

	UPROPERTY(EditDefaultsOnly, Category = "Zom|Input")
	TObjectPtr<UInputAction> IA_LightAttack;

	UPROPERTY(EditDefaultsOnly, Category = "Zom|Input")
	TObjectPtr<UInputAction> IA_HeavyAttack;

	UPROPERTY(EditDefaultsOnly, Category = "Zom|Input")
	TObjectPtr<UInputAction> IA_RangedShoot;

	UPROPERTY(EditDefaultsOnly, Category = "Zom|Input")
	TObjectPtr<UInputAction> IA_Reload;

	UPROPERTY(EditDefaultsOnly, Category = "Zom|Input")
	TObjectPtr<UInputAction> IA_Dodge;

	UPROPERTY(EditDefaultsOnly, Category = "Zom|Input")
	TObjectPtr<UInputAction> IA_Shove;

	// -------------
	// Ability Classes
	// -------------
	// Which UZomGameplayAbility each ability input activates. Orthogonal to UZomAbilitySetData (that grants
	// these classes to the ASC on BeginPlay; this just maps input to a class to try-activate).

	UPROPERTY(EditDefaultsOnly, Category = "Zom|Abilities")
	TSubclassOf<UZomGameplayAbility> LightAttackAbilityClass;

	UPROPERTY(EditDefaultsOnly, Category = "Zom|Abilities")
	TSubclassOf<UZomGameplayAbility> HeavyAttackAbilityClass;

	UPROPERTY(EditDefaultsOnly, Category = "Zom|Abilities")
	TSubclassOf<UZomGameplayAbility> RangedShootAbilityClass;

	UPROPERTY(EditDefaultsOnly, Category = "Zom|Abilities")
	TSubclassOf<UZomGameplayAbility> ReloadAbilityClass;

	UPROPERTY(EditDefaultsOnly, Category = "Zom|Abilities")
	TSubclassOf<UZomGameplayAbility> DodgeAbilityClass;

	UPROPERTY(EditDefaultsOnly, Category = "Zom|Abilities")
	TSubclassOf<UZomGameplayAbility> ShoveAbilityClass;

private:

	// -------------
	// Functions
	// -------------

	void Input_Move(const FInputActionValue& Value);
	void Input_MoveCompleted();
	void Input_Look(const FInputActionValue& Value);
	void Input_JumpStarted();
	void Input_JumpCompleted();
	void Input_SprintStarted();
	void Input_SprintCompleted();
	void Input_CrouchStarted();
	void Input_CrouchCompleted();

	void ActivateAbilityByClass(TSubclassOf<UZomGameplayAbility> AbilityClass);

	// -------------
	// Properties
	// -------------

	// Cached reference to the player character possessed by this controller
	TWeakObjectPtr<AZomPlayerCharacter> CachedPlayerCharacter;
};
