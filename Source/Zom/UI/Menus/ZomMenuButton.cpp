// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/UI/Menus/ZomMenuButton.h"
#include "CommonTextBlock.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"


void UZomMenuButton::NativePreConstruct()
{
	Super::NativePreConstruct();

	// PreConstruct so the label and texture also show in the UMG designer.
	SetButtonText(ButtonText);

	if (HighlightImage && HighlightTexture)
	{
		HighlightImage->SetBrushFromTexture(HighlightTexture);
	}

	UpdateHighlight();
}

void UZomMenuButton::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	OnFocusReceived().AddUObject(this, &UZomMenuButton::HandleFocusReceived);
	OnFocusLost().AddUObject(this, &UZomMenuButton::HandleFocusLost);
}

void UZomMenuButton::SetButtonText(const FText& InText)
{
	ButtonText = InText;

	if (ButtonLabel)
	{
		ButtonLabel->SetText(ButtonText);
	}
}

void UZomMenuButton::NativeOnHovered()
{
	Super::NativeOnHovered();

	bIsHoveredState = true;
	UpdateHighlight();
}

void UZomMenuButton::NativeOnUnhovered()
{
	Super::NativeOnUnhovered();

	bIsHoveredState = false;
	UpdateHighlight();
}

void UZomMenuButton::NativeOnSelected(bool bBroadcast)
{
	Super::NativeOnSelected(bBroadcast);

	UpdateHighlight();
}

void UZomMenuButton::NativeOnDeselected(bool bBroadcast)
{
	Super::NativeOnDeselected(bBroadcast);

	UpdateHighlight();
}

void UZomMenuButton::HandleFocusReceived()
{
	bHasFocusState = true;
	UpdateHighlight();
}

void UZomMenuButton::HandleFocusLost()
{
	bHasFocusState = false;
	UpdateHighlight();
}

void UZomMenuButton::UpdateHighlight()
{
	if (!HighlightImage)
	{
		return;
	}

	const bool bHighlighted = HighlightTexture && (bIsHoveredState || bHasFocusState || GetSelected());

	// Hidden rather than Collapsed so the button's layout doesn't shift when the highlight toggles.
	HighlightImage->SetVisibility(bHighlighted ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
}
