// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Zom/Characters/Base/ZomCharacterBase.h"
#include "ZomZombieBase.generated.h"


class UZombieTypeData;
class UZomZombieAttributeSet;
class AZomToxicGasVolume;


/**
 * Crowd zombies (Regular/Auds/Eyes/Bloater). Owns its own ASC/AttributeSet as real subobjects (unlike
 * the player, whose ASC lives on AZomPlayerState). No per-type C++ subclassing - behavior differences come
 * from UZombieTypeData and from which senses are configured on AZomZombieAIController (Section 3/5.1).
 * Perception and the State Tree live on that AIController, not a component on this pawn - AutoPossessAI +
 * AIControllerClass below drive possession.
 */
UCLASS(Blueprintable, meta = (DisplayName = "Zom Zombie Base"))
class ZOM_API AZomZombieBase : public AZomCharacterBase
{
	GENERATED_BODY()

public:
	AZomZombieBase(const FObjectInitializer& ObjectInitializer);

	// -------------
	// Functions
	// -------------

	// Getter so AZomZombieAIController doesn't hold a second reference to the same asset.
	UFUNCTION(BlueprintCallable, Category = "Zom")
	const UZombieTypeData* GetZombieTypeData() const { return ZombieTypeData; }

	// Seeds Health/MoveSpeed/AttackDamage from InTypeData and tells the possessing AZomZombieAIController to
	// reconfigure perception for it. Called once from BeginPlay for the editor-assigned default type, and
	// again by UZomZombiePoolSubsystem whenever a pooled instance is reactivated as a (possibly different)
	// type - a pooled actor's BeginPlay only runs once, so re-seeding on every activation has to be a
	// separate, explicitly re-callable path (Section 4.1/5.1).
	UFUNCTION(BlueprintCallable, Category = "Zom")
	void InitializeForType(UZombieTypeData* InTypeData);

protected:
	virtual void BeginPlay() override;

	virtual void HandleDeath() override;

	// Bound to ZombieAttributeSet->OnDamageTaken; reports the hit to AI perception (UAISense_Damage) so the
	// possessing AZomZombieAIController can react even if the instigator is outside sight/hearing range.
	void HandleDamageTaken(AActor* DamageInstigator, float Amount);

	// -------------
	// Components
	// -------------
	// The ASC subobject created below is assigned into the inherited AbilitySystemComponent pointer
	// (AZomCharacterBase) in the constructor - no separate member needed for it.

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zom", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UZomZombieAttributeSet> ZombieAttributeSet;

	// -------------
	// Properties
	// -------------

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zom")
	TObjectPtr<UZombieTypeData> ZombieTypeData;

	// Spawned on death if ZombieTypeData->Category is Bloater (Section 5.3).
	UPROPERTY(EditDefaultsOnly, Category = "Zom")
	TSubclassOf<AZomToxicGasVolume> ToxicGasVolumeClass;
};
