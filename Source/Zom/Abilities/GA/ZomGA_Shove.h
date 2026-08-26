// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Zom/Abilities/ZomGameplayAbility.h"
#include "ZomGA_Shove.generated.h"


/**
 * [Design] Signature ability - staggers a zombie and creates distance. Sweeps a sphere in front of the avatar,
 * applies UZomGE_Stagger to whatever it hits, and launches the avatar backward to create the distance.
 */
UCLASS()
class ZOM_API UZomGA_Shove : public UZomGameplayAbility
{
	GENERATED_BODY()

public:
	UZomGA_Shove();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	// How far in front of the avatar the shove sweep reaches.
	UPROPERTY(EditDefaultsOnly, Category = "Zom|Shove")
	float ShoveRange = 150.f;

	// Radius of the shove sweep.
	UPROPERTY(EditDefaultsOnly, Category = "Zom|Shove")
	float ShoveRadius = 75.f;

	// How long a hit target is staggered for (Zom.SetByCaller.Duration on UZomGE_Stagger).
	UPROPERTY(EditDefaultsOnly, Category = "Zom|Shove")
	float StaggerDuration = 2.f;

	// Backward launch speed applied to the avatar, creating distance from whatever was shoved.
	UPROPERTY(EditDefaultsOnly, Category = "Zom|Shove")
	float SelfLaunchSpeed = 400.f;
};
