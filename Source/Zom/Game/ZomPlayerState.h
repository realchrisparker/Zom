// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "ZomPlayerState.generated.h"


// Forward declarations

class UAbilitySystemComponent;
class UZomPlayerAttributeSet;


/**
 * Owns the player's UAbilitySystemComponent as a real subobject. Implements IAbilitySystemInterface directly
 * (rather than only through AZomCharacterBase) since some GAS lookups resolve the ASC by calling the
 * interface on whatever actor they're handed, which is sometimes the PlayerState, not the pawn.
 */
UCLASS(Blueprintable, meta=(DisplayName="Zom Player State"))
class ZOM_API AZomPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AZomPlayerState();

	// -------------
	// Functions
	// -------------

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// -------------
	// Properties
	// -------------

protected:

	// The player's AbilitySystemComponent, owned here as a real subobject (Section 3 of the dev doc).
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zom", meta = (DisplayName = "AbilitySystemComponent", AllowPrivateAccess = "true"))
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	// Single attribute-set instance for the player. UZomPlayerAttributeSet already inherits Health/MaxHealth/
	// Damage/MoveSpeed/MaxMoveSpeed from UZomAttributeSetBase, so one subobject covers both roles - registering
	// a separate bare UZomAttributeSetBase alongside it would create two ASC-registered instances that both
	// IsA(UZomAttributeSetBase), making GAS's modifier-target resolution ambiguous (see Section 4.1).
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zom", meta = (DisplayName = "PlayerAttributeSet", AllowPrivateAccess = "true"))
	TObjectPtr<UZomPlayerAttributeSet> PlayerAttributeSet;
};
