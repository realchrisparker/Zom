// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/SubSystems/SurfaceAudio/ZomSurfaceAudioSubsystem.h"
#include "Zom/Misc/ZomGameplayTags.h"
#include "Zom/Misc/ZomLogChannels.h"
#include "Zom/SubSystems/SurfaceAudio/Settings/ZomNoiseSettings.h"


// Loads the configured surface set and builds the flattened table.
void UZomSurfaceAudioSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Audible rather than silent, so a project with no surface set authored yet still emits noise the
	// debug rings can show, instead of looking like the system never ran.
	FallbackEntry.Loudness = 0.8f;
	FallbackEntry.FootstepSoundTag = TAG_Zom_Audio_Footstep_Default.GetTag();

	RebuildSurfaceTable();
}

// Re-reads the data asset and rebuilds the table.
void UZomSurfaceAudioSubsystem::RebuildSurfaceTable()
{
	const UZomNoiseSettings* Settings = GetDefault<UZomNoiseSettings>();

	// LoadSynchronous is acceptable here and nowhere else in this class: it runs once at game instance
	// init, off the hot path, and every later query reads the flattened array instead.
	SurfaceAudioSet = Settings ? Settings->SurfaceAudioSet.LoadSynchronous() : nullptr;

	const FZomSurfaceNoiseEntry DefaultEntry = SurfaceAudioSet ? SurfaceAudioSet->DefaultEntry : FallbackEntry;

	// Fill every slot with the default first, so a surface with no authored row needs no branch at lookup
	// time - the fallback is already sitting in its index.
	SurfaceTable.Reset();
	SurfaceTable.Init(DefaultEntry, static_cast<int32>(SurfaceType_Max));

	if (!SurfaceAudioSet)
	{
		UE_LOG(LogZom, Warning, TEXT("UZomSurfaceAudioSubsystem: no Surface Audio Set configured (Project Settings > Game > Zom Stealth). Every surface will use the built-in default."));
		return;
	}

	for (const TPair<TEnumAsByte<EPhysicalSurface>, FZomSurfaceNoiseEntry>& Pair : SurfaceAudioSet->Surfaces)
	{
		const int32 Index = static_cast<int32>(Pair.Key.GetValue());
		if (SurfaceTable.IsValidIndex(Index))
		{
			SurfaceTable[Index] = Pair.Value;
		}
	}

	UE_LOG(LogZom, Log, TEXT("UZomSurfaceAudioSubsystem: flattened %d authored surface rows from %s."),
		SurfaceAudioSet->Surfaces.Num(), *GetNameSafe(SurfaceAudioSet));
}

// Direct index into the flattened table.
const FZomSurfaceNoiseEntry& UZomSurfaceAudioSubsystem::GetSurfaceEntry(EPhysicalSurface Surface) const
{
	const int32 Index = static_cast<int32>(Surface);
	return SurfaceTable.IsValidIndex(Index) ? SurfaceTable[Index] : FallbackEntry;
}

// How loud this surface is underfoot.
float UZomSurfaceAudioSubsystem::GetSurfaceLoudness(EPhysicalSurface Surface) const
{
	return GetSurfaceEntry(Surface).Loudness;
}

// Which footstep row to play on this surface.
FGameplayTag UZomSurfaceAudioSubsystem::GetSurfaceFootstepSoundTag(EPhysicalSurface Surface) const
{
	return GetSurfaceEntry(Surface).FootstepSoundTag;
}
