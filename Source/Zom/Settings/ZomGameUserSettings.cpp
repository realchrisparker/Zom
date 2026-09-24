// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Settings/ZomGameUserSettings.h"
#include "Engine/Engine.h"


UZomGameUserSettings* UZomGameUserSettings::Get()
{
	return GEngine ? Cast<UZomGameUserSettings>(GEngine->GetGameUserSettings()) : nullptr;
}

void UZomGameUserSettings::SetToDefaults()
{
	Super::SetToDefaults();

	const UZomGameUserSettings* Defaults = GetDefault<UZomGameUserSettings>();

	MasterVolume = Defaults->MasterVolume;
	MusicVolume = Defaults->MusicVolume;
	SFXVolume = Defaults->SFXVolume;
	DialogueVolume = Defaults->DialogueVolume;

	LookSensitivity = Defaults->LookSensitivity;
	bInvertLookY = Defaults->bInvertLookY;

	bAudioCueCaptionsEnabled = Defaults->bAudioCueCaptionsEnabled;
	bColorblindSafeHUD = Defaults->bColorblindSafeHUD;
	bReducedScreenEffects = Defaults->bReducedScreenEffects;
}
