// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "ZomPlayerCharacterBase.generated.h"


// Forward declarations

class UAbilitySystemComponent;
class UGameplayEffect;
class UZomAbilitySetData;
struct FOnAttributeChangeData;


/**
 * Shared base class for every humanoid in Zom (player, zombies, Boss). Declares IAbilitySystemInterface and
 * caches an AbilitySystemComponent pointer populated by each leaf class (see InitializeAbilitySystem);
 * external code (HUD widgets, ability code, GameplayEffect application) always resolves the ASC through
 * GetAbilitySystemComponent() without caring where the component physically lives.
 */
UCLASS(Blueprintable, meta=(DisplayName="Zom Player Character Base"))
class ZOM_API AZomPlayerCharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:

	// Sets default values for this character's properties
	AZomPlayerCharacterBase(const FObjectInitializer& ObjectInitializer);

	// -------------
	// Functions
	// -------------

	// Returns the cached AbilitySystemComponent pointer. Never re-resolves it; InitializeAbilitySystem populates it.
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// Convenience getters pulling from whichever UZomAttributeSetBase instance the cached ASC has registered, so
	// HUD widgets and damage logic don't need to care where the AttributeSet lives. Return 0 before the ASC is
	// initialized (only a real risk for the player - see the note on InitializeAbilitySystem's call sites).
	UFUNCTION(BlueprintCallable, Category = "Zom")
	float GetHealth() const;

	UFUNCTION(BlueprintCallable, Category = "Zom")
	float GetMaxHealth() const;

	// -------------
	// Properties
	// -------------

protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// -------------
	// Functions
	// -------------

	// Caches the AbilitySystemComponent resolved off OwnerActor and calls InitAbilityActorInfo(OwnerActor, AvatarActor).
	// For AI, OwnerActor == AvatarActor == self. For the player, OwnerActor is the PlayerState, AvatarActor is the pawn.
	void InitializeAbilitySystem(AActor* InOwnerActor, AActor* InAvatarActor);

	// Iterates the ability classes and starting effects on AbilitySetData and grants/applies them through the cached ASC.
	void GrantAbilitySet(const UZomAbilitySetData* AbilitySetData);

	// Small wrapper around the MakeOutgoingSpec/ApplyGameplayEffectSpecToSelf boilerplate for self-applied effects.
	void ApplyGameplayEffectToSelf(TSubclassOf<UGameplayEffect> EffectClass, float Level = 1.f);

	// Fires once Health reaches zero (bound to the Health attribute-changed delegate in InitializeAbilitySystem).
	// Empty at this level; each subclass overrides it for actor-level death consequences (Section 4.6).
	virtual void HandleDeath();

	// -------------
	// Properties
	// -------------

	// Cached AbilitySystemComponent pointer. Owned elsewhere (AZomPlayerState for the player, a real subobject for AI);
	// this class never creates one itself.
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

private:

	// Bound to the Health attribute-changed delegate; calls HandleDeath() once Health reaches zero. One shared
	// trigger point on the base, per Section 4.6, rather than each subclass polling Health independently.
	void OnHealthAttributeChanged(const FOnAttributeChangeData& Data);
};
