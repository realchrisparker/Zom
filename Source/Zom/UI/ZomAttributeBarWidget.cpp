// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/UI/ZomAttributeBarWidget.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/ProgressBar.h"


void UZomAttributeBarWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (ValueBar)
	{
		ValueBar->SetFillColorAndOpacity(FillColor);
	}
}

void UZomAttributeBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CachedASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwningPlayerPawn());

	if (UAbilitySystemComponent* ASC = CachedASC.Get())
	{
		if (Attribute.IsValid())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(Attribute).AddUObject(this, &UZomAttributeBarWidget::HandleAttributeChanged);
		}

		if (MaxAttribute.IsValid())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(MaxAttribute).AddUObject(this, &UZomAttributeBarWidget::HandleAttributeChanged);
		}
	}

	OnAttributeBarValueChanged(GetCurrentValue(), GetMaxValue());
}

void UZomAttributeBarWidget::NativeDestruct()
{
	if (UAbilitySystemComponent* ASC = CachedASC.Get())
	{
		if (Attribute.IsValid())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(Attribute).RemoveAll(this);
		}

		if (MaxAttribute.IsValid())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(MaxAttribute).RemoveAll(this);
		}
	}

	Super::NativeDestruct();
}

void UZomAttributeBarWidget::HandleAttributeChanged(const FOnAttributeChangeData& Data)
{
	OnAttributeBarValueChanged(GetCurrentValue(), GetMaxValue());
}

float UZomAttributeBarWidget::GetCurrentValue() const
{
	const UAbilitySystemComponent* ASC = CachedASC.Get();
	return (ASC && Attribute.IsValid()) ? ASC->GetNumericAttribute(Attribute) : 0.f;
}

float UZomAttributeBarWidget::GetMaxValue() const
{
	const UAbilitySystemComponent* ASC = CachedASC.Get();
	return (ASC && MaxAttribute.IsValid()) ? ASC->GetNumericAttribute(MaxAttribute) : 0.f;
}

float UZomAttributeBarWidget::GetPercent() const
{
	const float MaxValue = GetMaxValue();
	return MaxValue > 0.f ? GetCurrentValue() / MaxValue : 0.f;
}
