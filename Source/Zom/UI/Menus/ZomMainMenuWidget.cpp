// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/UI/Menus/ZomMainMenuWidget.h"
#include "Zom/UI/Menus/ZomMenuButton.h"
#include "Zom/Game/ZomGameInstance.h"
#include "Zom/Game/ZomHUD.h"

#define LOCTEXT_NAMESPACE "ZomMainMenu"


UZomMainMenuWidget::UZomMainMenuWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Root of the front end - Back has nothing to return to.
	bIsBackHandler = false;
}

void UZomMainMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	NewGameButton->OnClicked().AddUObject(this, &UZomMainMenuWidget::HandleNewGameClicked);
	ContinueButton->OnClicked().AddUObject(this, &UZomMainMenuWidget::HandleContinueClicked);
	SettingsButton->OnClicked().AddUObject(this, &UZomMainMenuWidget::HandleSettingsClicked);
	QuitButton->OnClicked().AddUObject(this, &UZomMainMenuWidget::HandleQuitClicked);
}

void UZomMainMenuWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	// Re-checked on every activation (e.g. returning from Settings), not just construction.
	const UZomGameInstance* GameInstance = GetZomGameInstance();
	ContinueButton->SetIsEnabled(GameInstance && GameInstance->HasSave());
}

UWidget* UZomMainMenuWidget::NativeGetDesiredFocusTarget() const
{
	return ContinueButton->GetIsEnabled() ? ContinueButton.Get() : NewGameButton.Get();
}

void UZomMainMenuWidget::HandleNewGameClicked()
{
	UZomGameInstance* GameInstance = GetZomGameInstance();
	if (!GameInstance)
	{
		return;
	}

	if (!GameInstance->HasSave())
	{
		GameInstance->StartNewGame();
		return;
	}

	if (AZomHUD* HUD = GetZomHUD())
	{
		HUD->ShowConfirmDialog(
			LOCTEXT("NewGameTitle", "Start a New Game?"),
			LOCTEXT("NewGameBody", "Your saved progress will be overwritten."),
			[GameInstance]() { GameInstance->StartNewGame(); });
	}
}

void UZomMainMenuWidget::HandleContinueClicked()
{
	if (UZomGameInstance* GameInstance = GetZomGameInstance())
	{
		GameInstance->ContinueGame();
	}
}

void UZomMainMenuWidget::HandleSettingsClicked()
{
	if (AZomHUD* HUD = GetZomHUD())
	{
		HUD->ShowSettingsMenu();
	}
}

void UZomMainMenuWidget::HandleQuitClicked()
{
	UZomGameInstance* GameInstance = GetZomGameInstance();
	AZomHUD* HUD = GetZomHUD();
	if (!GameInstance || !HUD)
	{
		return;
	}

	HUD->ShowConfirmDialog(
		LOCTEXT("QuitTitle", "Quit Game?"),
		FText::GetEmpty(),
		[GameInstance]() { GameInstance->QuitGame(); });
}

#undef LOCTEXT_NAMESPACE
