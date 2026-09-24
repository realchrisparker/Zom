// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/UI/Menus/ZomPauseMenuWidget.h"
#include "Zom/UI/Menus/ZomMenuButton.h"
#include "Zom/Game/ZomGameInstance.h"
#include "Zom/Game/ZomHUD.h"
#include "CommonTextBlock.h"

#define LOCTEXT_NAMESPACE "ZomPauseMenu"


void UZomPauseMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (ResumeButton)
	{
		ResumeButton->OnClicked().AddUObject(this, &UZomPauseMenuWidget::HandleResumeClicked);
	}
	SaveGameButton->OnClicked().AddUObject(this, &UZomPauseMenuWidget::HandleSaveGameClicked);
	SettingsButton->OnClicked().AddUObject(this, &UZomPauseMenuWidget::HandleSettingsClicked);
	MainMenuButton->OnClicked().AddUObject(this, &UZomPauseMenuWidget::HandleMainMenuClicked);
	QuitButton->OnClicked().AddUObject(this, &UZomPauseMenuWidget::HandleQuitClicked);
}

void UZomPauseMenuWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	if (SaveStatusText)
	{
		SaveStatusText->SetText(FText::GetEmpty());
	}
}

UWidget* UZomPauseMenuWidget::NativeGetDesiredFocusTarget() const
{
	return ResumeButton ? ResumeButton.Get() : SaveGameButton.Get();
}

void UZomPauseMenuWidget::HandleResumeClicked()
{
	DeactivateWidget();
}

void UZomPauseMenuWidget::HandleSaveGameClicked()
{
	UZomGameInstance* GameInstance = GetZomGameInstance();
	const bool bSaved = GameInstance && GameInstance->SaveCurrentState();

	if (SaveStatusText)
	{
		SaveStatusText->SetText(bSaved
			? LOCTEXT("Saved", "Game saved. You'll resume from your last checkpoint.")
			: LOCTEXT("SaveFailed", "Save failed."));
	}
}

void UZomPauseMenuWidget::HandleSettingsClicked()
{
	if (AZomHUD* HUD = GetZomHUD())
	{
		HUD->ShowSettingsMenu();
	}
}

void UZomPauseMenuWidget::HandleMainMenuClicked()
{
	UZomGameInstance* GameInstance = GetZomGameInstance();
	AZomHUD* HUD = GetZomHUD();
	if (!GameInstance || !HUD)
	{
		return;
	}

	HUD->ShowConfirmDialog(
		LOCTEXT("MainMenuTitle", "Return to Main Menu?"),
		LOCTEXT("MainMenuBody", "Any progress since your last save will be lost."),
		[GameInstance]() { GameInstance->ReturnToMainMenu(); });
}

void UZomPauseMenuWidget::HandleQuitClicked()
{
	UZomGameInstance* GameInstance = GetZomGameInstance();
	AZomHUD* HUD = GetZomHUD();
	if (!GameInstance || !HUD)
	{
		return;
	}

	HUD->ShowConfirmDialog(
		LOCTEXT("QuitTitle", "Quit Game?"),
		LOCTEXT("QuitBody", "Any progress since your last save will be lost."),
		[GameInstance]() { GameInstance->QuitGame(); });
}

#undef LOCTEXT_NAMESPACE
