// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/UI/Menus/ZomConfirmDialogWidget.h"
#include "Zom/UI/Menus/ZomMenuButton.h"
#include "CommonTextBlock.h"


void UZomConfirmDialogWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	ConfirmButton->OnClicked().AddUObject(this, &UZomConfirmDialogWidget::HandleConfirmClicked);
	CancelButton->OnClicked().AddUObject(this, &UZomConfirmDialogWidget::HandleCancelClicked);
}

void UZomConfirmDialogWidget::Setup(const FText& Title, const FText& Body, TFunction<void()> InOnConfirmed)
{
	TitleText->SetText(Title);
	BodyText->SetText(Body);
	OnConfirmed = MoveTemp(InOnConfirmed);
}

void UZomConfirmDialogWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	// The stack pools and reuses instances - don't let a stale callback survive into the next Setup.
	OnConfirmed = nullptr;
}

UWidget* UZomConfirmDialogWidget::NativeGetDesiredFocusTarget() const
{
	// Default to the safe choice so a stray gamepad press can't wipe a save or quit.
	return CancelButton;
}

void UZomConfirmDialogWidget::HandleConfirmClicked()
{
	// Take the callback before deactivating, since NativeOnDeactivated clears it.
	TFunction<void()> Callback = MoveTemp(OnConfirmed);

	DeactivateWidget();

	if (Callback)
	{
		Callback();
	}
}

void UZomConfirmDialogWidget::HandleCancelClicked()
{
	DeactivateWidget();
}
