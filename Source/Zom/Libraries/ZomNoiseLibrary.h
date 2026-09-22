// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ZomNoiseLibrary.generated.h"


/**
 * The noise maths behind Zom's stealth, kept as standalone pure functions rather than methods on
 * UZomCharacterNoiseComponent so anything can use them: a footstep, a gunshot ability, a thrown bottle, a
 * breaking window. Only the footstep path happens to own a component.
 *
 * Everything here works in a normalized 0-1 "noise level", NOT decibels. The engine's hearing sense treats
 * Loudness as a linear multiplier on a radius, and locomotion weight composes with surface loudness by
 * multiplication, so a linear scale is one multiply where a logarithmic one would be an exp per footstep
 * undoing a log added only for flavour. NoiseLevelToDecibels exists for HUD text and nothing else.
 */
UCLASS()
class ZOM_API UZomNoiseLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	// How loud an action is, before distance: the actor's locomotion weight scaled by what it is standing on.
	// Standalone and pure because both inputs vary per character - footwear changes the locomotion weight
	// (sneakers quieter, boots louder), and the surface comes from whatever the caller is standing on. A
	// non-movement emitter (a gunshot) simply passes 1.0 for both and lets the distance multiplier do the work.
	UFUNCTION(BlueprintPure, Category = "Zom|Noise", meta = (DisplayName = "Calculate Noise Level"))
	static float CalculateNoiseLevel(float LocomotionWeight, float SurfaceLoudness, float MaxNoiseLevel = 1.f);

	// How far that noise carries, in cm. DistanceMultiplier IS the audible radius at a noise level of 1, so
	// it reads directly off the call site: footsteps around 1200, a gunshot in an enclosed hallway 6000+.
	// Separated from the level so the same loudness can carry very differently by context - the multiplier
	// is what makes a gunshot indoors pull zombies from rooms the player has never seen.
	UFUNCTION(BlueprintPure, Category = "Zom|Noise", meta = (DisplayName = "Calculate Noise Distance"))
	static float CalculateNoiseDistance(float NoiseLevel, float DistanceMultiplier);

	// Converts an authored audible radius into the Loudness UAISense_Hearing actually wants.
	//
	// The engine gates on Dist <= Listener.HearingRange * Loudness, and multiplies MaxRange by Loudness too,
	// so MaxRange cannot express an absolute cap and Loudness has to carry the whole radius. Dividing by the
	// project's reference range makes a baseline-hearing zombie hear at exactly NoiseDistance, while a keener
	// type hears proportionally further - which is what keeps UZombieTypeData::HearingRadius meaningful as a
	// per-type sensitivity rather than a hard ceiling that would clamp every loud noise to 600 uu.
	UFUNCTION(BlueprintPure, Category = "Zom|Noise", meta = (DisplayName = "Noise Distance To Loudness"))
	static float NoiseDistanceToLoudness(float NoiseDistance, float ReferenceHearingRange);

	// The inverse, for the listening side: turns FAIStimulus::Strength back into the radius the emitter
	// authored, in cm. This is how far a BASELINE zombie hears the noise - not how far this particular
	// listener does, which is scaled by its own HearingRadius / ReferenceHearingRange. Lets AI code reason
	// in world units ("a 6000 uu gunshot") instead of in loudness ratios.
	UFUNCTION(BlueprintPure, Category = "Zom|Noise", meta = (DisplayName = "Loudness To Noise Distance"))
	static float LoudnessToNoiseDistance(float Loudness, float ReferenceHearingRange);

	// The one place in the project that reports a noise to the AI. Returns false without reporting anything
	// when the noise is too quiet to matter, so an inaudible crouch-step costs the perception system nothing.
	UFUNCTION(BlueprintCallable, Category = "Zom|Noise", meta = (DisplayName = "Report Noise", WorldContext = "WorldContextObject", AdvancedDisplay = "MinNoiseDistance"))
	static bool ReportNoise(const UObject* WorldContextObject, FVector NoiseLocation, float NoiseDistance, AActor* Instigator, FGameplayTag NoiseTag, float MinNoiseDistance = 50.f);

	// Display only. The pipeline is linear 0-1 throughout; this exists so a HUD or debug string can show a
	// familiar-looking number, never so gameplay code can do arithmetic in dB.
	UFUNCTION(BlueprintPure, Category = "Zom|Noise", meta = (DisplayName = "Noise Level To Decibels"))
	static float NoiseLevelToDecibels(float NoiseLevel);
};
