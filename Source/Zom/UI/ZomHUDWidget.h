// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ZomHUDWidget.generated.h"


class UZomObjectiveTrackerWidget;
class UZomAbilityBarWidget;
class UZomInventoryWidget;
class UZomDamageIndicatorWidget;


/**
 * [Proposed] Root widget, composes the rest (Section 12 of the dev doc). C++ base only - the WBP_* visual
 * layout binding these children is editor-side.
 */
UCLASS()
class ZOM_API UZomHUDWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UZomObjectiveTrackerWidget> ObjectiveTracker;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UZomAbilityBarWidget> AbilityBar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UZomInventoryWidget> InventoryWidget;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UZomDamageIndicatorWidget> DamageIndicator;
};
