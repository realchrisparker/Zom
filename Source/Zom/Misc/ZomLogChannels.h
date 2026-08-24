// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Logging/LogMacros.h"

// General purpose channel for anything without a more specific home.
ZOM_API DECLARE_LOG_CATEGORY_EXTERN(LogZom, Log, All);

// Characters, pawns and their components.
ZOM_API DECLARE_LOG_CATEGORY_EXTERN(LogZomCharacter, Log, All);

// Game framework: game mode, game state, player state, session.
ZOM_API DECLARE_LOG_CATEGORY_EXTERN(LogZomGame, Log, All);

// Player input and controller handling.
ZOM_API DECLARE_LOG_CATEGORY_EXTERN(LogZomInput, Log, All);

// HUD, widgets and anything user-facing.
ZOM_API DECLARE_LOG_CATEGORY_EXTERN(LogZomUI, Log, All);
