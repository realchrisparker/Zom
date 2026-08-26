// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ZomDamageIndicatorWidget.generated.h"


class UZomAttributeSetBase;


/**
 * [Design] Directional damage indicator, since the third-person camera doesn't always show the threat
 * (Section 12 of the dev doc). Binds to UZomAttributeSetBase::OnDamageTaken; OnDamageReceived hands the
 * instigator's world location to Blueprint, which computes screen-space direction (a camera-relative
 * concern best left to UMG/materials) and drives the WBP_* visual.
 */
UCLASS()
class ZOM_API UZomDamageIndicatorWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void HandleDamageTaken(AActor* Instigator, float Amount);

	UFUNCTION(BlueprintImplementableEvent, Category = "Zom|UI")
	void OnDamageReceived(AActor* Instigator, float Amount);

private:
	TWeakObjectPtr<UZomAttributeSetBase> CachedAttributeSet;
};
