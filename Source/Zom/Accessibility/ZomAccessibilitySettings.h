// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "ZomAccessibilitySettings.generated.h"


/**
 * [Design, detail Open - the dev doc names the feature set but not the base class; UGameUserSettings is the
 * recommendation, not a confirmed decision] Audio-cue captions, colorblind-safe HUD, screen-effects toggle
 * (Section 13). Persists through the standard engine settings save path, a different lifecycle than
 * UZomSaveGame's checkpoint/run state.
 */
UCLASS()
class ZOM_API UZomAccessibilitySettings : public UGameUserSettings
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Zom|Accessibility", meta = (DisplayName = "Get Zom Accessibility Settings"))
	static UZomAccessibilitySettings* Get();

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Zom|Accessibility")
	bool bAudioCueCaptionsEnabled = false;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Zom|Accessibility")
	bool bColorblindSafeHUD = false;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Zom|Accessibility")
	bool bReducedScreenEffects = false;
};
