// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "MotionCombatSystem/Structs/MCS_HitReaction.h"
#include "ZomCharacterAudioComponent.generated.h"


// Forward declarations

class UAudioComponent;
class UZomCharacterSoundSet;


/**
 * Plays a character's sounds by Zom.Audio.* gameplay tag from a UZomCharacterSoundSet. Shared by every
 * AZomCharacterBase (player, zombies, Boss). Owns no audio component itself - AZomCharacterBase hands it the
 * head-attached VoiceAudioComponent. Also runs the idle vocal scheduler (random-interval timer, no tick) so a
 * horde's growls stay staggered; horde-wide limits come from the sound assets' Sound Concurrency, not from here.
 */
UCLASS(ClassGroup = (Zom), meta = (BlueprintSpawnableComponent, DisplayName = "Zom Character Audio Component"))
class ZOM_API UZomCharacterAudioComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UZomCharacterAudioComponent();

	// -------------
	// Functions
	// -------------

	// Swaps the active sound set (e.g. a pooled zombie reactivated as a different type) and resets cooldowns.
	UFUNCTION(BlueprintCallable, Category = "Zom|Audio", meta = (DisplayName = "Set Sound Set"))
	void SetSoundSet(UZomCharacterSoundSet* InSoundSet);

	// Returns the active sound set.
	UFUNCTION(BlueprintPure, Category = "Zom|Audio", meta = (DisplayName = "Get Sound Set"))
	UZomCharacterSoundSet* GetSoundSet() const { return SoundSet; }

	// Plays SoundTag's entry. AtLocation entries play at the owner's location. Returns true if a sound started.
	UFUNCTION(BlueprintCallable, Category = "Zom|Audio", meta = (DisplayName = "Play Character Sound"))
	bool PlayCharacterSound(FGameplayTag SoundTag);

	// Plays SoundTag's entry, placing AtLocation entries at Location (e.g. a hit's impact point). Returns true if a sound started.
	UFUNCTION(BlueprintCallable, Category = "Zom|Audio", meta = (DisplayName = "Play Character Sound At Location"))
	bool PlayCharacterSoundAtLocation(FGameplayTag SoundTag, FVector Location);

	// Plays the vocal for a hit reaction of the given severity (sound set override first, then the default mapping).
	// A heavier severity with no row of its own falls back to Vocal.Hit.Light. Returns true if a sound started.
	UFUNCTION(BlueprintCallable, Category = "Zom|Audio", meta = (DisplayName = "Play Hit Reaction Sound"))
	bool PlayHitReactionSound(EPGAS_HitSeverity Severity);

	// (Re)starts the idle vocal scheduler. No-op if the sound set has no Zom.Audio.Vocal.Idle row.
	UFUNCTION(BlueprintCallable, Category = "Zom|Audio", meta = (DisplayName = "Start Idle Vocals"))
	void StartIdleVocals();

	// Stops the idle vocal scheduler and the voice. AtLocation one-shots already playing are left to finish.
	UFUNCTION(BlueprintCallable, Category = "Zom|Audio", meta = (DisplayName = "Stop Audio"))
	void StopAudio();

	// Sets the audio component Voice entries play on. Called by AZomCharacterBase::PostInitializeComponents.
	void SetVoiceAudioComponent(UAudioComponent* InVoiceAudioComponent);

	// -------------
	// Properties
	// -------------

	// Sound set applied at BeginPlay (player/Boss). Zombies override it from UZombieTypeData::SoundSet on every activation.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zom|Audio", meta = (DisplayName = "Default Sound Set"))
	TObjectPtr<UZomCharacterSoundSet> DefaultSoundSet;

protected:
	// Called when the component is removed from play; stops the idle scheduler and voice
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

	// -------------
	// Functions
	// -------------

	// Shared play path: looks up the entry, applies cooldown/chance, rolls volume/pitch, and plays it on the voice or at Location.
	bool TryPlaySound(const FGameplayTag& SoundTag, const FVector& Location);

	// Returns the sound tag for a hit reaction severity: the sound set's override if present, else the default mapping.
	FGameplayTag GetHitReactionSoundTag(EPGAS_HitSeverity Severity) const;

	// Arms the one-shot idle timer. Re-armed after every fire so each wait gets a fresh random length.
	void ScheduleNextIdleVocal(float Delay);

	// Idle timer callback: plays Vocal.Idle if CanPlayIdleVocal, then schedules the next attempt.
	void HandleIdleVocalTimer();

	// Whether the owner is idle enough to growl: active (not pooled), alive, slower than the idle threshold, not
	// hit-reacting, and not already vocalizing (so a growl never cuts off a hit scream).
	bool CanPlayIdleVocal() const;

	// -------------
	// Properties
	// -------------

	// Active sound set. Set from DefaultSoundSet or UZombieTypeData::SoundSet.
	UPROPERTY(Transient)
	TObjectPtr<UZomCharacterSoundSet> SoundSet;

	// The owner's head-attached voice, owned by AZomCharacterBase.
	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> VoiceAudioComponent;

	// Handle for the idle vocal timer.
	FTimerHandle IdleVocalTimerHandle;

	// World time each sound tag last played, for FZomSoundEntry::Cooldown.
	TMap<FGameplayTag, double> LastPlayTimes;
};
