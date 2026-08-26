// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Objectives/ZomFetchItem.h"
#include "Zom/Objectives/ZomObjectiveSubsystem.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Pawn.h"


AZomFetchItem::AZomFetchItem()
{
	PrimaryActorTick.bCanEverTick = false;

	PickupSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PickupSphere"));
	PickupSphere->InitSphereRadius(75.f);
	PickupSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PickupSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickupSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SetRootComponent(PickupSphere);

	PickupSphere->OnComponentBeginOverlap.AddDynamic(this, &AZomFetchItem::OnPickupOverlap);
}

void AZomFetchItem::OnPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	const APawn* OtherPawn = Cast<APawn>(OtherActor);
	if (!OtherPawn || !OtherPawn->IsPlayerControlled())
	{
		return;
	}

	if (UZomObjectiveSubsystem* ObjectiveSubsystem = GetGameInstance()->GetSubsystem<UZomObjectiveSubsystem>())
	{
		ObjectiveSubsystem->CompleteStep(EZomObjectiveStep::Fetch);
	}

	Destroy();
}
