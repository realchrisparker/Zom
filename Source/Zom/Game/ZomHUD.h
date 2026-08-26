// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ZomHUD.generated.h"


class UZomHUDWidget;


UCLASS(Blueprintable, meta=(DisplayName="Zom HUD"))
class ZOM_API AZomHUD : public AHUD
{
	GENERATED_BODY()

public:
	AZomHUD();

	virtual void DrawHUD() override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Class to instantiate and add to viewport in BeginPlay. Requires a WBP_* child assigned in the editor
	// (Section 12) - stays null-safe if none is set.
	UPROPERTY(EditDefaultsOnly, Category = "Zom|UI")
	TSubclassOf<UZomHUDWidget> HUDWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "Zom|UI")
	TObjectPtr<UZomHUDWidget> HUDWidgetInstance;
};
