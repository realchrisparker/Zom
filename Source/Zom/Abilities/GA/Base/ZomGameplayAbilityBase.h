// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "MotionCombatSystem/AnimNotifyStates/AnimNotifyState_MCSWindow.h"
#include "ZomGameplayAbilityBase.generated.h"


// Forward declarations

class AZomCharacterBase;
class UAnimMontage;
class UAnimNotify_AttackStart;
class UAnimNotify_AttackEnd;
class UAnimNotify_CameraControl;
class UMCS_CombatCoreComponent;
struct FMCS_AttackEntry;


/**
 * Shared ability base for all six Zom abilities. Sets InstancingPolicy/NetExecutionPolicy once,
 * consistent with the full-GAS decision to teach production patterns even though prediction goes unused in a
 * singleplayer build, and blocks activation while staggered so no individual ability class has to remember to
 * (Section 4.2 of the dev doc).
 */
UCLASS(meta = (DisplayName = "Zom Gameplay Ability Base"))
class ZOM_API UZomGameplayAbilityBase : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UZomGameplayAbilityBase();

	// Always unbinds any montage notify handlers left over from BindMontageNotifies, regardless of which path
	// ended the ability (normal completion, cancellation, or an external force-end) - a single cleanup point
	// rather than relying on every subclass's montage-task callbacks to remember to unbind.
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	// Returns the avatar actor executing this ability, cast to AZomCharacterBase - every Zom ability's avatar
	// is one, so this replaces a repeated Cast<AZomCharacterBase>(GetAvatarActorFromActorInfo()) in every
	// ActivateAbility override that needs it.
	TObjectPtr<AZomCharacterBase> GetOwningCharacter() const;

	// Returns the owning character's MCS CombatCoreComponent, or nullptr if there's no owning character or no CombatCoreComponent.
	TObjectPtr<UMCS_CombatCoreComponent> GetOwningCharacterCombatCoreComponent() const;

	// Returns the attack entry the owning character's MCS CombatCoreComponent most recently resolved (a
	// default-constructed FMCS_AttackEntry if there's no owning character or no CombatCoreComponent). Attack
	// abilities (LightAttack, HeavyAttack, future weapon-specific variants) read AttackMontage/MontageSection
	// off this - see UMCS_CombatCoreComponent::GetCurrentAttack()'s own doc comment on this exact usage.
	FMCS_AttackEntry GetCurrentAttackEntry() const;

	// Call once this ability's own montage task ends (completed, blended out, interrupted, or cancelled), if
	// it played a montage sourced from GetCurrentAttackEntry(). UMCS_CombatCoreComponent's own redundant
	// "belt-and-suspenders" end signal (bound via Montage_SetEndDelegate in PlayCurrentAttack, guarding against
	// the AttackEnd AnimNotify sitting in the blend-out tail and getting skipped) only fires on its own
	// Montage_Play call - it returns immediately once it hands off to GAS, so that safety net never runs for a
	// GAS-driven attack unless the ability replicates it here. OnAttackEnd is a public BlueprintAssignable
	// delegate, so broadcasting it from outside the component is legal.
	void NotifyAttackMontageEnded() const;

	// Binds this ability's handlers to every AttackStart/AttackEnd/CameraControl/MCSWindow notify placed on
	// Montage, so the OnXNotify hooks below fire while this ability's own montage task is running. Call once
	// the ability knows which montage it's about to play (e.g. right before ReadyForActivation() on the
	// PlayMontageAndWait task). Mirrors UMCS_CombatCoreComponent::BindNotifiesForMontage - the notify UObjects
	// live on the montage asset itself and are shared by every actor that plays it (not duplicated per
	// character), so each handler below self-filters via IsOwningCharacterPlayingBoundMontage() before invoking
	// the hook, rejecting notify firings sourced from another actor sharing the same montage asset.
	void BindMontageNotifies(UAnimMontage* Montage);

	// Unbinds every handler added by BindMontageNotifies. Also called unconditionally from EndAbility, so
	// subclasses don't need to call this themselves as long as BindMontageNotifies was used.
	void UnbindMontageNotifies();

private:
	// Returns true if the owning character's AnimInstance is currently playing BoundMontage. Guards every
	// Handle*Notify below against firing for another actor's playback of the same shared montage asset.
	bool IsOwningCharacterPlayingBoundMontage() const;

	UFUNCTION()
	void HandleAttackStartNotify(UAnimNotify_AttackStart* NotifyInstance);

	UFUNCTION()
	void HandleAttackEndNotify(UAnimNotify_AttackEnd* NotifyInstance);

	UFUNCTION()
	void HandleCameraControlNotify(UAnimNotify_CameraControl* NotifyInstance, FGameplayTag CameraTag);

	UFUNCTION()
	void HandleMCSNotifyBegin(EMCS_AnimEventType EventType, UAnimNotifyState_MCSWindow* NotifyInstance);

	UFUNCTION()
	void HandleMCSNotifyEnd(EMCS_AnimEventType EventType, UAnimNotifyState_MCSWindow* NotifyInstance);

	// Montage BindMontageNotifies was last called with - IsOwningCharacterPlayingBoundMontage checks against
	// this, and UnbindMontageNotifies clears it.
	TWeakObjectPtr<UAnimMontage> BoundMontage;

	// Handles for every notify instance currently bound, keyed by type so UnbindMontageNotifies can remove
	// exactly what BindMontageNotifies added - mirrors UMCS_CombatCoreComponent's BoundXNotifies arrays.
	UPROPERTY()
	TArray<TObjectPtr<UAnimNotify_AttackStart>> BoundAttackStartNotifies;

	UPROPERTY()
	TArray<TObjectPtr<UAnimNotify_AttackEnd>> BoundAttackEndNotifies;

	UPROPERTY()
	TArray<TObjectPtr<UAnimNotify_CameraControl>> BoundCameraControlNotifies;

	UPROPERTY()
	TArray<TObjectPtr<UAnimNotifyState_MCSWindow>> BoundMCSNotifies;
};
