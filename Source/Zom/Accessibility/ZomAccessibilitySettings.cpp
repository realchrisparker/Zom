// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Accessibility/ZomAccessibilitySettings.h"
#include "Engine/Engine.h"


UZomAccessibilitySettings* UZomAccessibilitySettings::Get()
{
	return Cast<UZomAccessibilitySettings>(GEngine->GetGameUserSettings());
}
