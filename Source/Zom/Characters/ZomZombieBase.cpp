// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Characters/ZomZombieBase.h"
#include "AbilitySystemComponent.h"
#include "Zom/AI/Controllers/ZomZombieAIController.h"
#include "Zom/Characters/Data/ZombieTypeData.h"
#include "Zom/AI/ZomZombiePoolSubsystem.h"
#include "Zom/AI/ZomToxicGasVolume.h"
#include "Zom/Abilities/AttributeSets/ZomZombieAttributeSet.h"
#include "Zom/Misc/ZomLogChannels.h"
#include "Perception/AISense_Damage.h"


AZomZombieBase::AZomZombieBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Owns its own ASC/AttributeSet, unlike the player (whose ASC lives on AZomPlayerState) - assigned into
	// the inherited AbilitySystemComponent pointer, not a separate member.
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	ZombieAttributeSet = CreateDefaultSubobject<UZomZombieAttributeSet>(TEXT("ZombieAttributeSet"));

	// Perception and the State Tree live on AZomZombieAIController now, not a component on this pawn.
	AIControllerClass = AZomZombieAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	ToxicGasVolumeClass = AZomToxicGasVolume::StaticClass();
}

void AZomZombieBase::BeginPlay()
{
	Super::BeginPlay();

	InitializeAbilitySystem(this, this);

	if (ZombieAttributeSet)
	{
		ZombieAttributeSet->OnDamageTaken.AddUObject(this, &AZomZombieBase::HandleDamageTaken);
	}

	InitializeForType(ZombieTypeData);
}

void AZomZombieBase::InitializeForType(UZombieTypeData* InTypeData)
{
	ZombieTypeData = InTypeData;

	if (!ZombieTypeData || !ZombieAttributeSet)
	{
		return;
	}

	// UZombieTypeData::Health/Speed/AttackDamage are *initial* values only (Section 4.1) - seeded here, the
	// live attribute is authoritative afterward, until the next InitializeForType (pooled reactivation).
	// InitX (not SetX) - SetHealth clamps against the current MaxHealth, which is still 0 the first time this
	// runs, so SetHealth-before-SetMaxHealth was clamping Health straight to 0 and killing the zombie on spawn.
	ZombieAttributeSet->InitMaxHealth(ZombieTypeData->Health);
	ZombieAttributeSet->InitHealth(ZombieTypeData->Health);
	ZombieAttributeSet->InitMaxMoveSpeed(ZombieTypeData->Speed);
	ZombieAttributeSet->InitMoveSpeed(ZombieTypeData->Speed);
	ZombieAttributeSet->SetAttackDamage(ZombieTypeData->AttackDamage);

	// The AIController possesses once and is never re-spawned (pooled reactivation just hides/shows the pawn),
	// so it needs an explicit nudge to reconfigure perception for the new type - OnPossess only covers the
	// first activation.
	if (AZomZombieAIController* AIController = Cast<AZomZombieAIController>(GetController()))
	{
		AIController->ConfigureForType(ZombieTypeData);
	}
}

void AZomZombieBase::HandleDamageTaken(AActor* DamageInstigator, float Amount)
{
	UAISense_Damage::ReportDamageEvent(this, this, DamageInstigator, Amount, GetActorLocation(), GetActorLocation());
}

void AZomZombieBase::HandleDeath()
{
	if (ZombieTypeData && ZombieTypeData->Category == EZomZombieCategory::Bloater && ToxicGasVolumeClass)
	{
		GetWorld()->SpawnActor<AZomToxicGasVolume>(ToxicGasVolumeClass, GetActorLocation(), GetActorRotation());
	}

	if (UZomZombiePoolSubsystem* PoolSubsystem = GetWorld()->GetSubsystem<UZomZombiePoolSubsystem>())
	{
		PoolSubsystem->ReleaseZombie(this);
	}
	else
	{
		UE_LOG(LogZomCharacter, Warning, TEXT("%s died but no UZomZombiePoolSubsystem was found - not returned to a pool."), *GetName());
	}
}
