// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Characters/Base/ZomPlayerCharacterBase.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Zom/Abilities/AttributeSets/ZomAttributeSetBase.h"
#include "Zom/Abilities/ZomAbilitySetData.h"
#include "Zom/Abilities/ZomGameplayAbility.h"
#include "Zom/Abilities/ZomGameplayEffect.h"


// Sets default values
AZomPlayerCharacterBase::AZomPlayerCharacterBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// State Tree and GAS event-driven logic cover behavior without polling; leaf classes opt back in if they
	// have a proven per-frame need (e.g. AZomPlayerCharacter, for its locomotion/anim pipeline).
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AZomPlayerCharacterBase::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AZomPlayerCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AZomPlayerCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

// Returns the cached AbilitySystemComponent pointer. Never re-resolves it; InitializeAbilitySystem populates it.
UAbilitySystemComponent* AZomPlayerCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

// Caches the AbilitySystemComponent resolved off OwnerActor and calls InitAbilityActorInfo(OwnerActor, AvatarActor).
void AZomPlayerCharacterBase::InitializeAbilitySystem(AActor* InOwnerActor, AActor* InAvatarActor)
{
	if (!InOwnerActor)
	{
		return;
	}

	IAbilitySystemInterface* OwnerAbilitySystemInterface = Cast<IAbilitySystemInterface>(InOwnerActor);
	AbilitySystemComponent = OwnerAbilitySystemInterface ? OwnerAbilitySystemInterface->GetAbilitySystemComponent() : nullptr;

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(InOwnerActor, InAvatarActor);

		// RemoveAll before AddUObject: InitializeAbilitySystem can run more than once for the same ASC
		// (e.g. AZomPlayerCharacter calls it from both PossessedBy and OnRep_PlayerState), and this guards
		// against binding the same handler twice rather than HandleDeath firing multiple times per death.
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UZomAttributeSetBase::GetHealthAttribute()).RemoveAll(this);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UZomAttributeSetBase::GetHealthAttribute()).AddUObject(this, &AZomPlayerCharacterBase::OnHealthAttributeChanged);
	}
}

// Iterates the ability classes and starting effects on AbilitySetData and grants/applies them through the cached ASC.
void AZomPlayerCharacterBase::GrantAbilitySet(const UZomAbilitySetData* AbilitySetData)
{
	if (!AbilitySetData || !AbilitySystemComponent)
	{
		return;
	}

	for (const TSubclassOf<UZomGameplayAbility>& AbilityClass : AbilitySetData->GrantedAbilities)
	{
		if (AbilityClass)
		{
			AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1, INDEX_NONE, this));
		}
	}

	for (const TSubclassOf<UZomGameplayEffect>& EffectClass : AbilitySetData->StartingEffects)
	{
		if (EffectClass)
		{
			ApplyGameplayEffectToSelf(EffectClass);
		}
	}
}

// Small wrapper around the MakeOutgoingSpec/ApplyGameplayEffectSpecToSelf boilerplate for self-applied effects.
void AZomPlayerCharacterBase::ApplyGameplayEffectToSelf(TSubclassOf<UGameplayEffect> EffectClass, float Level)
{
	if (!AbilitySystemComponent || !EffectClass)
	{
		return;
	}

	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	const FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(EffectClass, Level, EffectContext);

	if (SpecHandle.IsValid())
	{
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}

float AZomPlayerCharacterBase::GetHealth() const
{
	const UZomAttributeSetBase* AttributeSet = AbilitySystemComponent ? AbilitySystemComponent->GetSet<UZomAttributeSetBase>() : nullptr;
	return AttributeSet ? AttributeSet->GetHealth() : 0.f;
}

float AZomPlayerCharacterBase::GetMaxHealth() const
{
	const UZomAttributeSetBase* AttributeSet = AbilitySystemComponent ? AbilitySystemComponent->GetSet<UZomAttributeSetBase>() : nullptr;
	return AttributeSet ? AttributeSet->GetMaxHealth() : 0.f;
}

// Empty at this level; each subclass overrides it for actor-level death consequences (Section 4.6).
void AZomPlayerCharacterBase::HandleDeath()
{
}

void AZomPlayerCharacterBase::OnHealthAttributeChanged(const FOnAttributeChangeData& Data)
{
	if (Data.NewValue <= 0.f)
	{
		HandleDeath();
	}
}
