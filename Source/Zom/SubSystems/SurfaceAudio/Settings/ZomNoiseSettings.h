// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ZomNoiseSettings.generated.h"


class UZomSurfaceAudioSet;


/**
 * Project Settings > Game > Zom Stealth. Project-wide tuning for the noise a character emits and how far it
 * carries. Per-character knobs (footwear, gait weights, stride length) live on UZomCharacterNoiseComponent
 * instead, so two characters can be loud or quiet independently of these level-wide constants.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Zom Stealth"))
class ZOM_API UZomNoiseSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UZomNoiseSettings();

	// Per-surface loudness and footstep sound tags. Loaded and flattened once by UZomSurfaceAudioSubsystem;
	// nothing queries the asset itself at runtime. Unset means every surface falls back to a hardcoded
	// audible default, so the system still works (and still sounds like something) on an unconfigured project.
	UPROPERTY(Config, EditAnywhere, Category = "Surfaces", meta = (DisplayName = "Surface Audio Set"))
	TSoftObjectPtr<UZomSurfaceAudioSet> SurfaceAudioSet;

	// The hearing radius a "baseline" zombie is assumed to have, used to convert an authored noise radius
	// into the Loudness UAISense_Hearing wants (see UZomNoiseLibrary::NoiseDistanceToLoudness).
	//
	// KEEP THIS EQUAL TO THE BASELINE UZombieTypeData::HearingRadius (the Walker's, currently 600). The
	// conversion is a straight division by this number, so if the two drift apart every computed radius -
	// and every debug ring drawn from it - is silently wrong by a constant factor.
	UPROPERTY(Config, EditAnywhere, Category = "Hearing", meta = (DisplayName = "Reference Hearing Range", ClampMin = "1", Units = "cm"))
	float ReferenceHearingRange = 600.f;

	// Audible radius of a footstep at a noise level of 1.0 (sprinting on the loudest surface). Every quieter
	// step is a fraction of this, so it sets the overall scale of movement stealth in one number.
	UPROPERTY(Config, EditAnywhere, Category = "Noise", meta = (DisplayName = "Footstep Distance Multiplier", ClampMin = "0", Units = "cm"))
	float FootstepDistanceMultiplier = 1200.f;

	// Noises that would carry less than this are dropped without reporting anything, so an inaudible
	// crouch-step on carpet never costs the perception system an event it would discard anyway.
	UPROPERTY(Config, EditAnywhere, Category = "Noise", meta = (DisplayName = "Min Noise Distance", ClampMin = "0", Units = "cm"))
	float MinNoiseDistance = 50.f;

	// Project default for the noise debug rings. Per-Blueprint overrides live on the component, and the
	// Zom.Debug.Noise console variable overrides both at runtime.
	UPROPERTY(Config, EditAnywhere, Category = "Debug", meta = (DisplayName = "Draw Noise Debug"))
	bool bDrawNoiseDebug = false;
};
