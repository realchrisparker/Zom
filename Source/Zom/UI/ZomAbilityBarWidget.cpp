// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/UI/ZomAbilityBarWidget.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffect.h"


void UZomAbilityBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CachedASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwningPlayerPawn());

	if (UAbilitySystemComponent* ASC = CachedASC.Get())
	{
		ASC->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(this, &UZomAbilityBarWidget::HandleEffectApplied);
		ASC->OnAnyGameplayEffectRemovedDelegate().AddUObject(this, &UZomAbilityBarWidget::HandleEffectRemoved);
	}
}

void UZomAbilityBarWidget::NativeDestruct()
{
	if (UAbilitySystemComponent* ASC = CachedASC.Get())
	{
		ASC->OnActiveGameplayEffectAddedDelegateToSelf.RemoveAll(this);
		ASC->OnAnyGameplayEffectRemovedDelegate().RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UZomAbilityBarWidget::HandleEffectApplied(UAbilitySystemComponent* ASC, const FGameplayEffectSpec& Spec, FActiveGameplayEffectHandle Handle)
{
	OnAbilityBarNeedsRefresh();
}

void UZomAbilityBarWidget::HandleEffectRemoved(const FActiveGameplayEffect& RemovedEffect)
{
	OnAbilityBarNeedsRefresh();
}

float UZomAbilityBarWidget::GetCooldownTimeRemaining(TSubclassOf<UGameplayEffect> CooldownEffectClass) const
{
	const UAbilitySystemComponent* ASC = CachedASC.Get();
	if (!ASC || !CooldownEffectClass)
	{
		return 0.f;
	}

	FGameplayEffectQuery Query;
	Query.EffectDefinition = CooldownEffectClass;

	float LongestRemaining = 0.f;
	for (const TPair<float, float>& TimeRemainingAndDuration : ASC->GetActiveEffectsTimeRemainingAndDuration(Query))
	{
		LongestRemaining = FMath::Max(LongestRemaining, TimeRemainingAndDuration.Key);
	}

	return LongestRemaining;
}
