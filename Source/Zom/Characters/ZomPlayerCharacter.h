// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ZomPlayerCharacter.generated.h"


// Forward declarations

class AZomPlayerController;
class UMotionWarpingComponent;
class UGameplayCameraComponent;


/**
 * AZomPlayerCharacter
 * The player character class for the Zom game.
 */
UCLASS(Blueprintable, meta = (DisplayName = "Zom Player Character"))
class ZOM_API AZomPlayerCharacter : public ACharacter
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

	// -------------
	// Properties
	// -------------

protected:

	// -------------
	// Functions
	// -------------

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Called when this pawn is possessed by a controller
	virtual void PossessedBy(AController* NewController) override;

	// Called when this pawn is unpossessed by its controller
	virtual void UnPossessed() override;

	// -------------
	// Components
	// -------------
	
	// Motion warping component used to align root motion montages with world targets
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zom", meta = (DisplayName = "MotionWarping", AllowPrivateAccess = "true"))
	TObjectPtr<UMotionWarpingComponent> MotionWarping;

	// Gameplay camera component, attached to the character's mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zom", meta = (DisplayName = "GameplayCamera", AllowPrivateAccess = "true"))
	TObjectPtr<UGameplayCameraComponent> GameplayCamera;

private:

	// -------------
	// Properties
	// -------------

	// Cached reference to the player controller that possesses this character
	TWeakObjectPtr<AZomPlayerController> CachedPlayerController;
};
