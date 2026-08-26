// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ZomInventoryWidget.generated.h"


class UZomInventoryComponent;


/**
 * Grid view over UZomInventoryComponent (Section 12 of the dev doc). C++ resolves the owning
 * player's inventory component and binds to its delegate; the WBP_* grid layout is editor-side.
 */
UCLASS()
class ZOM_API UZomInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Zom|UI")
	UZomInventoryComponent* GetInventoryComponent() const { return CachedInventory; }

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void HandleInventoryChanged();

	UFUNCTION(BlueprintImplementableEvent, Category = "Zom|UI")
	void OnInventoryUpdated();

private:
	UPROPERTY()
	TObjectPtr<UZomInventoryComponent> CachedInventory;
};
