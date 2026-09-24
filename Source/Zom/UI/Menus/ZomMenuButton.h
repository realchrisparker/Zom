// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "ZomMenuButton.generated.h"


class UCommonTextBlock;
class UImage;
class UTexture2D;


/**
 * Text-label button shared by every menu. Style comes from the WBP_MenuButton's CommonButtonStyle.
 *
 * HighlightImage shows HighlightTexture while the button is hovered (mouse), focused (gamepad/keyboard) or
 * selected, and is hidden otherwise. Hover and focus are tracked separately because CommonUI reports them as
 * separate events - gamepad focus never fires NativeOnHovered.
 */
UCLASS(Abstract)
class ZOM_API UZomMenuButton : public UCommonButtonBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Zom|UI")
	void SetButtonText(const FText& InText);

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeOnInitialized() override;
	virtual void NativeOnHovered() override;
	virtual void NativeOnUnhovered() override;
	virtual void NativeOnSelected(bool bBroadcast) override;
	virtual void NativeOnDeselected(bool bBroadcast) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zom|UI")
	FText ButtonText;

	// Background shown while hovered/focused/selected. Leave empty for no highlight.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zom|UI")
	TObjectPtr<UTexture2D> HighlightTexture;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> ButtonLabel;

	// Sits behind ButtonLabel, stretched to fill the button.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> HighlightImage;

private:
	void HandleFocusReceived();
	void HandleFocusLost();
	void UpdateHighlight();

	bool bIsHoveredState = false;
	bool bHasFocusState = false;
};
