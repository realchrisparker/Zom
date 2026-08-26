// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Game/ZomPlayerState.h"
#include "AbilitySystemComponent.h"
#include "Zom/Abilities/AttributeSets/ZomPlayerAttributeSet.h"


AZomPlayerState::AZomPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	PlayerAttributeSet = CreateDefaultSubobject<UZomPlayerAttributeSet>(TEXT("PlayerAttributeSet"));
}

UAbilitySystemComponent* AZomPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
