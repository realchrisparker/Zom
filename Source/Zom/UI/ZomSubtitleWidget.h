// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ZomSubtitleWidget.generated.h"


class UTextBlock;


/**
 * [Design] Displays subtitle text for Boss barks, triggered off the same UAudioComponent playback as the bark
 * itself, not a separate dialogue system (Section 6 of the dev doc). C++ base only - the WBP_* visual layout
 * (binding SubtitleTextBlock) is editor-side.
 */
UCLASS()
class ZOM_API UZomSubtitleWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Zom|UI")
	void ShowSubtitle(const FText& SubtitleText, float DisplayDuration);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SubtitleTextBlock;

private:
	UFUNCTION()
	void ClearSubtitle();

	FTimerHandle ClearTimerHandle;
};
