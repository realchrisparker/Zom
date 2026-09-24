// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Zom/Objectives/ZomObjectiveEnums.h"
#include "ZomObjectiveSubsystem.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnZomObjectiveStepCompleted, EZomObjectiveStep, CompletedStep);


/**
 * Owns the objective state machine, broadcasts delegates on state change, and mirrors progress onto
 * the player's ASC as Gameplay Tags (Zom.Objective.*.Complete, see Section 4.4) so abilities/effects can gate
 * on it. UGameInstanceSubsystem so it persists across OpenLevel calls, required since checkpoints reload the
 * level (Section 8).
 */
UCLASS()
class ZOM_API UZomObjectiveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Zom|Objectives")
	FOnZomObjectiveStepCompleted OnStepCompleted;

	// Advances CurrentStep to Step and mirrors it onto the player's ASC as the matching Zom.Objective.*.Complete
	// tag - both together, atomically, per the doc's explicit "never one without the other."
	UFUNCTION(BlueprintCallable, Category = "Zom|Objectives")
	void CompleteStep(EZomObjectiveStep Step);

	UFUNCTION(BlueprintPure, Category = "Zom|Objectives")
	EZomObjectiveStep GetCurrentStep() const { return CurrentStep; }

	UFUNCTION(BlueprintPure, Category = "Zom|Objectives")
	bool IsStepComplete(EZomObjectiveStep Step) const { return CompletedSteps.Contains(Step); }

	// Used by the save/restore flow (Section 11) to resume from a saved step - read explicitly every time
	// rather than trusting in-memory state, per the doc. Marks Step and every step before it complete (mirroring
	// their tags too) without broadcasting OnStepCompleted for each, since this isn't a live completion event.
	UFUNCTION(BlueprintCallable, Category = "Zom|Objectives")
	void RestoreStep(EZomObjectiveStep Step);

	// Clears all progress back to the first step, for New Game. The subsystem outlives level travel, so without
	// this a new run would inherit the previous run's CompletedSteps. Tags need no clearing - the next level
	// spawns a fresh pawn/ASC.
	UFUNCTION(BlueprintCallable, Category = "Zom|Objectives")
	void ResetObjectives();

private:
	void MirrorTagForStep(EZomObjectiveStep Step) const;
	static FGameplayTag GetCompletionTagForStep(EZomObjectiveStep Step);

	EZomObjectiveStep CurrentStep = EZomObjectiveStep::Fetch;
	TSet<EZomObjectiveStep> CompletedSteps;
};
