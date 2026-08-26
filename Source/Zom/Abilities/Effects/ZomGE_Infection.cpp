// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Abilities/Effects/ZomGE_Infection.h"
#include "Zom/Misc/ZomGameplayTags.h"


UZomGE_Infection::UZomGE_Infection()
{
	FSetByCallerFloat SetByCallerDuration;
	SetByCallerDuration.DataTag = TAG_Zom_SetByCaller_Duration.GetTag();
	DurationMagnitude = FGameplayEffectModifierMagnitude(SetByCallerDuration);

	// Stack per target so re-biting/re-gassing an already-infected target extends rather than adding a second
	// instance. Direct assignment: the SetStackingType() setter is editor-only and isn't linkable here.
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	StackingType = EGameplayEffectStackingType::AggregateByTarget;
	PRAGMA_ENABLE_DEPRECATION_WARNINGS
	StackLimitCount = 1;
	StackDurationRefreshPolicy = EGameplayEffectStackingDurationPolicy::ExtendDuration;
}
