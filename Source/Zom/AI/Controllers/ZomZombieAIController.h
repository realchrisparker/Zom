// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "ZomZombieAIController.generated.h"


class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;
class UAISenseConfig_Damage;
class UStateTreeAIComponent;
class UZombieTypeData;
struct FAIStimulus;


/**
 * [Design] Custom AIController for crowd zombies (Section 5.5 of the dev doc, revised): owns perception and
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
	bool HasValidTarget() const;

	UFUNCTION(BlueprintCallable, Category = "Zom|AI")
	AActor* GetCurrentTarget() const;

	UFUNCTION(BlueprintCallable, Category = "Zom|AI")
	FVector GetLastKnownTargetLocation() const;

protected:
	UFUNCTION()
	void HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

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
};
