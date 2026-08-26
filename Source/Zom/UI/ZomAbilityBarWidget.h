// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ZomAbilityBarWidget.generated.h"


class UAbilitySystemComponent;
class UGameplayEffect;
struct FGameplayEffectSpec;
struct FActiveGameplayEffectHandle;
struct FActiveGameplayEffect;


/**
 * Bound to ASC cooldown delegates (Section 12 of the dev doc). C++ resolves the owning player's ASC
 * and re-fires OnAbilityBarNeedsRefresh whenever any GameplayEffect is added/removed on it (covers cooldowns
 * without needing per-ability cooldown tags the doc doesn't name); Blueprint queries GetCooldownTimeRemaining
 * per ability slot and updates the WBP_* visuals.
 */
UCLASS()
class ZOM_API UZomAbilityBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Remaining time (seconds) on the longest-lived active instance of CooldownEffectClass on the owning
	// player's ASC, or 0 if none is active.
	UFUNCTION(BlueprintCallable, Category = "Zom|UI")
	float GetCooldownTimeRemaining(TSubclassOf<UGameplayEffect> CooldownEffectClass) const;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Zom|UI")
	void OnAbilityBarNeedsRefresh();

private:
	void HandleEffectApplied(UAbilitySystemComponent* ASC, const FGameplayEffectSpec& Spec, FActiveGameplayEffectHandle Handle);
	void HandleEffectRemoved(const FActiveGameplayEffect& RemovedEffect);

	TWeakObjectPtr<UAbilitySystemComponent> CachedASC;
};
