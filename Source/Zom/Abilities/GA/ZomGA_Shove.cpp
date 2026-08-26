// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Abilities/GA/ZomGA_Shove.h"
#include "Zom/Abilities/Effects/ZomGE_Stagger.h"
#include "Zom/Misc/ZomGameplayTags.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"


UZomGA_Shove::UZomGA_Shove()
{
}

void UZomGA_Shove::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	UWorld* World = AvatarActor ? AvatarActor->GetWorld() : nullptr;

	if (AvatarActor && World)
	{
		const FVector Start = AvatarActor->GetActorLocation();
		const FVector Forward = AvatarActor->GetActorForwardVector();
		const FVector End = Start + Forward * ShoveRange;

		TArray<FHitResult> Hits;
		FCollisionShape SweepShape = FCollisionShape::MakeSphere(ShoveRadius);
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ZomGA_Shove), false, AvatarActor);

		World->SweepMultiByChannel(Hits, Start, End, FQuat::Identity, ECC_Pawn, SweepShape, QueryParams);

		UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo_Ensured();

		TSet<AActor*> AlreadyHit;
		for (const FHitResult& Hit : Hits)
		{
			AActor* HitActor = Hit.GetActor();
			if (!HitActor || HitActor == AvatarActor || AlreadyHit.Contains(HitActor))
			{
				continue;
			}
			AlreadyHit.Add(HitActor);

			if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor))
			{
				FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
				EffectContext.AddSourceObject(AvatarActor);

				const FGameplayEffectSpecHandle StaggerSpec = SourceASC->MakeOutgoingSpec(UZomGE_Stagger::StaticClass(), GetAbilityLevel(), EffectContext);
				if (StaggerSpec.IsValid())
				{
					StaggerSpec.Data->SetSetByCallerMagnitude(TAG_Zom_SetByCaller_Duration.GetTag(), StaggerDuration);
					TargetASC->ApplyGameplayEffectSpecToSelf(*StaggerSpec.Data.Get());
				}
			}

			if (ACharacter* HitCharacter = Cast<ACharacter>(HitActor))
			{
				HitCharacter->LaunchCharacter(Forward * SelfLaunchSpeed, true, false);
			}
		}

		// Create distance: launch the shover backward, away from whatever was (or wasn't) hit.
		if (ACharacter* AvatarCharacter = Cast<ACharacter>(AvatarActor))
		{
			AvatarCharacter->LaunchCharacter(-Forward * SelfLaunchSpeed, true, false);
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
