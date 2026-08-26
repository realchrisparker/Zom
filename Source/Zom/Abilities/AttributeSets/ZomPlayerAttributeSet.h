// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Zom/Abilities/AttributeSets/ZomAttributeSetBase.h"
#include "ZomPlayerAttributeSet.generated.h"


/**
 * Player-only attributes (Stamina), owned by AZomPlayerState. See Section 4.1 of the dev doc.
 */
UCLASS()
class ZOM_API UZomPlayerAttributeSet : public UZomAttributeSetBase
{
	GENERATED_BODY()

public:
	UZomPlayerAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;

	// Current stamina. Clamped to [0, MaxStamina]. Spent by UZomGA_HeavyAttack/UZomGA_Dodge via UZomGE_StaminaDrain.
	UPROPERTY(BlueprintReadOnly, Category = "Zom|Attributes", ReplicatedUsing = OnRep_Stamina)
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS(UZomPlayerAttributeSet, Stamina)

	// Maximum stamina.
	UPROPERTY(BlueprintReadOnly, Category = "Zom|Attributes", ReplicatedUsing = OnRep_MaxStamina)
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS(UZomPlayerAttributeSet, MaxStamina)

protected:
	UFUNCTION()
	virtual void OnRep_Stamina(const FGameplayAttributeData& OldStamina);

	UFUNCTION()
	virtual void OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina);

private:
	void ClampStamina(const FGameplayAttribute& Attribute, float& NewValue) const;
};
