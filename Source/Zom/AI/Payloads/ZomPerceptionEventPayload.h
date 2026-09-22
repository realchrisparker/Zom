// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ZomPerceptionEventPayload.generated.h"


/**
 * Payload carried by the Zom.Perception.* tagged State Tree events UZomZombieAIComponent raises
 * (Section 5.5 of the dev doc). Wrapped via FConstStructView::Make() when calling SendStateTreeEvent.
 */
USTRUCT(BlueprintType)
struct FZomPerceptionEventPayload
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Zom|Perception")
	TWeakObjectPtr<AActor> SensedActor;

	UPROPERTY(BlueprintReadOnly, Category = "Zom|Perception")
	FVector Location = FVector::ZeroVector;

	// Hearing only: the loudness the noise was reported with, straight off FAIStimulus::Strength. NOT
	// normalized - UZomNoiseLibrary reports a radius divided by the reference hearing range, so a gunshot
	// legitimately arrives as 10.0. Compare it against other noises, never against 1.
	UPROPERTY(BlueprintReadOnly, Category = "Zom|Perception")
	float Strength = 0.f;

	// Hearing only: the radius the noise was emitted with, in cm - Strength converted back to world units by
	// UZomNoiseLibrary::LoudnessToNoiseDistance. This is how far a baseline-hearing zombie hears it, so a
	// tree can compare noises in real distances (a 300 uu footstep vs a 6000 uu gunshot). The distance from
	// this zombie to the noise is separate: measure it against Location.
	UPROPERTY(BlueprintReadOnly, Category = "Zom|Perception")
	float NoiseDistance = 0.f;

	// Hearing only: which kind of noise this was (Zom.Noise.Footstep, Zom.Noise.Gunshot, ...), rebuilt from
	// FAIStimulus::Tag. Lets a State Tree treat a gunshot differently from a footstep without a second query.
	UPROPERTY(BlueprintReadOnly, Category = "Zom|Perception")
	FGameplayTag NoiseTag;
};
