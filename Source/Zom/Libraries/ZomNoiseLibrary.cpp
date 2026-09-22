// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Libraries/ZomNoiseLibrary.h"
#include "Perception/AISense_Hearing.h"
#include "Zom/Misc/ZomLogChannels.h"
#include "Zom/SubSystems/SurfaceAudio/Settings/ZomNoiseSettings.h"


// Locomotion weight scaled by what the character is standing on.
float UZomNoiseLibrary::CalculateNoiseLevel(float LocomotionWeight, float SurfaceLoudness, float MaxNoiseLevel)
{
	return FMath::Clamp(LocomotionWeight * SurfaceLoudness, 0.f, FMath::Max(MaxNoiseLevel, 0.f));
}

// How far that noise carries.
float UZomNoiseLibrary::CalculateNoiseDistance(float NoiseLevel, float DistanceMultiplier)
{
	return FMath::Max(NoiseLevel * DistanceMultiplier, 0.f);
}

// Converts an authored radius into the Loudness the hearing sense wants.
float UZomNoiseLibrary::NoiseDistanceToLoudness(float NoiseDistance, float ReferenceHearingRange)
{
	// Guard the divide rather than trusting the settings clamp: a Blueprint caller can pass anything.
	return NoiseDistance / FMath::Max(ReferenceHearingRange, 1.f);
}

// Turns a received loudness back into the emitter's authored radius.
float UZomNoiseLibrary::LoudnessToNoiseDistance(float Loudness, float ReferenceHearingRange)
{
	// Same guard as the forward conversion, so the pair round-trips exactly for any settings value.
	return FMath::Max(Loudness, 0.f) * FMath::Max(ReferenceHearingRange, 1.f);
}

// The one place in the project that reports a noise to the AI.
bool UZomNoiseLibrary::ReportNoise(const UObject* WorldContextObject, FVector NoiseLocation, float NoiseDistance, AActor* Instigator, FGameplayTag NoiseTag, float MinNoiseDistance)
{
	// Instigator has to be valid, not just for tidiness: FAINoiseEvent::Compile() reads the team off it, and
	// the perception system keys the resulting stimulus BY it. A null instigator still passes the affiliation
	// filter as Neutral, then gets thrown away by the listener's own null check - an event that can never be
	// acted on. Failing here instead makes that a caller bug rather than a silent no-op.
	if (!WorldContextObject || !Instigator || NoiseDistance < MinNoiseDistance)
	{
		return false;
	}

	const UZomNoiseSettings* Settings = GetDefault<UZomNoiseSettings>();
	const float ReferenceRange = Settings ? Settings->ReferenceHearingRange : 600.f;
	const float Loudness = NoiseDistanceToLoudness(NoiseDistance, ReferenceRange);

	// MaxRange is 0 deliberately. The engine multiplies MaxRange by Loudness as well, so it can only ever be
	// a second ceiling in loudness-scaled units - it cannot say "this sound stops at 6000 uu". Loudness alone
	// carries the radius, expressed relative to a baseline-hearing listener.
	//
	// The tag goes over as an FName because FAINoiseEvent::Tag is one; it survives into FAIStimulus::Tag, so
	// AZomZombieAIController can tell a footstep from a gunshot without any extra plumbing.
	UAISense_Hearing::ReportNoiseEvent(
		const_cast<UObject*>(WorldContextObject),
		NoiseLocation,
		Loudness,
		Instigator,
		/*MaxRange*/ 0.f,
		NoiseTag.IsValid() ? NoiseTag.GetTagName() : NAME_None);

	UE_LOG(LogZomAI, Verbose, TEXT("Noise '%s' at %s: radius %.0f uu -> Loudness %.3f (reference %.0f) from %s"),
		*NoiseTag.ToString(), *NoiseLocation.ToCompactString(), NoiseDistance, Loudness, ReferenceRange, *GetNameSafe(Instigator));

	return true;
}

// Display only.
float UZomNoiseLibrary::NoiseLevelToDecibels(float NoiseLevel)
{
	return 20.f * FMath::LogX(10.f, FMath::Max(NoiseLevel, UE_KINDA_SMALL_NUMBER));
}
