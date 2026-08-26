// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Objectives/ZomRepairTarget.h"
#include "Zom/Objectives/ZomObjectiveSubsystem.h"


AZomRepairTarget::AZomRepairTarget()
{
	PrimaryActorTick.bCanEverTick = false;

	RepairTargetRoot = CreateDefaultSubobject<USceneComponent>(TEXT("RepairTargetRoot"));
	SetRootComponent(RepairTargetRoot);
}

void AZomRepairTarget::Interact(AActor* Interactor)
{
	if (bRepaired)
	{
		return;
	}
	bRepaired = true;

	if (UZomObjectiveSubsystem* ObjectiveSubsystem = GetGameInstance()->GetSubsystem<UZomObjectiveSubsystem>())
	{
		ObjectiveSubsystem->CompleteStep(EZomObjectiveStep::Repair);
	}
}
