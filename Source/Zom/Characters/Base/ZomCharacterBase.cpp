// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Characters/Base/ZomCharacterBase.h"
#include "Zom/Characters/Components/ZomCharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Zom/Abilities/AttributeSets/ZomAttributeSetBase.h"
#include "Zom/Abilities/GA/Base/ZomGameplayAbilityBase.h"
#include "Zom/Abilities/Effects/Base/ZomGameplayEffectBase.h"
#include "Zom/Abilities/Effects/ZomGE_Damage.h"
#include "Zom/Misc/ZomGameplayTags.h"
#include "Zom/Misc/ZomLogChannels.h"
#include "MotionCombatSystem/Components/MCS_CombatCoreComponent.h"
#include "MotionCombatSystem/Components/MCS_CombatHitboxComponent.h"
#include "MotionCombatSystem/Components/MCS_CombatHitReactionComponent.h"
#include "MotionCombatSystem/Components/MCS_CombatDefenseComponent.h"
#include "MotionCombatSystem/Structs/MCS_AttackEntry.h"


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

	CombatState = ECombatState::None;
}

// Called every frame
void AZomCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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

		if (CombatCoreComponent)
		{
			// RemoveDynamic before AddDynamic for the same re-entrancy reason as the health delegate above.
			CombatCoreComponent->OnAttackResolved.RemoveDynamic(this, &AZomCharacterBase::HandleAttackResolved);
			CombatCoreComponent->OnAttackResolved.AddDynamic(this, &AZomCharacterBase::HandleAttackResolved);
		}

		if (CombatHitboxComponent)
		{
			CombatHitboxComponent->OnHitboxHit.RemoveDynamic(this, &AZomCharacterBase::HandleHitboxHit);
			CombatHitboxComponent->OnHitboxHit.AddDynamic(this, &AZomCharacterBase::HandleHitboxHit);
		}

		if (CombatDefenseComponent)
		{
			CombatDefenseComponent->OnDefenseResolved.RemoveDynamic(this, &AZomCharacterBase::HandleDefenseResolved);
			CombatDefenseComponent->OnDefenseResolved.AddDynamic(this, &AZomCharacterBase::HandleDefenseResolved);
		}

		if (CombatHitReactionComponent)
		{
			CombatHitReactionComponent->OnHitReactionResolved.RemoveDynamic(this, &AZomCharacterBase::HandleHitReactionResolved);
			CombatHitReactionComponent->OnHitReactionResolved.AddDynamic(this, &AZomCharacterBase::HandleHitReactionResolved);
		}

		GrantDefaultAbilitiesAndEffects();

		OnAbilitySystemInitialized();
	}
}

// Grants every class in DefaultAbilities and applies every class in DefaultGameplayEffects. Both AddAbility and
// AddEffect are server-authoritative-only and idempotent, so this is safe to call on clients and safe to call
// again on a second InitializeAbilitySystem pass for the same ASC.
void AZomCharacterBase::GrantDefaultAbilitiesAndEffects()
{
	for (const TSubclassOf<UZomGameplayAbilityBase>& AbilityClass : DefaultAbilities)
	{
		AddAbility(AbilityClass);
	}

	for (const TSubclassOf<UZomGameplayEffectBase>& EffectClass : DefaultGameplayEffects)
	{
		AddEffect(EffectClass);
	}
}

