// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Zom/Abilities/Effects/Base/ZomGameplayEffectBase.h"
#include "ZomGE_Stagger.generated.h"


/**
 * Applied by UZomGA_Shove and by heavy player hits. Grants Zom.Status.Staggered to the target for a
 * SetByCaller-supplied duration (Zom.SetByCaller.Duration) - UZomGameplayAbilityBase already blocks activation
 * while this tag is present, so no per-ability stagger check is needed.
 */
UCLASS()
class ZOM_API UZomGE_Stagger : public UZomGameplayEffectBase
{
	GENERATED_BODY()

public:
	UZomGE_Stagger();

	virtual void PostInitProperties() override;
};
