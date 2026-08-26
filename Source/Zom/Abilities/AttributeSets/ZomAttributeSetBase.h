// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "ZomAttributeSetBase.generated.h"


// Standard GAS attribute accessor macro (GetXAttribute/GetX/SetX/InitX). Defined once here since it isn't
// provided by the engine itself; every Zom attribute set includes this header (directly or via inheritance).
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)


/**
 * Shared attribute set base for every humanoid in Zom (player, zombies, Boss). Health/Damage clamping and the
 * Damage-meta-attribute-to-Health conversion live here once so every subclass inherits correct behavior instead
 * of three copies of the same logic (Section 4.1 of the dev doc).
 */
// [New, not in the dev doc's Section 4] UZomDamageIndicatorWidget (Section 12) needs a data source for
// "directional" - the doc names the widget's behavior but never says what broadcasts direction. Added here.
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnZomDamageTaken, AActor* /*Instigator*/, float /*Amount*/);


UCLASS()
class ZOM_API UZomAttributeSetBase : public UAttributeSet
{
	GENERATED_BODY()

public:
	UZomAttributeSetBase();

	// -------------
	// Functions
	// -------------

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Broadcasts on this attribute set's owner whenever the Damage meta-attribute converts into a -Health
	// delta, carrying the effect's instigator - UZomDamageIndicatorWidget's data source.
	FOnZomDamageTaken OnDamageTaken;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	// -------------
	// Properties
	// -------------

	// Current health. Clamped to [0, MaxHealth].
	UPROPERTY(BlueprintReadOnly, Category = "Zom|Attributes", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UZomAttributeSetBase, Health)

	// Maximum health.
	UPROPERTY(BlueprintReadOnly, Category = "Zom|Attributes", ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UZomAttributeSetBase, MaxHealth)

	// Meta-attribute: GameplayEffects write incoming damage magnitude here via a Modifier; PostGameplayEffectExecute
	// converts it into a -Health delta and resets it to zero. Never replicated (transient, server-only bookkeeping).
	UPROPERTY(BlueprintReadOnly, Category = "Zom|Attributes")
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(UZomAttributeSetBase, Damage)

	// Current move speed. Clamped to [0, MaxMoveSpeed]. GAS-modifiable so slow/haste effects work on any humanoid.
	UPROPERTY(BlueprintReadOnly, Category = "Zom|Attributes", ReplicatedUsing = OnRep_MoveSpeed)
	FGameplayAttributeData MoveSpeed;
	ATTRIBUTE_ACCESSORS(UZomAttributeSetBase, MoveSpeed)

	// Maximum move speed.
	UPROPERTY(BlueprintReadOnly, Category = "Zom|Attributes", ReplicatedUsing = OnRep_MaxMoveSpeed)
	FGameplayAttributeData MaxMoveSpeed;
	ATTRIBUTE_ACCESSORS(UZomAttributeSetBase, MaxMoveSpeed)

protected:
	UFUNCTION()
	virtual void OnRep_Health(const FGameplayAttributeData& OldHealth);

	UFUNCTION()
	virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);

	UFUNCTION()
	virtual void OnRep_MoveSpeed(const FGameplayAttributeData& OldMoveSpeed);

	UFUNCTION()
	virtual void OnRep_MaxMoveSpeed(const FGameplayAttributeData& OldMaxMoveSpeed);

private:
	// Shared [0, Max] clamp used by both PreAttributeChange (current value) and PreAttributeBaseChange (base value).
	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;
};
