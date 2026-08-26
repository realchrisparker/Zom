// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/UI/ZomDamageIndicatorWidget.h"
#include "Zom/Abilities/AttributeSets/ZomAttributeSetBase.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"


void UZomDamageIndicatorWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwningPlayerPawn()))
	{
		CachedAttributeSet = const_cast<UZomAttributeSetBase*>(ASC->GetSet<UZomAttributeSetBase>());

		if (UZomAttributeSetBase* AttributeSet = CachedAttributeSet.Get())
		{
			AttributeSet->OnDamageTaken.AddUObject(this, &UZomDamageIndicatorWidget::HandleDamageTaken);
		}
	}
}

void UZomDamageIndicatorWidget::NativeDestruct()
{
	if (UZomAttributeSetBase* AttributeSet = CachedAttributeSet.Get())
	{
		AttributeSet->OnDamageTaken.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UZomDamageIndicatorWidget::HandleDamageTaken(AActor* Instigator, float Amount)
{
	OnDamageReceived(Instigator, Amount);
}
