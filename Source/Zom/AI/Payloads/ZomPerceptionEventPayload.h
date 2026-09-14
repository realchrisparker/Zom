// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZomPerceptionEventPayload.generated.h"


/**
 * Payload carried by the Zom.Perception.* tagged State Tree events UZomZombieAIComponent raises
 * (Section 5.5 of the dev doc). Wrapped via FConstStructView::Make() when calling SendStateTreeEvent.
 */
USTRUCT(BlueprintType)
struct FZomPerceptionEventPayload
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Zom|Perception")
	TWeakObjectPtr<AActor> SensedActor;

	UPROPERTY(BlueprintReadOnly, Category = "Zom|Perception")
	FVector Location = FVector::ZeroVector;
};
