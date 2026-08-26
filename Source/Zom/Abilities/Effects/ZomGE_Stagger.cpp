// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Abilities/Effects/ZomGE_Stagger.h"
#include "Zom/Misc/ZomGameplayTags.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"


UZomGE_Stagger::UZomGE_Stagger()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;

	FSetByCallerFloat SetByCallerDuration;
	SetByCallerDuration.DataTag = TAG_Zom_SetByCaller_Duration.GetTag();
	DurationMagnitude = FGameplayEffectModifierMagnitude(SetByCallerDuration);
}

void UZomGE_Stagger::PostInitProperties()
{
	Super::PostInitProperties();

	// FindOrAddComponent() calls NewObject() internally, which asserts if used inside a UObject
	// constructor (Outer == the object under construction). PostInitProperties() runs after
	// construction completes, so this is the earliest safe place to add GE components in C++.
	FInheritedTagContainer GrantedTags;
	GrantedTags.Added.AddTag(TAG_Zom_Status_Staggered.GetTag());
	FindOrAddComponent<UTargetTagsGameplayEffectComponent>().SetAndApplyTargetTagChanges(GrantedTags);
}
