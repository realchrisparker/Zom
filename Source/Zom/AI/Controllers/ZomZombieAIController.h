// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
// Not forward-declarable: the inline GetCurrentTarget() below upcasts ACharacter* -> AActor*, and a
// derived-to-base pointer conversion needs the complete type. AIController.h doesn't pull Character.h in,
// so without this the accessor only compiles by luck of unity-build ordering.
#include "GameFramework/Character.h"
#include "ZomZombieAIController.generated.h"


class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;
class UAISenseConfig_Damage;
class UStateTreeAIComponent;
class UZombieTypeData;
struct FAIStimulus;


/**
 * Custom AIController for crowd zombies (Section 5.5 of the dev doc, revised): owns perception and
 * the State Tree, rather than a UActorComponent living on the pawn. This matches Epic's own intended pattern -
 * UStateTreeAIComponent/UStateTreeAIComponentSchema are explicitly "designed to be run on an AIController" and
 * guarantee State Tree bindings access to the controller (and, through it, the possessed pawn). Perception
 * lives here too since it's a "brain" concept, not a "body" one, and this AIController is what raises the
 * tagged State Tree events derived from it.
 *
 * Three senses: Sight and Hearing are radius-based (configured per UZombieTypeData in ConfigureForType);
 * Damage has no radius - it's an explicit UAISense_Damage::ReportDamageEvent() call, made by AZomZombieBase
 * whenever its UZomZombieAttributeSet::OnDamageTaken fires, so a zombie can react to being shot/hit even from
 * outside sight/hearing range.
 */
UCLASS()
class ZOM_API AZomZombieAIController : public AAIController
{
	GENERATED_BODY()

public:
	AZomZombieAIController();

	// -------------
	// Functions
	// -------------

	virtual void OnPossess(APawn* InPawn) override;

	// Perception reads the *listener's* team off this controller (it owns the perception component) and a
	// sensed target's team off the pawn, so the two have to agree. Rather than storing a copy that can drift,
	// this defers to the possessed pawn's own AZomCharacterBase::GetGenericTeamId(), which is itself derived
	// from its faction tag - one authored source of truth for the whole chain.
	virtual FGenericTeamId GetGenericTeamId() const override;

	// Unbinds this controller from UMCS_CombatEventBus (bound in OnPossess) so a stale callback can't fire
	// into a StateTreeComponent that's about to go away.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Configures the perception senses off TypeData's radii/angle. Called from OnPossess for the zombie's
	// initial type, and again by UZomZombiePoolSubsystem whenever a pooled instance is reactivated as a
	// (possibly different) type - the concrete implementation behind Section 5.2's "Auds/Eyes are the same
	// component with one sense's config zeroed out" claim.
	void ConfigureForType(const UZombieTypeData* TypeData);

	// Pauses/resumes the State Tree logic. Called by UZomZombiePoolSubsystem on release/reactivation so a
	// hidden pooled zombie's brain doesn't keep running (the controller itself is never destroyed/re-spawned,
	// only the pawn is hidden - see Section 9).
	void PauseBrain();
	void ResumeBrain();

	// Backs property bindings for tasks in an already-active State Tree state (e.g. Chase needs the target's
	// current location every tick it's active). Distinct from the perception events, which drive transitions.
	UFUNCTION(BlueprintCallable, Category = "Zom|AI")
	bool HasValidTarget() const { return CurrentTarget.IsValid(); };

	UFUNCTION(BlueprintCallable, Category = "Zom|AI")
	AActor* GetCurrentTarget() const { return CurrentTarget.Get(); };

	UFUNCTION(BlueprintCallable, Category = "Zom|AI")
	FVector GetLastKnownTargetLocation() const { return LastKnownTargetLocation; };

	// Deliberately separate from the target getters above. A heard noise is a place worth investigating, not
	// a confirmed target - hearing does no line-of-sight check, so a noise through a wall must not read as
	// "I can see you". An Investigate state binds to these; Chase binds to the target getters.
	UFUNCTION(BlueprintCallable, Category = "Zom|AI")
	bool HasHeardNoise() const { return bHasHeardNoise; };

	UFUNCTION(BlueprintCallable, Category = "Zom|AI")
	FVector GetLastHeardNoiseLocation() const { return LastHeardNoiseLocation; };

	// Called by an Investigate state once it has reached the noise and found nothing, so the zombie does not
	// keep re-investigating the same spot.
	UFUNCTION(BlueprintCallable, Category = "Zom|AI")
	void ClearHeardNoise();

protected:
	UFUNCTION()
	void HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	// Bound to UMCS_CombatEventBus in OnPossess (see ZomZombieAIController.cpp) so combat state can drive
	// this zombie's own State Tree the same way perception does - each relays into a Zom.Combat.Event.* tagged
	// SendStateTreeEvent, but only once filtered down to what's actually relevant to this controller.

	// Relayed only if Defender is this controller's own CurrentTarget - "the actor I'm pursuing is currently
	// guarding" is the actionable signal (e.g. hold back / bait instead of attacking).
	UFUNCTION()
	void HandleDefenseWindowOpened(AActor* Defender, float Duration);

	UFUNCTION()
	void HandleDefenseWindowClosed(AActor* Defender);

	// Relayed only if Attacker is this controller's own possessed pawn - "MY attack just got parried/blocked"
	// is the actionable signal (e.g. disengage/reposition), not just "someone got parried somewhere."
	UFUNCTION()
	void HandleParrySuccess(AActor* Defender, AActor* Attacker);

	UFUNCTION()
	void HandleBlockSuccess(AActor* Defender, AActor* Attacker);

	// -------------
	// Components
	// -------------
	// Perception subobject is assigned into AAIController's own inherited PerceptionComponent pointer via
	// SetPerceptionComponent() in the constructor - no separate member needed for it (AAIController already
	// declares one; redeclaring it here is a UHT shadowing error).

	// Bespoke per Section 3/5.5 - the crowd State Tree asset is assigned on this component (content, not C++).
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zom|AI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStateTreeAIComponent> StateTreeComponent;

	UPROPERTY()
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	UPROPERTY()
	TObjectPtr<UAISenseConfig_Hearing> HearingConfig;

	UPROPERTY()
	TObjectPtr<UAISenseConfig_Damage> DamageConfig;

private:

	// Current target.
	TWeakObjectPtr<ACharacter> CurrentTarget;

	// Last known location of the current target, updated on perception events. Used to back State Tree property
	FVector LastKnownTargetLocation = FVector::ZeroVector;

	// Where the last noise came from, and whether there is one outstanding. Separate from the target fields
	// above because hearing is unoccluded: this is somewhere to go and look, not someone that has been seen.
	FVector LastHeardNoiseLocation = FVector::ZeroVector;
	bool bHasHeardNoise = false;
};
