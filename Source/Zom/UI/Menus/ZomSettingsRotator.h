// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonRotator.h"
#include "ZomSettingsRotator.generated.h"


class UButton;


/**
 * Option selector used by UZomSettingsWidget. Keyboard/gamepad left-right already rotate a UCommonRotator;
 * this adds clickable arrow buttons for mouse users.
 *
 * The arrows must have Is Focusable unticked: if they could take focus, gamepad navigation would land on them
 * instead of the rotator, and left/right would move focus between arrows rather than change the option.
 * UButton only exposes that as a designer setting, so it's enforced as a Widget Blueprint compile error.
 */
UCLASS(Abstract)
class ZOM_API UZomSettingsRotator : public UCommonRotator
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;

#if WITH_EDITOR
	virtual void ValidateCompiledWidgetTree(const UWidgetTree& BlueprintWidgetTree, IWidgetCompilerLog& CompileLog) const override;
#endif

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> LeftArrowButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> RightArrowButton;

private:
	UFUNCTION()
	void HandleLeftArrowClicked();

	UFUNCTION()
	void HandleRightArrowClicked();
};
