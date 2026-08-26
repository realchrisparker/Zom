// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/UI/ZomObjectiveTrackerWidget.h"
#include "Zom/Objectives/ZomObjectiveSubsystem.h"


void UZomObjectiveTrackerWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UZomObjectiveSubsystem* ObjectiveSubsystem = GetGameInstance()->GetSubsystem<UZomObjectiveSubsystem>())
	{
		ObjectiveSubsystem->OnStepCompleted.AddDynamic(this, &UZomObjectiveTrackerWidget::HandleStepCompleted);
		OnObjectiveStepChanged(ObjectiveSubsystem->GetCurrentStep());
	}
}

void UZomObjectiveTrackerWidget::NativeDestruct()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UZomObjectiveSubsystem* ObjectiveSubsystem = GI->GetSubsystem<UZomObjectiveSubsystem>())
		{
			ObjectiveSubsystem->OnStepCompleted.RemoveDynamic(this, &UZomObjectiveTrackerWidget::HandleStepCompleted);
		}
	}

	Super::NativeDestruct();
}

void UZomObjectiveTrackerWidget::HandleStepCompleted(EZomObjectiveStep CompletedStep)
{
	OnObjectiveStepChanged(CompletedStep);
}
