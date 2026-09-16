// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Characters/Components/ZomCharacterAudioComponent.h"
#include "AbilitySystemComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Zom/Characters/Base/ZomCharacterBase.h"
#include "Zom/Characters/Data/ZomCharacterSoundSet.h"
#include "Zom/Misc/ZomGameplayTags.h"


namespace
{
	// Returns a random value inside Interval, tolerating a Min/Max authored the wrong way round.
	float RandomInInterval(const FFloatInterval& Interval)
	{
		return FMath::FRandRange(FMath::Min(Interval.Min, Interval.Max), FMath::Max(Interval.Min, Interval.Max));
	}
}


// Sets default values for this component's properties
UZomCharacterAudioComponent::UZomCharacterAudioComponent()
{
	// Idle vocals run on a timer and everything else is event-driven - no per-frame work.
	PrimaryComponentTick.bCanEverTick = false;
}

// Called when the component is removed from play; stops the idle scheduler and voice
void UZomCharacterAudioComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopAudio();

	Super::EndPlay(EndPlayReason);
}

// Sets the audio component Voice entries play on.
void UZomCharacterAudioComponent::SetVoiceAudioComponent(UAudioComponent* InVoiceAudioComponent)
{
	VoiceAudioComponent = InVoiceAudioComponent;
}

// Swaps the active sound set and resets cooldowns.
void UZomCharacterAudioComponent::SetSoundSet(UZomCharacterSoundSet* InSoundSet)
{
	SoundSet = InSoundSet;

	// Cooldowns belong to the previous activation/type - a reactivated pooled zombie starts fresh.
	LastPlayTimes.Reset();
}

// Plays SoundTag's entry at the owner's location.
bool UZomCharacterAudioComponent::PlayCharacterSound(FGameplayTag SoundTag)
{
	const AActor* Owner = GetOwner();
	return Owner && TryPlaySound(SoundTag, Owner->GetActorLocation());
}

// Plays SoundTag's entry at Location.
bool UZomCharacterAudioComponent::PlayCharacterSoundAtLocation(FGameplayTag SoundTag, FVector Location)
{
	return TryPlaySound(SoundTag, Location);
}

// Plays the vocal for a hit reaction of the given severity.
bool UZomCharacterAudioComponent::PlayHitReactionSound(EPGAS_HitSeverity Severity)
{
	if (!SoundSet)
	{
		return false;
	}

	FGameplayTag SoundTag = GetHitReactionSoundTag(Severity);

	// A set only needs one hit vocal to cover every non-lethal severity. Only a *missing* row falls back - a row that
	// exists but is on cooldown or failed its chance roll stays silent rather than playing the lighter vocal instead.
	if (Severity != EPGAS_HitSeverity::Death && !SoundSet->FindSound(SoundTag))
	{
		SoundTag = TAG_Zom_Audio_Vocal_Hit_Light.GetTag();
	}

	return PlayCharacterSound(SoundTag);
}

// Returns the sound tag for a hit reaction severity.
FGameplayTag UZomCharacterAudioComponent::GetHitReactionSoundTag(EPGAS_HitSeverity Severity) const
{
	if (SoundSet)
	{
		if (const FGameplayTag* OverrideTag = SoundSet->HitSeverityOverrides.Find(Severity); OverrideTag && OverrideTag->IsValid())
		{
			return *OverrideTag;
		}
	}

	switch (Severity)
	{
	case EPGAS_HitSeverity::Death:
		return TAG_Zom_Audio_Vocal_Death.GetTag();

	case EPGAS_HitSeverity::Light:
	case EPGAS_HitSeverity::Dazed:
		return TAG_Zom_Audio_Vocal_Hit_Light.GetTag();

	default:
		return TAG_Zom_Audio_Vocal_Hit_Heavy.GetTag();
	}
}

// (Re)starts the idle vocal scheduler.
void UZomCharacterAudioComponent::StartIdleVocals()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(IdleVocalTimerHandle);
	}

	if (!SoundSet || !SoundSet->FindSound(TAG_Zom_Audio_Vocal_Idle.GetTag()))
	{
		return;
	}

	// First wait is anywhere in [0, Max] rather than the normal interval, so zombies activated in the same frame
	// (a spawn wave) don't all growl on the same beat.
	const float MaxInterval = FMath::Max(SoundSet->IdleVocalInterval.Min, SoundSet->IdleVocalInterval.Max);
	ScheduleNextIdleVocal(FMath::FRandRange(0.f, MaxInterval));
}

