// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Zom/Objectives/ZomObjectiveEnums.h"
#include "ZomObjectiveTrackerWidget.generated.h"


/**
 * [Design] Bound to UZomObjectiveSubsystem delegates (Section 12 of the dev doc). C++ handles the
 * subsystem-binding lifecycle; OnObjectiveStepChanged is a BlueprintImplementableEvent so the WBP_* visual
 * update (text/icon changes) stays editor-side.
 */
UCLASS()
class ZOM_API UZomObjectiveTrackerWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void HandleStepCompleted(EZomObjectiveStep CompletedStep);

	UFUNCTION(BlueprintImplementableEvent, Category = "Zom|UI")
	void OnObjectiveStepChanged(EZomObjectiveStep NewStep);
};
