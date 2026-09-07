// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Zom/Characters/Base/ZomCharacterBase.h"
#include "Zom/Misc/ZomGameplayTags.h"
#include "MotionCombatSystem/Structs/MCS_AttackEntry.h"
#include "MotionCombatSystem/Structs/MCS_AttackHitbox.h"
#include "ZomPlayerCharacter.generated.h"


// Forward declarations

class AZomPlayerController;
class UMotionWarpingComponent;
class UGameplayCameraComponent;
class UZomInventoryComponent;
class UMCS_CombatHitboxComponent;


/**
 * AZomPlayerCharacter
 * The player character class for the Zom game.
 */
UCLASS(Blueprintable, meta = (DisplayName = "Zom Player Character"))
class ZOM_API AZomPlayerCharacter : public AZomCharacterBase
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
	FGameplayTagContainer GetCurrentCamera() const { return CurrentCamera; }

	// Mirrors AZomCharacterBase::GetHealth/GetMaxHealth, but for Stamina - which lives on UZomPlayerAttributeSet
	// (player-only, per Section 4.1 of the dev doc) rather than the shared UZomAttributeSetBase, so these can't
	// live on the base class alongside Health/MaxHealth.
	UFUNCTION(BlueprintCallable, Category = "Zom")
	float GetStamina() const;

	UFUNCTION(BlueprintCallable, Category = "Zom")
	float GetMaxStamina() const;

	// -------------
	// Combat Events
	// -------------
	// Forwarded from the CombatCoreComponent delegates bound in BeginPlay (see BindCombatCoreEvents). Pure
	// Blueprint hooks - no C++ gameplay logic yet - so designers can wire up VFX/SFX/reactions per event.
	// OnCameraControl isn't here: it's handled directly in C++ (HandleCombatCameraControl updates CurrentCamera).

	UFUNCTION(BlueprintImplementableEvent, Category = "Zom|Combat", meta = (DisplayName = "On Attack Start"))
	void OnAttackStart();

	UFUNCTION(BlueprintImplementableEvent, Category = "Zom|Combat", meta = (DisplayName = "On Attack End"))
	void OnAttackEnd();

	UFUNCTION(BlueprintImplementableEvent, Category = "Zom|Combat", meta = (DisplayName = "On Hitbox Window Begin"))
	void OnHitboxWindowBegin(AActor* Attacker, const FMCS_AttackEntry& Attack, const TArray<FMCS_AttackHitbox>& Hitboxes);

	UFUNCTION(BlueprintImplementableEvent, Category = "Zom|Combat", meta = (DisplayName = "On Hitbox Window End"))
	void OnHitboxWindowEnd(AActor* Attacker);

	UFUNCTION(BlueprintImplementableEvent, Category = "Zom|Combat", meta = (DisplayName = "On Combo Window Begin"))
	void OnComboWindowBegin();

	UFUNCTION(BlueprintImplementableEvent, Category = "Zom|Combat", meta = (DisplayName = "On Combo Window End"))
	void OnComboWindowEnd();

	UFUNCTION(BlueprintImplementableEvent, Category = "Zom|Combat", meta = (DisplayName = "On Parry Window Begin"))
	void OnParryWindowBegin(AActor* Attacker);

	UFUNCTION(BlueprintImplementableEvent, Category = "Zom|Combat", meta = (DisplayName = "On Parry Window End"))
	void OnParryWindowEnd(AActor* Attacker);

	UFUNCTION(BlueprintImplementableEvent, Category = "Zom|Combat", meta = (DisplayName = "On Defense Window Begin"))
	void OnDefenseWindowBegin(AActor* Defender);

	UFUNCTION(BlueprintImplementableEvent, Category = "Zom|Combat", meta = (DisplayName = "On Defense Window End"))
	void OnDefenseWindowEnd(AActor* Defender);

	// -------------
	// Properties
	// -------------

	// Current camera state. Read every RunCameraDirector tick by CDE_Player's Chooser Table (CHT_CurrentCamera)
	// to pick which entry of its CameraRigsByTag map to activate. Set by whichever gameplay system owns a given
	// camera state (e.g. aiming, targeting) - defaults to TAG_Zom_Camera_State_Default.
	// FGameplayTagContainer rather than a bare FGameplayTag because Chooser Table Gameplay Tag columns can only
	// bind to FGameplayTagContainer properties - see https://forums.unrealengine.com/t/2668708 (UE-324898).
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Zom|Camera State", meta = (DisplayName = "Current Camera"))
	FGameplayTagContainer CurrentCamera = FGameplayTagContainer(TAG_Zom_Camera_State_Default.GetTag());

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

	// Binds the Handle* functions below to GetCombatCoreComponent()'s delegates. Called from BeginPlay, not
	// PossessedBy/InitializeAbilitySystem - unlike OnAttackResolved (AZomCharacterBase), none of these events
	// need the ASC, only the CombatCoreComponent subobject, which is already valid by BeginPlay.
	void BindCombatCoreEvents();

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
	// Functions
	// -------------
	// Bound to GetCombatCoreComponent()'s delegates by BindCombatCoreEvents. Each just forwards to the matching
	// OnXxx BlueprintImplementableEvent above, except HandleCombatCameraControl, which updates CurrentCamera
	// directly - see that property's comment for why the game already treats CameraTag as camera-state input.

	UFUNCTION()
	void HandleCombatAttackStart();

	UFUNCTION()
	void HandleCombatAttackEnd();

	UFUNCTION()
	void HandleCombatHitboxWindowBegin(AActor* Attacker, const FMCS_AttackEntry& Attack, const TArray<FMCS_AttackHitbox>& Hitboxes);

	UFUNCTION()
	void HandleCombatHitboxWindowEnd(AActor* Attacker);

	UFUNCTION()
	void HandleCombatCameraControl(FGameplayTag CameraTag);

	UFUNCTION()
	void HandleCombatComboWindowBegin();

	UFUNCTION()
	void HandleCombatComboWindowEnd();

	UFUNCTION()
	void HandleCombatParryWindowBegin(AActor* Attacker);

	UFUNCTION()
	void HandleCombatParryWindowEnd(AActor* Attacker);

	UFUNCTION()
	void HandleCombatDefenseWindowBegin(AActor* Defender);

	UFUNCTION()
	void HandleCombatDefenseWindowEnd(AActor* Defender);

	// -------------
	// Properties
	// -------------

	// Cached reference to the player controller that possesses this character
	TWeakObjectPtr<AZomPlayerController> CachedPlayerController;
};
