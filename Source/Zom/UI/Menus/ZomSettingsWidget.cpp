// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/UI/Menus/ZomSettingsWidget.h"
#include "Zom/UI/Menus/ZomMenuButton.h"
#include "Zom/Game/ZomGameInstance.h"
#include "Zom/Settings/ZomGameUserSettings.h"
#include "Zom/UI/Menus/ZomSettingsRotator.h"
#include "Components/CheckBox.h"
#include "Components/Slider.h"
#include "Kismet/KismetSystemLibrary.h"

#define LOCTEXT_NAMESPACE "ZomSettings"


// 0 = uncapped, matching UGameUserSettings::SetFrameRateLimit.
const TArray<int32> UZomSettingsWidget::FrameRateOptions = { 30, 60, 120, 144, 0 };

namespace ZomSettings
{
	// Index-aligned with EWindowMode::Type (Fullscreen, WindowedFullscreen, Windowed).
	static TArray<FText> GetWindowModeLabels()
	{
		return {
			LOCTEXT("Fullscreen", "Fullscreen"),
			LOCTEXT("WindowedFullscreen", "Borderless"),
			LOCTEXT("Windowed", "Windowed")
		};
	}

	// Index-aligned with scalability levels 0-4.
	static TArray<FText> GetQualityLabels()
	{
		return {
			LOCTEXT("Low", "Low"),
			LOCTEXT("Medium", "Medium"),
			LOCTEXT("High", "High"),
			LOCTEXT("Epic", "Epic"),
			LOCTEXT("Cinematic", "Cinematic")
		};
	}
}

void UZomSettingsWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	QualityRotator->PopulateTextLabels(ZomSettings::GetQualityLabels());
	WindowModeRotator->PopulateTextLabels(ZomSettings::GetWindowModeLabels());

	TArray<FText> FrameRateLabels;
	for (const int32 FrameRate : FrameRateOptions)
	{
		FrameRateLabels.Add(FrameRate > 0 ? FText::AsNumber(FrameRate) : LOCTEXT("Unlimited", "Unlimited"));
	}
	FrameRateRotator->PopulateTextLabels(FrameRateLabels);

	// Match UZomGameUserSettings::LookSensitivity's clamp; the volume sliders keep the default 0-1 range.
	LookSensitivitySlider->SetMinValue(0.1f);
	LookSensitivitySlider->SetMaxValue(5.f);

	ApplyButton->OnClicked().AddUObject(this, &UZomSettingsWidget::ApplyToSettings);
	RevertButton->OnClicked().AddUObject(this, &UZomSettingsWidget::ReadFromSettings);
	BackButton->OnClicked().AddUObject(this, &UZomSettingsWidget::HandleBackClicked);
}

void UZomSettingsWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	ReadFromSettings();
}

UWidget* UZomSettingsWidget::NativeGetDesiredFocusTarget() const
{
	return QualityRotator;
}

void UZomSettingsWidget::ReadFromSettings()
{
	const UZomGameUserSettings* Settings = UZomGameUserSettings::Get();
	if (!Settings)
	{
		return;
	}

	// Graphics
	// GetOverallScalabilityLevel returns -1 for a custom mix of per-group levels; show High rather than nothing.
	const int32 QualityLevel = Settings->GetOverallScalabilityLevel();
	QualityRotator->SetSelectedItem(QualityLevel >= 0 ? QualityLevel : 2);

	WindowModeRotator->SetSelectedItem(static_cast<int32>(Settings->GetFullscreenMode()));

	ResolutionOptions.Reset();
	UKismetSystemLibrary::GetSupportedFullscreenResolutions(ResolutionOptions);
	const FIntPoint CurrentResolution = Settings->GetScreenResolution();
	ResolutionOptions.AddUnique(CurrentResolution);

	TArray<FText> ResolutionLabels;
	for (const FIntPoint& Resolution : ResolutionOptions)
	{
		// Printf rather than FText::Format, which would group digits ("1,920 x 1,080").
		ResolutionLabels.Add(FText::FromString(FString::Printf(TEXT("%d x %d"), Resolution.X, Resolution.Y)));
	}
	ResolutionRotator->PopulateTextLabels(ResolutionLabels);
	ResolutionRotator->SetSelectedItem(ResolutionOptions.IndexOfByKey(CurrentResolution));

	const int32 FrameRateIndex = FrameRateOptions.IndexOfByKey(FMath::RoundToInt(Settings->GetFrameRateLimit()));
	FrameRateRotator->SetSelectedItem(FrameRateIndex != INDEX_NONE ? FrameRateIndex : FrameRateOptions.Num() - 1);

	VSyncCheckBox->SetIsChecked(Settings->IsVSyncEnabled());

	// Audio
	MasterVolumeSlider->SetValue(Settings->MasterVolume);
	MusicVolumeSlider->SetValue(Settings->MusicVolume);
	SFXVolumeSlider->SetValue(Settings->SFXVolume);
	DialogueVolumeSlider->SetValue(Settings->DialogueVolume);

	// Controls
	LookSensitivitySlider->SetValue(Settings->LookSensitivity);
	InvertLookYCheckBox->SetIsChecked(Settings->bInvertLookY);

	// Accessibility
	CaptionsCheckBox->SetIsChecked(Settings->bAudioCueCaptionsEnabled);
	ColorblindHUDCheckBox->SetIsChecked(Settings->bColorblindSafeHUD);
	ReducedScreenEffectsCheckBox->SetIsChecked(Settings->bReducedScreenEffects);
}

void UZomSettingsWidget::ApplyToSettings()
{
	UZomGameUserSettings* Settings = UZomGameUserSettings::Get();
	if (!Settings)
	{
		return;
	}

	// Graphics
	Settings->SetOverallScalabilityLevel(QualityRotator->GetSelectedIndex());
	Settings->SetFullscreenMode(static_cast<EWindowMode::Type>(WindowModeRotator->GetSelectedIndex()));
	if (ResolutionOptions.IsValidIndex(ResolutionRotator->GetSelectedIndex()))
	{
		Settings->SetScreenResolution(ResolutionOptions[ResolutionRotator->GetSelectedIndex()]);
	}
	Settings->SetFrameRateLimit(FrameRateOptions[FrameRateRotator->GetSelectedIndex()]);
	Settings->SetVSyncEnabled(VSyncCheckBox->IsChecked());

	// Audio
	Settings->MasterVolume = MasterVolumeSlider->GetValue();
	Settings->MusicVolume = MusicVolumeSlider->GetValue();
	Settings->SFXVolume = SFXVolumeSlider->GetValue();
	Settings->DialogueVolume = DialogueVolumeSlider->GetValue();

	// Controls
	Settings->LookSensitivity = LookSensitivitySlider->GetValue();
	Settings->bInvertLookY = InvertLookYCheckBox->IsChecked();

	// Accessibility
	Settings->bAudioCueCaptionsEnabled = CaptionsCheckBox->IsChecked();
	Settings->bColorblindSafeHUD = ColorblindHUDCheckBox->IsChecked();
	Settings->bReducedScreenEffects = ReducedScreenEffectsCheckBox->IsChecked();

	// Applies resolution/window mode/scalability and saves everything (incl. our Config fields) to disk.
	Settings->ApplySettings(false);

	if (UZomGameInstance* GameInstance = GetZomGameInstance())
	{
		GameInstance->ApplyAudioSettings();
	}
}

void UZomSettingsWidget::HandleBackClicked()
{
	DeactivateWidget();
}

#undef LOCTEXT_NAMESPACE
