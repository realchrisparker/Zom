// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Characters/ZomBoss.h"
#include "AbilitySystemComponent.h"
#include "Components/StateTreeComponent.h"
#include "Components/AudioComponent.h"
#include "Zom/Abilities/AttributeSets/ZomZombieAttributeSet.h"
#include "Zom/Abilities/AttributeSets/ZomAttributeSetBase.h"
#include "Zom/Characters/ZomBossData.h"
#include "Zom/UI/ZomSubtitleWidget.h"
#include "Zom/Objectives/ZomObjectiveSubsystem.h"
#include "Zom/Misc/ZomGameplayTags.h"
#include "Zom/Misc/ZomLogChannels.h"


AZomBoss::AZomBoss(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	ZombieAttributeSet = CreateDefaultSubobject<UZomZombieAttributeSet>(TEXT("ZombieAttributeSet"));

	// Bespoke tree asset, separate from the crowd base tree.
	StateTreeComponent = CreateDefaultSubobject<UStateTreeComponent>(TEXT("StateTreeComponent"));

	DialogueAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("DialogueAudioComponent"));
	DialogueAudioComponent->SetupAttachment(RootComponent);
	DialogueAudioComponent->bAutoActivate = false;
}

void AZomBoss::BeginPlay()
{
	Super::BeginPlay();

	InitializeAbilitySystem(this, this);

	if (BossData && ZombieAttributeSet)
	{
		// InitX (not SetX) - SetHealth clamps against the current MaxHealth, which is still 0 the first time
		// this runs, so SetHealth-before-SetMaxHealth was clamping Health straight to 0 on spawn.
		ZombieAttributeSet->InitMaxHealth(BossData->Health);
		ZombieAttributeSet->InitHealth(BossData->Health);
		ZombieAttributeSet->SetAttackDamage(BossData->AttackDamage);
	}

	if (AbilitySystemComponent)
	{
		// Independent of the base class's own health-changed binding (which drives HandleDeath) - one shared
		// delegate, two consumers, per Section 6/4.6 of the dev doc.
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UZomAttributeSetBase::GetHealthAttribute()).AddUObject(this, &AZomBoss::HandleHealthAttributeChanged);
	}

	if (BossData)
	{
		bEncounterStartBarkPlayed = true;
		PlayBark(BossData->EncounterStartBark);
	}
}

void AZomBoss::HandleHealthAttributeChanged(const FOnAttributeChangeData& Data)
{
	if (bPhaseTwoEntered || !BossData || !AbilitySystemComponent)
	{
		return;
	}

	const float MaxHealth = ZombieAttributeSet ? ZombieAttributeSet->GetMaxHealth() : 0.f;
	if (MaxHealth <= 0.f)
	{
		return;
	}

	const float HealthFraction = Data.NewValue / MaxHealth;
	if (HealthFraction <= BossData->PhaseTwoHealthThreshold)
	{
		bPhaseTwoEntered = true;
		AbilitySystemComponent->AddLooseGameplayTag(TAG_Zom_Boss_Phase2.GetTag());
		PlayBark(BossData->MidFightTauntBark);
	}
}

void AZomBoss::PlayBark(const FZomBossDialogueBark& Bark)
{
	if (Bark.Sound && DialogueAudioComponent)
	{
		DialogueAudioComponent->SetSound(Bark.Sound);
		DialogueAudioComponent->Play();
	}

	if (SubtitleWidgetInstance && !Bark.SubtitleText.IsEmpty())
	{
		SubtitleWidgetInstance->ShowSubtitle(Bark.SubtitleText, 4.f);
	}
}

void AZomBoss::HandleDeath()
{
	if (BossData)
	{
		PlayBark(BossData->DeathBark);
	}

	if (UZomObjectiveSubsystem* ObjectiveSubsystem = GetGameInstance()->GetSubsystem<UZomObjectiveSubsystem>())
	{
		// Routes through the objective subsystem rather than a direct AZomExtractionPoint reference, keeping
		// the dependency direction consistent with Section 8 - AZomExtractionPoint already listens for this.
		ObjectiveSubsystem->CompleteStep(EZomObjectiveStep::Boss);
	}
	else
	{
		UE_LOG(LogZomCharacter, Warning, TEXT("%s died but no UZomObjectiveSubsystem was found - extraction won't unlock."), *GetName());
	}
}
