// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Game/ZomCheatManager.h"
#include "HAL/IConsoleManager.h"

UZomCheatManager::UZomCheatManager()
{

}

// Console-friendly wrapper around the Zom.Debug.Noise console variable.
void UZomCheatManager::ZomDebugNoise(int32 Enable)
{
	// The cvar is the real switch (UZomCharacterNoiseComponent reads it directly); this exec exists so the
	// toggle is discoverable by typing "Zom" in the console alongside the other cheats.
	if (IConsoleVariable* Variable = IConsoleManager::Get().FindConsoleVariable(TEXT("Zom.Debug.Noise")))
	{
		Variable->Set(Enable, ECVF_SetByConsole);
	}
}
