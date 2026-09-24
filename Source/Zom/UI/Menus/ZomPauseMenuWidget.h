// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Zom/UI/Menus/ZomMenuWidget.h"
#include "ZomPauseMenuWidget.generated.h"


class UCommonTextBlock;
class UZomMenuButton;


/**
 * In-game pause menu (AZomHUD::PauseMenuClass, opened by IA_PauseMenu): Save Game, Settings, Main Menu, Quit.
 * Back resumes. Pausing itself is AZomHUD's job, keyed off the menu stack, so it holds across Settings/dialogs.
 */
UCLASS(Abstract)
class ZOM_API UZomPauseMenuWidget : public UZomMenuWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnActivated() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	// Optional - Back already resumes, this is for mouse users.
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UZomMenuButton> ResumeButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UZomMenuButton> SaveGameButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UZomMenuButton> SettingsButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UZomMenuButton> MainMenuButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UZomMenuButton> QuitButton;

	// Shows "Game Saved" / failure feedback after Save Game.
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> SaveStatusText;

private:
	void HandleResumeClicked();
	void HandleSaveGameClicked();
	void HandleSettingsClicked();
	void HandleMainMenuClicked();
	void HandleQuitClicked();
};
