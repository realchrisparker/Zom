// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Zom/UI/Menus/ZomMenuWidget.h"
#include "ZomConfirmDialogWidget.generated.h"


class UCommonTextBlock;
class UZomMenuButton;


/**
 * Yes/No modal pushed onto the same MenuStack as the menu that asked (via AZomHUD::ShowConfirmDialog). Cancel
 * and Back just close it; Confirm closes it and then runs the callback.
 */
UCLASS(Abstract)
class ZOM_API UZomConfirmDialogWidget : public UZomMenuWidget
{
	GENERATED_BODY()

public:
	void Setup(const FText& Title, const FText& Body, TFunction<void()> InOnConfirmed);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnDeactivated() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> TitleText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> BodyText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UZomMenuButton> ConfirmButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UZomMenuButton> CancelButton;

private:
	void HandleConfirmClicked();
	void HandleCancelClicked();

	TFunction<void()> OnConfirmed;
};
