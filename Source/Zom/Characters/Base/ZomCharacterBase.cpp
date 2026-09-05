// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Characters/Base/ZomCharacterBase.h"
#include "Zom/Characters/Components/ZomCharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Zom/Abilities/AttributeSets/ZomAttributeSetBase.h"
#include "Zom/Abilities/ZomGameplayAbility.h"
#include "Zom/Abilities/ZomGameplayEffect.h"
#include "Zom/Misc/ZomLogChannels.h"
#include "MotionCombatSystem/Components/MCS_CombatCoreComponent.h"
#include "MotionCombatSystem/Components/MCS_CombatHitboxComponent.h"
#include "MotionCombatSystem/Components/MCS_CombatHitReactionComponent.h"
#include "MotionCombatSystem/Components/MCS_CombatDefenseComponent.h"


// Sets default values
AZomCharacterBase::AZomCharacterBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// State Tree and GAS event-driven logic cover behavior without polling; leaf classes opt back in if they
	// have a proven per-frame need (e.g. AZomPlayerCharacter, for its locomotion/anim pipeline).
	PrimaryActorTick.bCanEverTick = false;

	// Motion Combat System components.

	CombatCoreComponent = CreateDefaultSubobject<UMCS_CombatCoreComponent>(TEXT("CombatCoreComponent"));
	CombatHitboxComponent = CreateDefaultSubobject<UMCS_CombatHitboxComponent>(TEXT("CombatHitboxComponent"));
	CombatHitReactionComponent = CreateDefaultSubobject<UMCS_CombatHitReactionComponent>(TEXT("CombatHitReactionComponent"));
	CombatDefenseComponent = CreateDefaultSubobject<UMCS_CombatDefenseComponent>(TEXT("CombatDefenseComponent"));
}

// Called when the game starts or when spawned
void AZomCharacterBase::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AZomCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AZomCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

// Called when the character starts crouching; keeps Stance in sync
void AZomCharacterBase::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	Stance = EStance::Crouch;
}

// Called when the character stops crouching; keeps Stance in sync
void AZomCharacterBase::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	Stance = EStance::Stand;
}

// Returns the cached AbilitySystemComponent pointer. Never re-resolves it; InitializeAbilitySystem populates it.
UAbilitySystemComponent* AZomCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

// Caches the AbilitySystemComponent resolved off OwnerActor and calls InitAbilityActorInfo(OwnerActor, AvatarActor).
void AZomCharacterBase::InitializeAbilitySystem(AActor* InOwnerActor, AActor* InAvatarActor)
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
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UZomAttributeSetBase::GetHealthAttribute()).AddUObject(this, &AZomCharacterBase::OnHealthAttributeChanged);

		OnAbilitySystemInitialized();
	}
}

// Grants a single ability class if not already tracked as granted. Server-only; idempotent.
bool AZomCharacterBase::AddAbility(TSubclassOf<UZomGameplayAbility> AbilityClass)
{
	if (!AbilityClass || !AbilitySystemComponent || !AbilitySystemComponent->IsOwnerActorAuthoritative())
	{
		return false;
	}

	if (GrantedAbilityHandles.Contains(AbilityClass))
	{
		return true; // Already granted - no-op, not a duplicate spec.
	}

	// Debug-only guardrail: TryActivateAbilitiesByTag activates ALL granted specs matching a query tag, so if
	// two simultaneously-granted abilities share an AssetTag (e.g. an uncleaned combat-state weapon swap left
	// both the old and new weapon's attack ability granted), both would activate/play their montage at once.
#if !UE_BUILD_SHIPPING
	if (const UZomGameplayAbility* NewAbilityCDO = AbilityClass.GetDefaultObject())
	{
		const FGameplayTagContainer& NewTags = NewAbilityCDO->GetAssetTags();
		if (NewTags.Num() > 0)
		{
			for (const TPair<TSubclassOf<UZomGameplayAbility>, FGameplayAbilitySpecHandle>& Pair : GrantedAbilityHandles)
			{
				const UZomGameplayAbility* ExistingCDO = Pair.Key ? Pair.Key.GetDefaultObject() : nullptr;
				if (ExistingCDO && ExistingCDO->GetAssetTags().HasAny(NewTags))
				{
					UE_LOG(LogZom, Warning, TEXT("AddAbility: %s shares AssetTag(s) with already-granted %s on %s - TryActivateAbilitiesByTag will activate both. Ensure the old ability is RemoveAbility'd before granting a replacement with an overlapping tag."),
						*AbilityClass->GetName(), *Pair.Key->GetName(), *GetNameSafe(this));
				}
			}
		}
	}
#endif

	const FGameplayAbilitySpecHandle NewHandle = AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1, INDEX_NONE, this));
	GrantedAbilityHandles.Add(AbilityClass, NewHandle);
	return true;
}

