// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayAbilitySpecHandle.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameFramework/Character.h"
#include "Zom/Characters/Enums/ZomCharacterEnums.h"
#include "ZomCharacterBase.generated.h"


// Forward declarations

class UAbilitySystemComponent;
class UGameplayEffect;
class UZomGameplayAbility;
class UZomGameplayEffect;
struct FOnAttributeChangeData;
class UMCS_CombatCoreComponent;
class UMCS_CombatHitboxComponent;
class UMCS_CombatHitReactionComponent;
class UMCS_CombatDefenseComponent;


/**
 * Shared base class for every humanoid in Zom (player, zombies, Boss). Declares IAbilitySystemInterface and
 * caches an AbilitySystemComponent pointer populated by each leaf class (see InitializeAbilitySystem);
 * external code (HUD widgets, ability code, GameplayEffect application) always resolves the ASC through
 * GetAbilitySystemComponent() without caring where the component physically lives.
 */
UCLASS(Blueprintable, meta=(DisplayName="Zom Character Base"))
class ZOM_API AZomCharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:

	// Sets default values for this character's properties
	AZomCharacterBase(const FObjectInitializer& ObjectInitializer);

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

	// Returns the cached combat components.
	UFUNCTION(BlueprintCallable, Category = "Zom|Combat", meta = (DisplayName = "Get Combat Core Component"))
	UMCS_CombatCoreComponent* GetCombatCoreComponent() const { return CombatCoreComponent; }

	UFUNCTION(BlueprintCallable, Category = "Zom|Combat", meta = (DisplayName = "Get Combat Hitbox Component"))
	UMCS_CombatHitboxComponent* GetCombatHitboxComponent() const { return CombatHitboxComponent; }

	UFUNCTION(BlueprintCallable, Category = "Zom|Combat", meta = (DisplayName = "Get Combat Hit Reaction Component"))
	UMCS_CombatHitReactionComponent* GetCombatHitReactionComponent() const { return CombatHitReactionComponent; }

	UFUNCTION(BlueprintCallable, Category = "Zom|Combat", meta = (DisplayName = "Get Combat Defense Component"))
	UMCS_CombatDefenseComponent* GetCombatDefenseComponent() const { return CombatDefenseComponent; }

	// Returns the current attack situation, which is used to determine which attacks are valid for the character.
	UFUNCTION(BlueprintCallable, Category = "Zom|Combat", meta = (DisplayName = "Get Current Attack Situation"))
	FMCS_AttackSituation GetCurrentAttackSituation() const;

	// Grants a single ability class to the ASC if not already granted (idempotent - re-adding a class already
	// present is a safe no-op, not a duplicate spec). Server-only (no-op off authority). Returns true if the
	// ability is granted after the call (whether newly granted or already present).
	UFUNCTION(BlueprintCallable, Category = "Zom|Abilities")
	bool AddAbility(TSubclassOf<UZomGameplayAbility> AbilityClass);

	// Revokes a single previously-AddAbility'd ability class. No-op (returns false) if not granted.
	UFUNCTION(BlueprintCallable, Category = "Zom|Abilities")
	bool RemoveAbility(TSubclassOf<UZomGameplayAbility> AbilityClass);

	// Applies a single effect class to self if not already active via this API (idempotent w.r.t. this API's
	// own bookkeeping only - it doesn't prevent other code from independently applying the same effect class).
	UFUNCTION(BlueprintCallable, Category = "Zom|Abilities")
	bool AddEffect(TSubclassOf<UZomGameplayEffect> EffectClass, float Level = 1.f);

	// Removes an effect previously applied via AddEffect. No-op/false if not tracked, or if the effect was
	// Instant (Instant effects have no active handle to remove - that's expected, not an error).
	UFUNCTION(BlueprintCallable, Category = "Zom|Abilities")
	bool RemoveEffect(TSubclassOf<UZomGameplayEffect> EffectClass);

	// Fired at the end of InitializeAbilitySystem, once the cached ASC is actually valid - the earliest point
	// at which AddAbility/AddEffect can succeed. Call them from here, not from BeginPlay/Event Possessed:
	// those Blueprint events fire from inside Super::BeginPlay()/Super::PossessedBy(), before this class's own
	// InitializeAbilitySystem call later in the same function has run, so AbilitySystemComponent is still null.
	UFUNCTION(BlueprintImplementableEvent, Category = "Zom|Abilities", meta = (DisplayName = "On Ability System Initialized"))
	void OnAbilitySystemInitialized();

	// -------------
	// Properties
	// -------------

	// Current gait. Shared by every AZomCharacterBase subclass (player, zombies, Boss) so AI-driven
	// enemies can drive the same locomotion enum the player's input does, without needing their own copy.
	// For the player: set manually by AZomPlayerController in response to the IA_WalkRun toggle and the
	// IA_Sprint hold - not inferred from speed, to avoid Gait flickering (and the motion matching Chooser
	// jumping with it) when velocity hovers near a speed threshold.
	UPROPERTY(BlueprintReadWrite, Category = "Zom|Movement State", meta = (DisplayName = "Gait"))
	EGait Gait = EGait::Walk;

	// Current stance. Kept in sync with the movement component's crouched state via OnStartCrouch/OnEndCrouch
	// below, so it updates only on an actual stance change rather than being recomputed every animation tick.
	UPROPERTY(BlueprintReadWrite, Category = "Zom|Movement State", meta = (DisplayName = "Stance"))
	EStance Stance = EStance::Stand;

	// Current combat state. Unarmed is the default state; other states are set manually (by AZomPlayerController,
	// in response to combat input actions, for the player) or by AI/ability logic for other subclasses.
	UPROPERTY(BlueprintReadWrite, Category = "Zom|Combat", meta = (DisplayName = "Combat State"))
	ECombatState CombatState = ECombatState::Unarmed;

protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Called when the character starts crouching; keeps Stance in sync
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	// Called when the character stops crouching; keeps Stance in sync
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	// -------------
	// Functions
	// -------------

	// Caches the AbilitySystemComponent resolved off OwnerActor and calls InitAbilityActorInfo(OwnerActor, AvatarActor).
	// For AI, OwnerActor == AvatarActor == self. For the player, OwnerActor is the PlayerState, AvatarActor is the pawn.
	void InitializeAbilitySystem(AActor* InOwnerActor, AActor* InAvatarActor);

	// Small wrapper around the MakeOutgoingSpec/ApplyGameplayEffectSpecToSelf boilerplate for self-applied effects.
	// Returns the resulting FActiveGameplayEffectHandle (invalid for Instant effects, which have no ongoing
	// active instance to hand back - that's expected, not an error).
	FActiveGameplayEffectHandle ApplyGameplayEffectToSelf(TSubclassOf<UGameplayEffect> EffectClass, float Level = 1.f);

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

	// Handles for every ability currently granted via AddAbility, keyed by class so AddAbility/RemoveAbility can
	// check "already granted?" in O(1) and so double-adding the same class is a safe no-op rather than a
	// stacked duplicate spec. Plain members (not UPROPERTY) - the handle structs hold no UObject* for GC to track.
	TMap<TSubclassOf<UZomGameplayAbility>, FGameplayAbilitySpecHandle> GrantedAbilityHandles;

	// Handles for every effect currently active via AddEffect, keyed by class for the same reason. Instant
	// effects get an entry with an invalid handle so a second AddEffect(SameClass) call is still recognized as
	// "already applied via this API" and no-ops instead of re-applying.
	TMap<TSubclassOf<UZomGameplayEffect>, FActiveGameplayEffectHandle> ActiveEffectHandles;

	// -------------
	// Components
	// -------------

	// Motion Combat System components. Every AZomCharacterBase subclass (player, zombies, Boss) gets the full
	// combat component set as real subobjects, so both AI and player share the same attack/hitbox/defense pipeline.

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zom|Combat", meta = (DisplayName = "MCS_CombatCoreComponent"))
	TObjectPtr<UMCS_CombatCoreComponent> CombatCoreComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zom|Combat", meta = (DisplayName = "MCS_CombatHitboxComponent"))
	TObjectPtr<UMCS_CombatHitboxComponent> CombatHitboxComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zom|Combat", meta = (DisplayName = "MCS_CombatHitReactionComponent"))
	TObjectPtr<UMCS_CombatHitReactionComponent> CombatHitReactionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zom|Combat", meta = (DisplayName = "MCS_CombatDefenseComponent"))
	TObjectPtr<UMCS_CombatDefenseComponent> CombatDefenseComponent;

private:

	// Bound to the Health attribute-changed delegate; calls HandleDeath() once Health reaches zero. One shared
	// trigger point on the base, per Section 4.6, rather than each subclass polling Health independently.
	void OnHealthAttributeChanged(const FOnAttributeChangeData& Data);
};