// Grants a single ability class if not already tracked as granted. Server-only; idempotent.
bool AZomCharacterBase::AddAbility(TSubclassOf<UZomGameplayAbilityBase> AbilityClass)
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
	if (const UZomGameplayAbilityBase* NewAbilityCDO = AbilityClass.GetDefaultObject())
	{
		const FGameplayTagContainer& NewTags = NewAbilityCDO->GetAssetTags();
		if (NewTags.Num() > 0)
		{
			for (const TPair<TSubclassOf<UZomGameplayAbilityBase>, FGameplayAbilitySpecHandle>& Pair : GrantedAbilityHandles)
			{
				const UZomGameplayAbilityBase* ExistingCDO = Pair.Key ? Pair.Key.GetDefaultObject() : nullptr;
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
bool AZomCharacterBase::RemoveAbility(TSubclassOf<UZomGameplayAbilityBase> AbilityClass)
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
bool AZomCharacterBase::AddEffect(TSubclassOf<UZomGameplayEffectBase> EffectClass, float Level)
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
bool AZomCharacterBase::RemoveEffect(TSubclassOf<UZomGameplayEffectBase> EffectClass)
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
	//TODO: Implement actor-level death consequences, such as playing a death animation, disabling input, and notifying the game mode.
}

void AZomCharacterBase::OnHealthAttributeChanged(const FOnAttributeChangeData& Data)
{
	if (Data.NewValue <= 0.f)
	{
		HandleDeath();
	}
}

void AZomCharacterBase::HandleAttackResolved(const FMCS_AttackEntry& ResolvedAttack)
{
	// Per OnAttackResolved's contract: this only fires on the GAS path (AttackTag valid) or the
	// Blueprint-only path (bAutoPlayMontage false, AttackTag empty). Nothing to activate in the latter case.
	if (!ResolvedAttack.AttackTag.IsValid() || !AbilitySystemComponent)
	{
		return;
	}

	// InstancedPerActor abilities silently return false here (only a Verbose engine log, easy to miss) if the
	// same ability is already active - e.g. a combo continuation re-resolving an AttackTag while the ability
	// from the swing that opened the combo window hasn't ended yet. UZomGA_LightAttack/HeavyAttack's mirrored
	// CancelAbilitiesWithTag handles this by cancelling the orphaned ability before the new one activates.
	AbilitySystemComponent->TryActivateAbilitiesByTag(FGameplayTagContainer(ResolvedAttack.AttackTag));
}

void AZomCharacterBase::HandleDefenseResolved(const FMCS_DefenseEntry& ResolvedDefense)
{
	// Per OnDefenseResolved's contract: this only fires on the GAS path (DefenseTag valid) or the
	// Blueprint-only path (bAutoPlayMontage false, DefenseTag empty). Nothing to activate in the latter case.
	if (!ResolvedDefense.DefenseTag.IsValid() || !AbilitySystemComponent)
	{
		return;
	}

	// InstancedPerActor abilities silently return false here (only a Verbose engine log, easy to miss) if the
	// same ability is already active - e.g. a combo continuation re-resolving a DefenseTag while the ability
	// from the previous defense hasn't ended yet. UZomGA_Block/Parry's mirrored
	// CancelAbilitiesWithTag handles this by cancelling the orphaned ability before the new one activates.
	AbilitySystemComponent->TryActivateAbilitiesByTag(FGameplayTagContainer(ResolvedDefense.DefenseTag));
}

void AZomCharacterBase::HandleHitReactionResolved(const FMCS_HitReaction& ResolvedReaction)
{
	// Per OnHitReactionResolved's contract: this fires on the hand-off path (HitReactionTag valid) or the
	// Blueprint-only path (bAutoPlayHitReactionMontage false, HitReactionTag empty). Nothing to activate in the latter.
	if (!ResolvedReaction.HitReactionTag.IsValid() || !AbilitySystemComponent)
	{
		return;
	}

	// Must start the montage synchronously: the component arms its death/explicit-reaction locks right after this
	// returns, and only if the montage is already playing. UZomGA_HitReaction retriggers, so back-to-back hits restart it.
	AbilitySystemComponent->TryActivateAbilitiesByTag(FGameplayTagContainer(ResolvedReaction.HitReactionTag));
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

// -------------
// IMCS_CombatCharacterInterface
// -------------

// Whether this actor can currently be targeted (true = valid target).
bool AZomCharacterBase::CanBeTargeted_Implementation() const
{
	return true;
}

// IMCS_CombatCharacterInterface: routes incoming damage through UZomGE_Damage the same way every other GE is
// applied (see ApplyGameplayEffectToSelf) - Damage's SetByCaller magnitude carries the hit-specific amount,
// while the Damage->Health conversion and clamping stays centralized in UZomAttributeSetBase::PostGameplayEffectExecute.
// Runs on the struck actor (this), which is why EffectContext's source object is set to itself rather than the
// attacker - HandleHitboxHit supplies Attacker (other callers may pass null), but it isn't written into the context yet. Not a
// problem for the Damage attribute write itself (SetByCaller doesn't care who's listed as the causer), just a
// known gap for anything that later wants "who dealt this hit" (kill credit, XP, combat log) off the context.
bool AZomCharacterBase::TakeCombatDamage_Implementation(float Damage, const FHitResult& Hit, const FMCS_AttackEntry& AttackEntry, AActor* Attacker) const
{
	if (!AbilitySystemComponent || Damage <= 0.f)
	{
		return false;
	}

	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(const_cast<AZomCharacterBase*>(this));
	EffectContext.AddHitResult(Hit);

	const FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(UZomGE_Damage::StaticClass(), 1.f, EffectContext);
	if (!SpecHandle.IsValid())
	{
		return false;
	}

	SpecHandle.Data->SetSetByCallerMagnitude(TAG_Zom_SetByCaller_Magnitude.GetTag(), Damage);

	const FActiveGameplayEffectHandle ActiveHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	if (!ActiveHandle.WasSuccessfullyApplied())
	{
		return false;
	}

	if (CombatHitReactionComponent)
	{
		// The attack authors the severity, but death is decided by Health. The Damage->Health conversion has already
		// run synchronously, so a lethal hit is upgraded to Death - the component then locks it in and fires
		// OnDeathReactionFinished. Attacker may be null (see above); hit direction still resolves from Hit's impact point.
		const EPGAS_HitSeverity Severity = GetHealth() <= 0.f ? EPGAS_HitSeverity::Death : AttackEntry.HitSeverity;
		CombatHitReactionComponent->PerformHitReaction(Hit, const_cast<AZomCharacterBase*>(this), Severity, Attacker);
	}

	return true;
}

// Fires on the attacker when their own CombatHitboxComponent sweep lands on someone else (self-hits are already
// filtered out by the component itself). AttackEntry.Damage is the base/minimum damage for the resolved attack -
// future modifier passes (weapon upgrades, difficulty scaling, crits, etc.) should adjust it here, before it's
// handed to TakeCombatDamage, since that runs on the struck actor and only receives this attacker as a reference.
void AZomCharacterBase::HandleHitboxHit(AActor* HitActor, const FHitResult& HitResult, FMCS_AttackEntry AttackEntry)
{
	// Early out if there's no valid hit actor or it doesn't implement the combat interface.
	if (!HitActor || !HitActor->Implements<UMCS_CombatCharacterInterface>())
	{
		return;
	}

	// Give the struck actor's own CombatDefenseComponent a chance to negate this hit before any damage is
	// calculated.
	//
	// Parry is checked via ConsumeSuccessfulParry(this), NOT TryParry(): parry is a skill-timed player input
	// now (see AZomPlayerController::Input_Parry), so success/fail is decided the instant the player presses
	// the button, not whenever a hit happens to land afterward. This just asks "did the player already win
	// the parry roll against this specific attacker" and consumes that result so it can't be reused.
	//
	// Defense/Block is still checked reactively here, gated behind its bool flag first: TryDefense()
	// unconditionally broadcasts OnDefenseFail when called, and most hits land on an actor who isn't
	// blocking at all, so calling it unconditionally would spam that event (and the plugin's own warning
	// logs) for every ordinary unguarded hit. bIsInDefenseWindow is the self-driven "currently guarding"
	// state (set only while this actor's own montage is playing a DefenseWindow notify) - there's no
	// separate "block" input yet, so it stays automatic.
	if (UMCS_CombatDefenseComponent* DefenderDefense = HitActor->FindComponentByClass<UMCS_CombatDefenseComponent>())
	{
		if (DefenderDefense->ConsumeSuccessfulParry(this))
		{
			return; // Parried - attack negated, no damage applied.
		}

		if (DefenderDefense->bIsInDefenseWindow && DefenderDefense->TryDefense(this))
		{
			// A successful block doesn't necessarily zero the damage out - GetCurrentDefense().
			// DamageMitigationPercent (authored per-row, defaults to 1.0/full negation) decides how much
			// gets through as "chip damage." AttackEntry is a by-value parameter here (a per-hit copy),
			// so mutating it doesn't touch the shared FMCS_AttackEntry data the attack was resolved from.
			const float MitigationPercent = FMath::Clamp(DefenderDefense->GetCurrentDefense().DamageMitigationPercent, 0.f, 1.f);
			AttackEntry.Damage *= (1.f - MitigationPercent);

			if (AttackEntry.Damage <= KINDA_SMALL_NUMBER)
			{
				return; // Fully blocked - no damage applied.
			}
			// Otherwise fall through to TakeCombatDamage below with the reduced Damage.
		}
	}

	// Route the hit to the struck actor's TakeCombatDamage implementation. This runs on the attacker, so pass self.
	IMCS_CombatCharacterInterface::Execute_TakeCombatDamage(HitActor, AttackEntry.Damage, HitResult, AttackEntry, this);
}