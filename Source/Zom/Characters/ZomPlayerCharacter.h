// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Zom/Characters/Base/ZomPlayerCharacterBase.h"
#include "Zom/Characters/Enums/ZomCharacterEnums.h"
#include "ZomPlayerCharacter.generated.h"


// Forward declarations

class AZomPlayerController;
class UMotionWarpingComponent;
class UGameplayCameraComponent;
class UZomInventoryComponent;


/**
 * AZomPlayerCharacter
 * The player character class for the Zom game.
 */
UCLASS(Blueprintable, meta = (DisplayName = "Zom Player Character"))
class ZOM_API AZomPlayerCharacter : public AZomPlayerCharacterBase
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AZomPlayerCharacter(const FObjectInitializer& ObjectInitializer);


	// -------------
	// Functions
	// -------------

	/**
	 * Returns the cached player controller that possesses this character.
	 * @return The cached player controller, or nullptr if not possessed by a player controller.
	 */
	UFUNCTION(BlueprintCallable, Category = "Zom", meta = (DisplayName = "Get Cached Player Controller"))
	AZomPlayerController* GetCachedPlayerController() const { return CachedPlayerController.Get(); }

	/**
	 * Returns the motion warping component used to align root motion montages with world targets.
	 * @return The motion warping component.
	 */
	UFUNCTION(BlueprintCallable, Category = "Zom", meta = (DisplayName = "Get Motion Warping Component"))
	UMotionWarpingComponent* GetMotionWarpingComponent() const { return MotionWarping; }

	/**
	 * Returns the gameplay camera component attached to the character's mesh.
	 * @return The gameplay camera component.
	 */
	UFUNCTION(BlueprintCallable, Category = "Zom", meta = (DisplayName = "Get Gameplay Camera Component"))
	UGameplayCameraComponent* GetGameplayCameraComponent() const { return GameplayCamera; }

	/**
	 * Returns the current camera tag.
	 * @return The current camera tag.
	 */
	UFUNCTION(BlueprintPure, Category = "Zom", meta = (DisplayName = "Get Current Camera"))
	FGameplayTag GetCurrentCamera() const { return CurrentCamera; }

	/**
	 * Sets the current camera tag.
	 * @param NewCamera The new camera tag to set.
	 */
	UFUNCTION(BlueprintCallable, Category = "Zom", meta = (DisplayName = "Set Current Camera"))
	void SetCurrentCamera(FGameplayTag NewCamera) { CurrentCamera = NewCamera; }

	// -------------
	// Properties
	// -------------

	// Current gait. Walk/Run are set automatically by UZomCharacterMovementComponent based on current speed;
	// Sprint is set manually (by AZomPlayerController, in response to the sprint input action).
	UPROPERTY(BlueprintReadWrite, Category = "Zom|Movement State", meta = (DisplayName = "Gait"))
	EGait Gait = EGait::Walk;

protected:

	// -------------
	// Functions
	// -------------

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called when this pawn is possessed by a controller
	virtual void PossessedBy(AController* NewController) override;

	// Called when this pawn is unpossessed by its controller
	virtual void UnPossessed() override;

	// Called on clients when PlayerState is replicated; mirrors the ASC initialization done in PossessedBy
	// so the cached AbilitySystemComponent pointer is populated under replication too
	virtual void OnRep_PlayerState() override;

	// Fires once Health reaches zero. Log-only for now - the real save/respawn flow lands with Section 11's
	// save system; don't fabricate it early.
	virtual void HandleDeath() override;

	// -------------
	// Components
	// -------------

	// Motion warping component used to align root motion montages with world targets
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zom", meta = (DisplayName = "MotionWarping", AllowPrivateAccess = "true"))
	TObjectPtr<UMotionWarpingComponent> MotionWarping;

	// Gameplay camera component, attached to the character's mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zom", meta = (DisplayName = "GameplayCamera", AllowPrivateAccess = "true"))
	TObjectPtr<UGameplayCameraComponent> GameplayCamera;

	// Inventory component (Section 7 of the dev doc)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zom", meta = (DisplayName = "Inventory", AllowPrivateAccess = "true"))
	TObjectPtr<UZomInventoryComponent> Inventory;

private:

	// -------------
	// Properties
	// -------------

	// Cached reference to the player controller that possesses this character
	TWeakObjectPtr<AZomPlayerController> CachedPlayerController;

	// Current camera state. Read every RunCameraDirector tick by UZomPlayerCameraDirectorEvaluator (CDE_Player)
	// to pick which entry of its CameraRigsByTag map to activate. Set by whichever gameplay system owns a given
	// camera state (e.g. aiming, targeting) - defaults to TAG_Zom_Camera_State_Default.
	FGameplayTag CurrentCamera;
};
