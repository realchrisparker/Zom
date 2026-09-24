// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ZomHUD.generated.h"


class UCommonActivatableWidget;
class UZomConfirmDialogWidget;
class UZomHUDWidget;
class UZomMenuWidget;
class UZomUIRootWidget;


/**
 * Owns all of the local player's UI: the gameplay HUD widget and the UZomUIRootWidget whose MenuStack every
 * menu is pushed onto. Any menu showing pauses the game (if bPauseWhileMenuOpen) and hides the HUD widget - the
 * menus themselves don't pause, since the stack deactivates the menu underneath whenever another is pushed.
 *
 * BP_HUD configures the menu classes; BP_FrontEndHUD is a child of it with no HUDWidgetClass,
 * StartupMenuClass = WBP_MainMenu and bPauseWhileMenuOpen off.
 */
UCLASS(Blueprintable, meta=(DisplayName="Zom HUD"))
class ZOM_API AZomHUD : public AHUD
{
	GENERATED_BODY()

public:
	AZomHUD();

	virtual void DrawHUD() override;

	UFUNCTION(BlueprintCallable, Category = "Zom|UI")
	UZomMenuWidget* PushMenu(TSubclassOf<UZomMenuWidget> MenuClass);

	UFUNCTION(BlueprintCallable, Category = "Zom|UI")
	void ShowPauseMenu();

	UFUNCTION(BlueprintCallable, Category = "Zom|UI")
	void ShowSettingsMenu();

	// Pushes ConfirmDialogClass; OnConfirmed runs only if the player picks Confirm.
	void ShowConfirmDialog(const FText& Title, const FText& Body, TFunction<void()> OnConfirmed);

	UFUNCTION(BlueprintPure, Category = "Zom|UI")
	bool IsMenuOpen() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Class to instantiate and add to viewport in BeginPlay. Requires a WBP_* child assigned in the editor
	// (Section 12) - stays null-safe if none is set.
	UPROPERTY(EditDefaultsOnly, Category = "Zom|UI")
	TSubclassOf<UZomHUDWidget> HUDWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "Zom|UI")
	TObjectPtr<UZomHUDWidget> HUDWidgetInstance;

	UPROPERTY(EditDefaultsOnly, Category = "Zom|UI")
	TSubclassOf<UZomUIRootWidget> UIRootClass;

	UPROPERTY(BlueprintReadOnly, Category = "Zom|UI")
	TObjectPtr<UZomUIRootWidget> UIRootInstance;

	// Pushed on BeginPlay if set - the front-end map uses this for WBP_MainMenu.
	UPROPERTY(EditDefaultsOnly, Category = "Zom|UI")
	TSubclassOf<UZomMenuWidget> StartupMenuClass;

	UPROPERTY(EditDefaultsOnly, Category = "Zom|UI")
	TSubclassOf<UZomMenuWidget> PauseMenuClass;

	UPROPERTY(EditDefaultsOnly, Category = "Zom|UI")
	TSubclassOf<UZomMenuWidget> SettingsMenuClass;

	UPROPERTY(EditDefaultsOnly, Category = "Zom|UI")
	TSubclassOf<UZomConfirmDialogWidget> ConfirmDialogClass;

	// Gameplay maps pause the world while any menu is up; the front end leaves its backdrop running.
	UPROPERTY(EditDefaultsOnly, Category = "Zom|UI")
	bool bPauseWhileMenuOpen = true;

private:
	void HandleDisplayedMenuChanged(UCommonActivatableWidget* DisplayedWidget);
};
