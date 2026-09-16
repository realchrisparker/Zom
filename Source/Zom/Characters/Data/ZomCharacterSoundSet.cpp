// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Characters/Data/ZomCharacterSoundSet.h"


// Returns the entry for SoundTag, or null if this set has no row for it.
const FZomSoundEntry* UZomCharacterSoundSet::FindSound(const FGameplayTag& SoundTag) const
{
	return SoundTag.IsValid() ? Sounds.Find(SoundTag) : nullptr;
}