// Stops the idle vocal scheduler and the voice.
void UZomCharacterAudioComponent::StopAudio()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(IdleVocalTimerHandle);
	}

	if (IsValid(VoiceAudioComponent))
	{
		VoiceAudioComponent->Stop();
	}
}

// Arms the one-shot idle timer.
void UZomCharacterAudioComponent::ScheduleNextIdleVocal(float Delay)
{
	if (UWorld* World = GetWorld())
	{
		// SetTimer treats a rate <= 0 as "clear", which would silently end the scheduler - keep it positive.
		World->GetTimerManager().SetTimer(IdleVocalTimerHandle, this, &UZomCharacterAudioComponent::HandleIdleVocalTimer, FMath::Max(Delay, 0.1f), false);
	}
}

// Idle timer callback.
void UZomCharacterAudioComponent::HandleIdleVocalTimer()
{
	if (!SoundSet)
	{
		return;
	}

	if (CanPlayIdleVocal())
	{
		PlayCharacterSound(TAG_Zom_Audio_Vocal_Idle.GetTag());
	}

	// Always re-arm, even when this attempt was skipped (moving, hit-reacting), so the zombie growls again once idle.
	ScheduleNextIdleVocal(RandomInInterval(SoundSet->IdleVocalInterval));
}

// Whether the owner is idle enough to growl.
bool UZomCharacterAudioComponent::CanPlayIdleVocal() const
{
	const AZomCharacterBase* Character = Cast<AZomCharacterBase>(GetOwner());
	if (!Character || Character->IsHidden() || Character->GetHealth() <= 0.f)
	{
		return false;
	}

	// Read from the actor, not UZomZombieAnimInstanceBase::Speed2D - that's written on a worker thread and may be
	// throttled for off-screen zombies.
	if (Character->GetVelocity().Size2D() >= SoundSet->IdleSpeedThreshold)
	{
		return false;
	}

	if (const UAbilitySystemComponent* AbilitySystem = Character->GetAbilitySystemComponent(); AbilitySystem && AbilitySystem->HasMatchingGameplayTag(TAG_Zom_Status_HitReacting))
	{
		return false;
	}

	return !VoiceAudioComponent || !VoiceAudioComponent->IsPlaying();
}

// Shared play path.
bool UZomCharacterAudioComponent::TryPlaySound(const FGameplayTag& SoundTag, const FVector& Location)
{
	const FZomSoundEntry* Entry = SoundSet ? SoundSet->FindSound(SoundTag) : nullptr;
	const UWorld* World = GetWorld();
	const AActor* Owner = GetOwner();

	if (!Entry || !Entry->Sound || !World || !Owner)
	{
		return false;
	}

	const double Now = World->GetTimeSeconds();
	if (const double* LastPlayTime = LastPlayTimes.Find(SoundTag); LastPlayTime && Now - *LastPlayTime < Entry->Cooldown)
	{
		return false;
	}

	if (Entry->PlayChance < 1.f && FMath::FRand() >= Entry->PlayChance)
	{
		return false;
	}

	const float Volume = RandomInInterval(Entry->VolumeRange);
	const float Pitch = RandomInInterval(Entry->PitchRange);

	switch (Entry->Playback)
	{
	case EZomSoundPlayback::Voice:
		// A pooled (hidden) character stays silent - its voice would otherwise play from wherever it's parked.
		if (!VoiceAudioComponent || Owner->IsHidden())
		{
			return false;
		}

		VoiceAudioComponent->SetSound(Entry->Sound);
		VoiceAudioComponent->SetVolumeMultiplier(Volume);
		VoiceAudioComponent->SetPitchMultiplier(Pitch);
		VoiceAudioComponent->Play();
		break;

	case EZomSoundPlayback::AtLocation:
		// Owner is passed so "Limit To Owner" concurrency (one vocal per character) still applies to one-shots.
		UGameplayStatics::PlaySoundAtLocation(this, Entry->Sound, Location, FRotator::ZeroRotator, Volume, Pitch, 0.f, nullptr, nullptr, Owner);
		break;
	}

	LastPlayTimes.Add(SoundTag, Now);
	return true;
}
