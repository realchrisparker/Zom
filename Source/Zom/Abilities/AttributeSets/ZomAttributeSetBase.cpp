// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Abilities/AttributeSets/ZomAttributeSetBase.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"


UZomAttributeSetBase::UZomAttributeSetBase()
{
}

void UZomAttributeSetBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Damage is a transient meta-attribute, reset to zero within the same server frame it's applied, so it's
	// intentionally left out of replication.
	DOREPLIFETIME_CONDITION_NOTIFY(UZomAttributeSetBase, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UZomAttributeSetBase, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UZomAttributeSetBase, MoveSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UZomAttributeSetBase, MaxMoveSpeed, COND_None, REPNOTIFY_Always);
}

void UZomAttributeSetBase::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	else if (Attribute == GetMoveSpeedAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxMoveSpeed());
	}
}

void UZomAttributeSetBase::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	ClampAttribute(Attribute, NewValue);
}

void UZomAttributeSetBase::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);

	ClampAttribute(Attribute, NewValue);
}

void UZomAttributeSetBase::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		// Damage -> -Health conversion (Section 4.1): a single UZomGE_Damage effect writes a magnitude into this
		// shared meta-attribute regardless of victim type, so both player and zombie damage funnel through here.
		const float LocalDamage = GetDamage();
		SetDamage(0.f);

		if (LocalDamage > 0.f)
		{
			SetHealth(FMath::Clamp(GetHealth() - LocalDamage, 0.f, GetMaxHealth()));
			OnDamageTaken.Broadcast(Data.EffectSpec.GetEffectContext().GetInstigator(), LocalDamage);
		}
	}
	else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
	}
	else if (Data.EvaluatedData.Attribute == GetMoveSpeedAttribute())
	{
		SetMoveSpeed(FMath::Clamp(GetMoveSpeed(), 0.f, GetMaxMoveSpeed()));
	}
}

void UZomAttributeSetBase::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UZomAttributeSetBase, Health, OldHealth);
}

void UZomAttributeSetBase::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UZomAttributeSetBase, MaxHealth, OldMaxHealth);
}

void UZomAttributeSetBase::OnRep_MoveSpeed(const FGameplayAttributeData& OldMoveSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UZomAttributeSetBase, MoveSpeed, OldMoveSpeed);
}

void UZomAttributeSetBase::OnRep_MaxMoveSpeed(const FGameplayAttributeData& OldMaxMoveSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UZomAttributeSetBase, MaxMoveSpeed, OldMaxMoveSpeed);
}
