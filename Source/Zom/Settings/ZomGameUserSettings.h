// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "ZomGameUserSettings.generated.h"


/**
 * Player preferences - graphics (inherited from UGameUserSettings), audio volumes, look controls and the
 * Section 13 accessibility toggles. Registered as GameUserSettingsClassName in DefaultEngine.ini, so it persists
 * through the standard engine settings save path (GameUserSettings.ini), a different lifecycle than
 * UZomSaveGame's checkpoint/run state. Formerly UZomAccessibilitySettings (ClassRedirect in DefaultEngine.ini).
 */
UCLASS()
class ZOM_API UZomGameUserSettings : public UGameUserSettings
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Zom|Settings", meta = (DisplayName = "Get Zom Game User Settings"))
	static UZomGameUserSettings* Get();

	virtual void SetToDefaults() override;

	// -------------
	// Audio
	// -------------
	// 0-1 linear values pushed onto the AudioModulation control buses by UZomGameInstance::ApplyAudioSettings.

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Zom|Settings|Audio", meta = (ClampMin = "0", ClampMax = "1"))
	float MasterVolume = 1.f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Zom|Settings|Audio", meta = (ClampMin = "0", ClampMax = "1"))
	float MusicVolume = 1.f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Zom|Settings|Audio", meta = (ClampMin = "0", ClampMax = "1"))
	float SFXVolume = 1.f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Zom|Settings|Audio", meta = (ClampMin = "0", ClampMax = "1"))
	float DialogueVolume = 1.f;

	// -------------
	// Controls
	// -------------

	// Multiplier on IA_Look input, applied in AZomPlayerController::Input_Look.
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Zom|Settings|Controls", meta = (ClampMin = "0.1", ClampMax = "5"))
	float LookSensitivity = 1.f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Zom|Settings|Controls")
	bool bInvertLookY = false;

	// -------------
	// Accessibility (Section 13)
	// -------------

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Zom|Settings|Accessibility")
	bool bAudioCueCaptionsEnabled = false;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Zom|Settings|Accessibility")
	bool bColorblindSafeHUD = false;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Zom|Settings|Accessibility")
	bool bReducedScreenEffects = false;
};
