// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/AI/ZomToxicGasVolume.h"
#include "Components/SphereComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Zom/Abilities/ZomGameplayEffect.h"
#include "Zom/Misc/ZomGameplayTags.h"


AZomToxicGasVolume::AZomToxicGasVolume()
{
	PrimaryActorTick.bCanEverTick = false;

	GasSphere = CreateDefaultSubobject<USphereComponent>(TEXT("GasSphere"));
	GasSphere->InitSphereRadius(300.f);
	GasSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GasSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	GasSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SetRootComponent(GasSphere);

	GasSphere->OnComponentBeginOverlap.AddDynamic(this, &AZomToxicGasVolume::OnGasOverlap);
}

void AZomToxicGasVolume::BeginPlay()
{
	Super::BeginPlay();

	SetLifeSpan(LifeSpanSeconds);
}

void AZomToxicGasVolume::OnGasOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this || !InfectionEffectClass)
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
	if (!TargetASC)
	{
		return;
	}

	FGameplayEffectContextHandle EffectContext = TargetASC->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	const FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(InfectionEffectClass, 1.f, EffectContext);
	if (SpecHandle.IsValid())
	{
		SpecHandle.Data->SetSetByCallerMagnitude(TAG_Zom_SetByCaller_Duration.GetTag(), InfectionDuration);
		TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}
