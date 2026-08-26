// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Objectives/ZomObjectiveSubsystem.h"
#include "Zom/Characters/Base/ZomPlayerCharacterBase.h"
#include "Zom/Misc/ZomGameplayTags.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemComponent.h"


FGameplayTag UZomObjectiveSubsystem::GetCompletionTagForStep(EZomObjectiveStep Step)
{
	switch (Step)
	{
	case EZomObjectiveStep::Fetch:		return TAG_Zom_Objective_Fetch_Complete.GetTag();
	case EZomObjectiveStep::Repair:		return TAG_Zom_Objective_Repair_Complete.GetTag();
	case EZomObjectiveStep::Defend:		return TAG_Zom_Objective_Defend_Complete.GetTag();
	case EZomObjectiveStep::Boss:		return TAG_Zom_Objective_Boss_Complete.GetTag();
	case EZomObjectiveStep::Extracted:	return TAG_Zom_Objective_Extracted_Complete.GetTag();
	default:							return FGameplayTag();
	}
}

void UZomObjectiveSubsystem::MirrorTagForStep(EZomObjectiveStep Step) const
{
	AZomPlayerCharacterBase* PlayerCharacter = Cast<AZomPlayerCharacterBase>(UGameplayStatics::GetPlayerCharacter(GetGameInstance(), 0));
	UAbilitySystemComponent* ASC = PlayerCharacter ? PlayerCharacter->GetAbilitySystemComponent() : nullptr;

	const FGameplayTag CompletionTag = GetCompletionTagForStep(Step);
	if (ASC && CompletionTag.IsValid())
	{
		ASC->AddLooseGameplayTag(CompletionTag);
	}
}

void UZomObjectiveSubsystem::CompleteStep(EZomObjectiveStep Step)
{
	CurrentStep = Step;
	CompletedSteps.Add(Step);

	MirrorTagForStep(Step);

	OnStepCompleted.Broadcast(Step);
}

void UZomObjectiveSubsystem::RestoreStep(EZomObjectiveStep Step)
{
	CurrentStep = Step;

	for (uint8 StepIndex = 0; StepIndex <= static_cast<uint8>(Step); ++StepIndex)
	{
		const EZomObjectiveStep PriorStep = static_cast<EZomObjectiveStep>(StepIndex);
		CompletedSteps.Add(PriorStep);
		MirrorTagForStep(PriorStep);
	}
}
