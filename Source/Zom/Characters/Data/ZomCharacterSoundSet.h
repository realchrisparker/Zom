// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Math/Interval.h"
#include "MotionCombatSystem/Structs/MCS_HitReaction.h"
#include "ZomCharacterSoundSet.generated.h"


// Forward declaration

class USoundBase;


/**
 * How a sound set entry is played by UZomCharacterAudioComponent.
 */
UENUM(BlueprintType)
enum class EZomSoundPlayback : uint8
{
	// Plays on the owner's head-attached voice UAudioComponent, replacing whatever that voice was saying. Silent while
	// the owner is hidden (pooled), and stopped when a zombie returns to the pool.
	Voice		UMETA(DisplayName = "Voice"),

	// Fire-and-forget one-shot at a world location (UGameplayStatics::PlaySoundAtLocation, owned by the character for
	// concurrency). Keeps playing after the owner hides - use for death vocals and impacts.
	AtLocation	UMETA(DisplayName = "At Location")
};


/**
 * One playable sound event: the sound plus the per-play randomization and gating applied on top of it. Sound Class,
 * Attenuation and Concurrency live on the sound asset itself (MetaSound Source / Sound Cue), not here.
 */
USTRUCT(BlueprintType)
struct ZOM_API FZomSoundEntry
{
	GENERATED_BODY()

	// MetaSound Source or Sound Cue to play. An entry with no sound never plays.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom|Audio", meta = (DisplayName = "Sound"))
	TObjectPtr<USoundBase> Sound;

	// Whether this plays on the character's voice or as a one-shot at a location.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom|Audio", meta = (DisplayName = "Playback"))
	EZomSoundPlayback Playback = EZomSoundPlayback::Voice;

	// Random volume multiplier rolled for each play.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom|Audio", meta = (DisplayName = "Volume Range"))
	FFloatInterval VolumeRange = FFloatInterval(1.f, 1.f);

	// Random pitch multiplier rolled for each play.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom|Audio", meta = (DisplayName = "Pitch Range"))
	FFloatInterval PitchRange = FFloatInterval(1.f, 1.f);

	// Chance (0-1) that a play request actually plays.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom|Audio", meta = (DisplayName = "Play Chance", ClampMin = "0", ClampMax = "1"))
	float PlayChance = 1.f;

	// Minimum seconds between plays of this entry on the same character (e.g. stops fast combos spamming hit vocals).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom|Audio", meta = (DisplayName = "Cooldown", ClampMin = "0", Units = "s"))
	float Cooldown = 0.f;
};


/**
 * Every sound a character can make, keyed by Zom.Audio.* gameplay tag and played through UZomCharacterAudioComponent.
 * New sound events are a new native tag plus a new row here - no component changes. Zombies get theirs from
 * UZombieTypeData::SoundSet (types can share one asset); the player and Boss set UZomCharacterAudioComponent::DefaultSoundSet.
 */
UCLASS(BlueprintType)
class ZOM_API UZomCharacterSoundSet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	// -------------
	// Functions
	// -------------

	// Returns the entry for SoundTag, or null if this set has no row for it.
	const FZomSoundEntry* FindSound(const FGameplayTag& SoundTag) const;

	// -------------
	// Sounds
	// -------------

	// Sound events keyed by Zom.Audio.* tag (e.g. Zom.Audio.Vocal.Idle, Zom.Audio.Vocal.Hit.Light, Zom.Audio.Vocal.Death).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom|Audio", meta = (DisplayName = "Sounds", Categories = "Zom.Audio", ForceInlineRow))
	TMap<FGameplayTag, FZomSoundEntry> Sounds;

	// -------------
	// Idle
	// -------------

	// Random seconds between idle vocal attempts (Zom.Audio.Vocal.Idle). No Idle row = no idle vocals.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom|Audio|Idle", meta = (DisplayName = "Idle Vocal Interval"))
	FFloatInterval IdleVocalInterval = FFloatInterval(4.f, 10.f);

	// The character only counts as idle (and may growl) while its 2D speed is below this.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom|Audio|Idle", meta = (DisplayName = "Idle Speed Threshold", ClampMin = "0"))
	float IdleSpeedThreshold = 2.9f;

	// -------------
	// Hit Reactions
	// -------------

	// Optional per-severity sound tag overrides. Severities not listed use UZomCharacterAudioComponent's default
	// mapping: Death -> Vocal.Death, Light/Dazed -> Vocal.Hit.Light, everything else -> Vocal.Hit.Heavy.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zom|Audio|Hit Reactions", meta = (DisplayName = "Hit Severity Overrides", Categories = "Zom.Audio"))
	TMap<EPGAS_HitSeverity, FGameplayTag> HitSeverityOverrides;
};
