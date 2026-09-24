// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Zom/UI/Menus/ZomMenuWidget.h"
#include "ZomSettingsWidget.generated.h"


class UCheckBox;
class UZomSettingsRotator;
class USlider;
class UZomMenuButton;


/**
 * Settings screen shared by the main and pause menus - one scrolling list with Graphics / Audio / Controls /
 * Accessibility sections. The controls are only a staging copy: nothing reaches UZomGameUserSettings until Apply,
 * so Back (or Revert) simply discards edits. Controls are re-read from the settings object on every activation.
 */
UCLASS(Abstract)
class ZOM_API UZomSettingsWidget : public UZomMenuWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnActivated() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	// -------------
	// Graphics
	// -------------

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UZomSettingsRotator> QualityRotator;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UZomSettingsRotator> WindowModeRotator;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UZomSettingsRotator> ResolutionRotator;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UZomSettingsRotator> FrameRateRotator;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCheckBox> VSyncCheckBox;

	// -------------
	// Audio
	// -------------

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> MasterVolumeSlider;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> MusicVolumeSlider;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> SFXVolumeSlider;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> DialogueVolumeSlider;

	// -------------
	// Controls
	// -------------

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> LookSensitivitySlider;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCheckBox> InvertLookYCheckBox;

	// -------------
	// Accessibility
	// -------------

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCheckBox> CaptionsCheckBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCheckBox> ColorblindHUDCheckBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCheckBox> ReducedScreenEffectsCheckBox;

	// -------------
	// Actions
	// -------------

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UZomMenuButton> ApplyButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UZomMenuButton> RevertButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UZomMenuButton> BackButton;

private:
	// Settings object -> controls.
	void ReadFromSettings();

	// Controls -> settings object, then apply and save to GameUserSettings.ini.
	void ApplyToSettings();

	void HandleBackClicked();

	// Option lists behind the rotators, index-aligned with their labels. Resolutions are rebuilt on each read.
	TArray<FIntPoint> ResolutionOptions;
	static const TArray<int32> FrameRateOptions;
};
