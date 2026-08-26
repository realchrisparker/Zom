// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Objectives/ZomExtractionPoint.h"
#include "Zom/Objectives/ZomObjectiveSubsystem.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"


AZomExtractionPoint::AZomExtractionPoint()
{
	PrimaryActorTick.bCanEverTick = false;

	ExtractionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("ExtractionVolume"));
	ExtractionVolume->InitBoxExtent(FVector(150.f, 150.f, 100.f));
	ExtractionVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ExtractionVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	ExtractionVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SetRootComponent(ExtractionVolume);

	ExtractionVolume->OnComponentBeginOverlap.AddDynamic(this, &AZomExtractionPoint::OnExtractionOverlap);
}

void AZomExtractionPoint::BeginPlay()
{
	Super::BeginPlay();

	if (UZomObjectiveSubsystem* ObjectiveSubsystem = GetGameInstance()->GetSubsystem<UZomObjectiveSubsystem>())
	{
		bUnlocked = ObjectiveSubsystem->IsStepComplete(EZomObjectiveStep::Boss);
		ObjectiveSubsystem->OnStepCompleted.AddDynamic(this, &AZomExtractionPoint::HandleObjectiveStepCompleted);
	}
}

void AZomExtractionPoint::HandleObjectiveStepCompleted(EZomObjectiveStep CompletedStep)
{
	if (CompletedStep == EZomObjectiveStep::Boss)
	{
		bUnlocked = true;
	}
}

void AZomExtractionPoint::OnExtractionOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bUnlocked)
	{
		return;
	}

	const APawn* OtherPawn = Cast<APawn>(OtherActor);
	if (!OtherPawn || !OtherPawn->IsPlayerControlled())
	{
		return;
	}

	if (UZomObjectiveSubsystem* ObjectiveSubsystem = GetGameInstance()->GetSubsystem<UZomObjectiveSubsystem>())
	{
		ObjectiveSubsystem->CompleteStep(EZomObjectiveStep::Extracted);
	}
}
