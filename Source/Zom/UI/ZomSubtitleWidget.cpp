// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/UI/ZomSubtitleWidget.h"
#include "Components/TextBlock.h"


void UZomSubtitleWidget::ShowSubtitle(const FText& SubtitleText, float DisplayDuration)
{
	if (SubtitleTextBlock)
	{
		SubtitleTextBlock->SetText(SubtitleText);
	}

	SetVisibility(ESlateVisibility::Visible);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(ClearTimerHandle, this, &UZomSubtitleWidget::ClearSubtitle, DisplayDuration, false);
	}
}

void UZomSubtitleWidget::ClearSubtitle()
{
	SetVisibility(ESlateVisibility::Collapsed);
}
