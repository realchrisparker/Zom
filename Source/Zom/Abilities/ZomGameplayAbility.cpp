// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Abilities/ZomGameplayAbility.h"
#include "Zom/Misc/ZomGameplayTags.h"
#include "Zom/Characters/Base/ZomCharacterBase.h"
#include "MotionCombatSystem/Components/MCS_CombatCoreComponent.h"
#include "MotionCombatSystem/Structs/MCS_AttackEntry.h"


UZomGameplayAbility::UZomGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	ActivationBlockedTags.AddTag(TAG_Zom_Status_Staggered.GetTag());
}

AZomCharacterBase* UZomGameplayAbility::GetOwningCharacter() const
{
	return Cast<AZomCharacterBase>(GetAvatarActorFromActorInfo());
}

FMCS_AttackEntry UZomGameplayAbility::GetCurrentAttackEntry() const
{
	const AZomCharacterBase* OwningCharacter = GetOwningCharacter();
	const UMCS_CombatCoreComponent* CombatCore = OwningCharacter ? OwningCharacter->GetCombatCoreComponent() : nullptr;
	return CombatCore ? CombatCore->GetCurrentAttack() : FMCS_AttackEntry();
}

void UZomGameplayAbility::NotifyAttackMontageEnded() const
{
	if (const AZomCharacterBase* OwningCharacter = GetOwningCharacter())
	{
		if (UMCS_CombatCoreComponent* CombatCore = OwningCharacter->GetCombatCoreComponent())
		{
			CombatCore->OnAttackEnd.Broadcast();
		}
	}
}
