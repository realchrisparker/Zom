// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AttributeSet.h"
#include "ZomAttributeBarWidget.generated.h"


class UAbilitySystemComponent;
class UProgressBar;
struct FOnAttributeChangeData;


/**
 * Reusable display for a single GAS attribute value/max pair (Section 12 of the dev doc). One class
 * drives every vitals bar (health, stamina, ...) - Attribute/MaxAttribute are configured per-instance
 * (e.g. in the HUD's widget tree) rather than needing a subclass per stat. C++ resolves the owning
 * player's ASC, binds to both attributes' value-change delegates, and re-fires OnAttributeBarValueChanged;
 * Blueprint drives the WBP_* visual (bar fill, text, etc.) and may also pull GetCurrentValue/GetMaxValue/
 * GetPercent on demand.
 */
UCLASS()
class ZOM_API UZomAttributeBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// The attribute this bar displays, e.g. Health or Stamina. Set per-instance.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zom|UI")
	FGameplayAttribute Attribute;

	// The paired maximum attribute, e.g. MaxHealth or MaxStamina.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zom|UI")
	FGameplayAttribute MaxAttribute;

	// Fill color applied to ValueBar. Set per-instance (e.g. red for health, green for stamina).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zom|UI")
	FLinearColor FillColor = FLinearColor::White;

	UFUNCTION(BlueprintPure, Category = "Zom|UI")
	float GetCurrentValue() const;

	UFUNCTION(BlueprintPure, Category = "Zom|UI")
	float GetMaxValue() const;

	// GetCurrentValue() / GetMaxValue(), or 0 if GetMaxValue() is <= 0.
	UFUNCTION(BlueprintPure, Category = "Zom|UI")
	float GetPercent() const;

protected:
	// The bar whose fill color FillColor is applied to. Optional so subclasses/WBPs that don't name a
	// child "ValueBar" still compile; FillColor simply has no effect without it.
	UPROPERTY(BlueprintReadOnly, Category = "Zom|UI", meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> ValueBar;

	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// Fired once in NativeConstruct to seed initial UI state, then again whenever Attribute or
	// MaxAttribute changes.
	UFUNCTION(BlueprintImplementableEvent, Category = "Zom|UI")
	void OnAttributeBarValueChanged(float CurrentValue, float MaxValue);

private:
	void HandleAttributeChanged(const FOnAttributeChangeData& Data);

	TWeakObjectPtr<UAbilitySystemComponent> CachedASC;
};
