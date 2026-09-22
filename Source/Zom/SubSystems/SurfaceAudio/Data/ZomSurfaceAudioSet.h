// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Chaos/ChaosEngineInterface.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "ZomSurfaceAudioSet.generated.h"


/**
 * One surface's noise and audio data. Loudness feeds UZomNoiseLibrary::CalculateNoiseLevel; FootstepSoundTag
 * only NAMES a row - the sound itself stays in the character's own UZomCharacterSoundSet, so the sound set
 * remains the single source of audio and two characters can sound different walking on the same floor.
 */
USTRUCT(BlueprintType)
struct ZOM_API FZomSurfaceNoiseEntry
{
	GENERATED_BODY()

	// Multiplier on the locomotion weight, so it scales how loud this surface is to the AI rather than
	// setting an absolute level. Gravel and metal ~1.0, concrete ~0.8, grass ~0.4, carpet ~0.2.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom|Surface", meta = (DisplayName = "Loudness", ClampMin = "0", ClampMax = "2"))
	float Loudness = 1.f;

	// Zom.Audio.Footstep.* row handed to UZomCharacterAudioComponent::PlayCharacterSoundAtLocation. Unset
	// falls back to the component's own default tag.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom|Surface", meta = (DisplayName = "Footstep Sound Tag", Categories = "Zom.Audio.Footstep"))
	FGameplayTag FootstepSoundTag;
};


/**
 * Maps every physical surface in the project to how much noise it makes underfoot and which footstep row to
 * play. Authored once as DA_SurfaceAudio and pointed at by UZomNoiseSettings; UZomSurfaceAudioSubsystem
 * reads it exactly once and flattens it into a fixed array, so nothing ever hashes this TMap at runtime.
 *
 * Adding a surface is a new SurfaceType in DefaultEngine.ini plus a row here - no code changes.
 */
UCLASS(BlueprintType)
class ZOM_API UZomSurfaceAudioSet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	// Per-surface rows. Surfaces with no row here use DefaultEntry.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom|Surface", meta = (DisplayName = "Surfaces", ForceInlineRow))
	TMap<TEnumAsByte<EPhysicalSurface>, FZomSurfaceNoiseEntry> Surfaces;

	// Used for SurfaceType_Default, for any surface with no row, and for a trace that hits geometry carrying
	// no physical material at all - which is most untextured blockout geometry, so this gets used a lot
	// early on. Keep its Loudness audible rather than zero, or the system will look broken on a grey-box level.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom|Surface", meta = (DisplayName = "Default Entry"))
	FZomSurfaceNoiseEntry DefaultEntry;
};
