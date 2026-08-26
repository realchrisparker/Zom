// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/AI/Controllers/ZomZombieAIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Damage.h"
#include "Components/StateTreeAIComponent.h"
#include "GameFramework/Character.h"
#include "StructUtils/StructView.h"
#include "Zom/Characters/ZomZombieBase.h"
#include "Zom/AI/ZombieTypeData.h"
#include "Zom/AI/ZomPerceptionEventPayload.h"
#include "Zom/Misc/ZomGameplayTags.h"


AZomZombieAIController::AZomZombieAIController()
{
	// Assigned into AAIController's own inherited PerceptionComponent pointer - it already declares one,
	// redeclaring it in this class would be a UHT shadowing error.
	SetPerceptionComponent(*CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent")));

	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
	HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;

	// No radius, no affiliation filter to configure (UAISenseConfig_Damage doesn't have one) - Damage is an
	// explicit UAISense_Damage::ReportDamageEvent() call (made by AZomZombieBase whenever it takes damage),
	// not a passive proximity sense like Sight/Hearing.
	DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageConfig"));

	// Concrete implementation behind the Section 5.2 claim that Auds/Eyes are "the same component with one
	// sense's config zeroed out" - the actual radii come from UZombieTypeData in ConfigureForType, not here.
	PerceptionComponent->ConfigureSense(*SightConfig);
	PerceptionComponent->ConfigureSense(*HearingConfig);
	PerceptionComponent->ConfigureSense(*DamageConfig);
	PerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

	PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AZomZombieAIController::HandleTargetPerceptionUpdated);

	// Bespoke State Tree asset assigned on this component in the editor - separate from the crowd base tree
	// is the wrong phrasing here since this *is* the crowd tree; Boss keeps its own StateTreeComponent
	// directly on the pawn instead of an AIController, per its explicit "doesn't get this component" note.
	StateTreeComponent = CreateDefaultSubobject<UStateTreeAIComponent>(TEXT("StateTreeComponent"));
}

void AZomZombieAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	const AZomZombieBase* OwningZombie = Cast<AZomZombieBase>(InPawn);
	ConfigureForType(OwningZombie ? OwningZombie->GetZombieTypeData() : nullptr);
}

void AZomZombieAIController::ConfigureForType(const UZombieTypeData* TypeData)
{
	if (!TypeData)
	{
		return;
	}

	SightConfig->SightRadius = TypeData->SightRadius;
	SightConfig->LoseSightRadius = TypeData->SightRadius + 50.f;
	SightConfig->PeripheralVisionAngleDegrees = TypeData->SightAngleDegrees;

	HearingConfig->HearingRange = TypeData->HearingRadius;

	PerceptionComponent->ConfigureSense(*SightConfig);
	PerceptionComponent->ConfigureSense(*HearingConfig);

	// Forget any previously perceived actors/stimuli from a prior activation (relevant on pooled reactivation).
	PerceptionComponent->ForgetAll();

	CurrentTarget.Reset();
	LastKnownTargetLocation = FVector::ZeroVector;
}

void AZomZombieAIController::PauseBrain()
{
	if (StateTreeComponent)
	{
		StateTreeComponent->StopLogic(TEXT("Pooled"));
	}
}

void AZomZombieAIController::ResumeBrain()
{
	if (StateTreeComponent)
	{
		StateTreeComponent->StartLogic();
	}
}

void AZomZombieAIController::HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Actor || !StateTreeComponent)
	{
		return;
	}

	FZomPerceptionEventPayload Payload;
	Payload.SensedActor = Actor;
	Payload.Location = Stimulus.StimulusLocation;

	const bool bIsSight = Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>();
	const bool bIsHearing = Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>();
	const bool bIsDamage = Stimulus.Type == UAISense::GetSenseID<UAISense_Damage>();

	if (bIsSight)
	{
		LastKnownTargetLocation = Stimulus.StimulusLocation;

		if (Stimulus.WasSuccessfullySensed())
		{
			CurrentTarget = Cast<ACharacter>(Actor);
			StateTreeComponent->SendStateTreeEvent(TAG_Zom_Perception_Sight_TargetSeen.GetTag(), FConstStructView::Make(Payload));
		}
		else
		{
			StateTreeComponent->SendStateTreeEvent(TAG_Zom_Perception_Sight_TargetLost.GetTag(), FConstStructView::Make(Payload));
		}
	}
	else if (bIsHearing && Stimulus.WasSuccessfullySensed())
	{
		CurrentTarget = Cast<ACharacter>(Actor);
		LastKnownTargetLocation = Stimulus.StimulusLocation;
		StateTreeComponent->SendStateTreeEvent(TAG_Zom_Perception_Hearing_NoiseHeard.GetTag(), FConstStructView::Make(Payload));
	}
	else if (bIsDamage && Stimulus.WasSuccessfullySensed())
	{
		// Actor here is the damage instigator (see UAISense_Damage::RegisterWrappedEvent), not the zombie
		// itself - reacting to who hit it, regardless of sight/hearing range.
		CurrentTarget = Cast<ACharacter>(Actor);
		LastKnownTargetLocation = Stimulus.StimulusLocation;
		StateTreeComponent->SendStateTreeEvent(TAG_Zom_Perception_Damage_Taken.GetTag(), FConstStructView::Make(Payload));
	}
}

bool AZomZombieAIController::HasValidTarget() const
{
	return CurrentTarget.IsValid();
}

AActor* AZomZombieAIController::GetCurrentTarget() const
{
	return CurrentTarget.Get();
}

FVector AZomZombieAIController::GetLastKnownTargetLocation() const
{
	return LastKnownTargetLocation;
}