// Revokes a single ability class previously granted via AddAbility. Idempotent.
bool AZomCharacterBase::RemoveAbility(TSubclassOf<UZomGameplayAbility> AbilityClass)
{
	if (!AbilityClass || !AbilitySystemComponent || !AbilitySystemComponent->IsOwnerActorAuthoritative())
	{
		return false;
	}

	FGameplayAbilitySpecHandle Handle;
	if (!GrantedAbilityHandles.RemoveAndCopyValue(AbilityClass, Handle))
	{
		return false; // Wasn't granted via this API.
	}

	AbilitySystemComponent->ClearAbility(Handle);
	return true;
}

// Applies a single effect class to self if not already tracked as active via this API. Idempotent w.r.t. this
// API's own bookkeeping only.
bool AZomCharacterBase::AddEffect(TSubclassOf<UZomGameplayEffect> EffectClass, float Level)
{
	if (!EffectClass || !AbilitySystemComponent || !AbilitySystemComponent->IsOwnerActorAuthoritative())
	{
		return false;
	}

	if (ActiveEffectHandles.Contains(EffectClass))
	{
		return true; // Already applied via this API - no-op, not a re-stack.
	}

	// Track it even if the handle comes back invalid (Instant effects never produce an active handle) - the
	// map entry itself is what makes a second AddEffect(EffectClass) call a no-op instead of re-applying.
	ActiveEffectHandles.Add(EffectClass, ApplyGameplayEffectToSelf(EffectClass, Level));
	return true;
}

// Removes an effect previously applied via AddEffect. No-op/false if not tracked or the tracked handle is
// invalid (e.g. it was an Instant effect - nothing ongoing to remove).
bool AZomCharacterBase::RemoveEffect(TSubclassOf<UZomGameplayEffect> EffectClass)
{
	if (!EffectClass || !AbilitySystemComponent || !AbilitySystemComponent->IsOwnerActorAuthoritative())
	{
		return false;
	}

	FActiveGameplayEffectHandle Handle;
	if (!ActiveEffectHandles.RemoveAndCopyValue(EffectClass, Handle))
	{
		return false;
	}

	if (Handle.IsValid())
	{
		AbilitySystemComponent->RemoveActiveGameplayEffect(Handle);
	}
	return true;
}

// Small wrapper around the MakeOutgoingSpec/ApplyGameplayEffectSpecToSelf boilerplate for self-applied effects.
FActiveGameplayEffectHandle AZomCharacterBase::ApplyGameplayEffectToSelf(TSubclassOf<UGameplayEffect> EffectClass, float Level)
{
	if (!AbilitySystemComponent || !EffectClass)
	{
		return FActiveGameplayEffectHandle();
	}

	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	const FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(EffectClass, Level, EffectContext);

	if (SpecHandle.IsValid())
	{
		return AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}

	return FActiveGameplayEffectHandle();
}

float AZomCharacterBase::GetHealth() const
{
	const UZomAttributeSetBase* AttributeSet = AbilitySystemComponent ? AbilitySystemComponent->GetSet<UZomAttributeSetBase>() : nullptr;
	return AttributeSet ? AttributeSet->GetHealth() : 0.f;
}

float AZomCharacterBase::GetMaxHealth() const
{
	const UZomAttributeSetBase* AttributeSet = AbilitySystemComponent ? AbilitySystemComponent->GetSet<UZomAttributeSetBase>() : nullptr;
	return AttributeSet ? AttributeSet->GetMaxHealth() : 0.f;
}

// Empty at this level; each subclass overrides it for actor-level death consequences (Section 4.6).
void AZomCharacterBase::HandleDeath()
{
}

void AZomCharacterBase::OnHealthAttributeChanged(const FOnAttributeChangeData& Data)
{
	if (Data.NewValue <= 0.f)
	{
		HandleDeath();
	}
}

// Returns the current attack situation, which is used to determine which attacks are valid for the character.
FMCS_AttackSituation AZomCharacterBase::GetCurrentAttackSituation() const
{
	FMCS_AttackSituation AttackSituation;

	if (const UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		AttackSituation.bIsGrounded = MovementComponent->IsMovingOnGround();
		AttackSituation.bIsInAir = MovementComponent->IsFalling();
	}

	AttackSituation.bIsRunning = Gait == EGait::Run || Gait == EGait::Sprint;
	AttackSituation.bIsCrouching = Stance == EStance::Crouch;

	if (CombatDefenseComponent)
	{
		AttackSituation.bIsBlocking = CombatDefenseComponent->bIsInDefenseWindow;
		AttackSituation.bIsParrying = CombatDefenseComponent->bIsInParryWindow;
	}

	AttackSituation.Speed = GetVelocity().Size();
	AttackSituation.Altitude = GetActorLocation().Z;
	AttackSituation.Health = GetHealth();

	// Stamina lives on UZomPlayerAttributeSet, not the base UZomAttributeSetBase this class reads from,
	// so it's left at the struct default here; AZomPlayerCharacter should override to fill it in.

	return AttackSituation;
}