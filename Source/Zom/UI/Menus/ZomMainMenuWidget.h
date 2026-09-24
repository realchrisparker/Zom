// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Zom/UI/Menus/ZomMenuWidget.h"
#include "ZomMainMenuWidget.generated.h"


class UZomMenuButton;


/** Front-end menu (StartupMenuClass on BP_FrontEndHUD): New Game, Continue, Settings, Quit. */
UCLASS(Abstract)
class ZOM_API UZomMainMenuWidget : public UZomMenuWidget
{
	GENERATED_BODY()

public:
	UZomMainMenuWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnActivated() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UZomMenuButton> NewGameButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UZomMenuButton> ContinueButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UZomMenuButton> SettingsButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UZomMenuButton> QuitButton;

private:
	void HandleNewGameClicked();
	void HandleContinueClicked();
	void HandleSettingsClicked();
	void HandleQuitClicked();
};
