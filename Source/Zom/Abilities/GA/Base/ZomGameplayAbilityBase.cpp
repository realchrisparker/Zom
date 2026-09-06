// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Abilities/GA/Base/ZomGameplayAbilityBase.h"
#include "Zom/Misc/ZomGameplayTags.h"
#include "Zom/Characters/Base/ZomCharacterBase.h"
#include "MotionCombatSystem/Components/MCS_CombatCoreComponent.h"
#include "MotionCombatSystem/Components/MCS_CombatHitboxComponent.h"
#include "MotionCombatSystem/Structs/MCS_AttackEntry.h"
#include "MotionCombatSystem/AnimNotifies/AnimNotify_AttackStart.h"
#include "MotionCombatSystem/AnimNotifies/AnimNotify_AttackEnd.h"
#include "MotionCombatSystem/AnimNotifies/AnimNotify_CameraControl.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"


UZomGameplayAbilityBase::UZomGameplayAbilityBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	ActivationBlockedTags.AddTag(TAG_Zom_Status_Staggered.GetTag());
}

TObjectPtr<AZomCharacterBase> UZomGameplayAbilityBase::GetOwningCharacter() const
{
	return Cast<AZomCharacterBase>(GetAvatarActorFromActorInfo());
}

TObjectPtr<UMCS_CombatCoreComponent> UZomGameplayAbilityBase::GetOwningCharacterCombatCoreComponent() const
{
	const AZomCharacterBase* OwningCharacter = GetOwningCharacter();
	return OwningCharacter ? OwningCharacter->GetCombatCoreComponent() : nullptr;
}

FMCS_AttackEntry UZomGameplayAbilityBase::GetCurrentAttackEntry() const
{
	const AZomCharacterBase* OwningCharacter = GetOwningCharacter();
	const UMCS_CombatCoreComponent* CombatCore = GetOwningCharacterCombatCoreComponent();
	return CombatCore ? CombatCore->GetCurrentAttack() : FMCS_AttackEntry();
}

void UZomGameplayAbilityBase::NotifyAttackMontageEnded() const
{
	if (const AZomCharacterBase* OwningCharacter = GetOwningCharacter())
	{
		if (UMCS_CombatCoreComponent* CombatCore = OwningCharacter->GetCombatCoreComponent())
		{
			CombatCore->OnAttackEnd.Broadcast();
		}
	}
}

// Always unbinds - regardless of which path ended the ability - so a subclass that called BindMontageNotifies
// never has to remember to clean up itself.
void UZomGameplayAbilityBase::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	UnbindMontageNotifies();
}

// Mirrors UMCS_CombatCoreComponent::BindNotifiesForMontage.
void UZomGameplayAbilityBase::BindMontageNotifies(UAnimMontage* Montage)
{
	UnbindMontageNotifies();

	if (!Montage)
	{
		return;
	}

	BoundMontage = Montage;

	for (const FAnimNotifyEvent& Event : Montage->Notifies)
	{
		if (UAnimNotify_AttackStart* StartNotify = Cast<UAnimNotify_AttackStart>(Event.Notify))
		{
			StartNotify->OnAttackStart.AddDynamic(this, &UZomGameplayAbilityBase::HandleAttackStartNotify);
			BoundAttackStartNotifies.Add(StartNotify);
		}
		else if (UAnimNotify_AttackEnd* EndNotify = Cast<UAnimNotify_AttackEnd>(Event.Notify))
		{
			EndNotify->OnAttackEnd.AddDynamic(this, &UZomGameplayAbilityBase::HandleAttackEndNotify);
			BoundAttackEndNotifies.Add(EndNotify);
		}
		else if (UAnimNotify_CameraControl* CameraNotify = Cast<UAnimNotify_CameraControl>(Event.Notify))
		{
			CameraNotify->OnCameraControl.AddDynamic(this, &UZomGameplayAbilityBase::HandleCameraControlNotify);
			BoundCameraControlNotifies.Add(CameraNotify);
		}

		if (UAnimNotifyState_MCSWindow* WindowNotify = Cast<UAnimNotifyState_MCSWindow>(Event.NotifyStateClass))
		{
			WindowNotify->OnNotifyBegin.AddDynamic(this, &UZomGameplayAbilityBase::HandleMCSNotifyBegin);
			WindowNotify->OnNotifyEnd.AddDynamic(this, &UZomGameplayAbilityBase::HandleMCSNotifyEnd);
			BoundMCSNotifies.Add(WindowNotify);
		}
	}
}

void UZomGameplayAbilityBase::UnbindMontageNotifies()
{
	for (UAnimNotify_AttackStart* Notify : BoundAttackStartNotifies)
	{
		if (Notify)
		{
			Notify->OnAttackStart.RemoveDynamic(this, &UZomGameplayAbilityBase::HandleAttackStartNotify);
		}
	}
	BoundAttackStartNotifies.Reset();

	for (UAnimNotify_AttackEnd* Notify : BoundAttackEndNotifies)
	{
		if (Notify)
		{
			Notify->OnAttackEnd.RemoveDynamic(this, &UZomGameplayAbilityBase::HandleAttackEndNotify);
		}
	}
	BoundAttackEndNotifies.Reset();

	for (UAnimNotify_CameraControl* Notify : BoundCameraControlNotifies)
	{
		if (Notify)
		{
			Notify->OnCameraControl.RemoveDynamic(this, &UZomGameplayAbilityBase::HandleCameraControlNotify);
		}
	}
	BoundCameraControlNotifies.Reset();

	for (UAnimNotifyState_MCSWindow* Notify : BoundMCSNotifies)
	{
		if (Notify)
		{
			Notify->OnNotifyBegin.RemoveDynamic(this, &UZomGameplayAbilityBase::HandleMCSNotifyBegin);
			Notify->OnNotifyEnd.RemoveDynamic(this, &UZomGameplayAbilityBase::HandleMCSNotifyEnd);
		}
	}
	BoundMCSNotifies.Reset();

	BoundMontage.Reset();
}

// Guards every handler below against firing for another actor's playback of the same shared montage asset -
// notify UObjects live on the montage, not per-character, so every character currently playing any montage
// containing one of these notify types would otherwise trigger this ability instance's handlers too. Matches
// by identity (does my AnimInstance have/had an instance of this exact montage) rather than a liveness check
// like Montage_IsPlaying/GetCurrentActiveMontage, both of which flip false the instant the montage starts
// stopping - too strict for a notify (e.g. AttackEnd) placed right at the montage's tail/blend-out.
bool UZomGameplayAbilityBase::IsOwningCharacterPlayingBoundMontage() const
{
	UAnimMontage* Montage = BoundMontage.Get();
	if (!Montage)
	{
		return false;
	}

	const AZomCharacterBase* OwningCharacter = GetOwningCharacter();
	const USkeletalMeshComponent* Mesh = OwningCharacter ? OwningCharacter->GetMesh() : nullptr;
	const UAnimInstance* AnimInstance = Mesh ? Mesh->GetAnimInstance() : nullptr;
	return AnimInstance && AnimInstance->GetInstanceForMontage(Montage) != nullptr;
}

void UZomGameplayAbilityBase::HandleAttackStartNotify(UAnimNotify_AttackStart* NotifyInstance)
{
	if (NotifyInstance && IsOwningCharacterPlayingBoundMontage())
	{
		// Get owning character's combat core component to broadcast the attack start event.
		UMCS_CombatCoreComponent* CombatCore = GetOwningCharacterCombatCoreComponent();
		CombatCore->OnAttackStart.Broadcast();
	}
}

void UZomGameplayAbilityBase::HandleAttackEndNotify(UAnimNotify_AttackEnd* NotifyInstance)
{
	if (NotifyInstance && IsOwningCharacterPlayingBoundMontage())
	{
		// Get owning character's combat core component to broadcast the attack end event.
		UMCS_CombatCoreComponent* CombatCore = GetOwningCharacterCombatCoreComponent();
		CombatCore->OnAttackEnd.Broadcast();
	}
}

void UZomGameplayAbilityBase::HandleCameraControlNotify(UAnimNotify_CameraControl* NotifyInstance, FGameplayTag CameraTag)
{
	if (NotifyInstance && IsOwningCharacterPlayingBoundMontage())
	{
		// Get owning character's combat core component to broadcast the camera control event.
		UMCS_CombatCoreComponent* CombatCore = GetOwningCharacterCombatCoreComponent();
		CombatCore->OnCameraControl.Broadcast(CameraTag);
	}
}

void UZomGameplayAbilityBase::HandleMCSNotifyBegin(EMCS_AnimEventType EventType, UAnimNotifyState_MCSWindow* NotifyInstance)
{
	if (NotifyInstance && IsOwningCharacterPlayingBoundMontage())
	{
		// Get owning character and combat core component for handling the MCS notify begin event.
		AZomCharacterBase* OwningCharacter = GetOwningCharacter();
		UMCS_CombatCoreComponent* CombatCore = OwningCharacter ? OwningCharacter->GetCombatCoreComponent() : nullptr;
		if (!CombatCore)
		{
			return;
		}

		switch (EventType)
		{
		case EMCS_AnimEventType::HitboxWindow:
			if (UMCS_CombatHitboxComponent* HitboxComponent = OwningCharacter->GetCombatHitboxComponent())
			{
				HitboxComponent->ResetAlreadyHit();
				HitboxComponent->StartMultiHitDetection(CombatCore->GetCurrentAttack(), NotifyInstance->Hitboxes);
			}
			break;

		case EMCS_AnimEventType::ComboWindow:
			CombatCore->OnComboWindowBegin.Broadcast();
			break;

		case EMCS_AnimEventType::ParryWindow:
			CombatCore->OnParryWindowBegin.Broadcast(OwningCharacter);
			break;

		case EMCS_AnimEventType::DefenseWindow:
			CombatCore->OnDefenseWindowBegin.Broadcast(OwningCharacter);
			break;

		default:
			break;
		}
	}
}

void UZomGameplayAbilityBase::HandleMCSNotifyEnd(EMCS_AnimEventType EventType, UAnimNotifyState_MCSWindow* NotifyInstance)
{
	if (NotifyInstance && IsOwningCharacterPlayingBoundMontage())
	{
		// Get owning character and combat core component for handling the MCS notify end event.
		AZomCharacterBase* OwningCharacter = GetOwningCharacter();
		UMCS_CombatCoreComponent* CombatCore = OwningCharacter ? OwningCharacter->GetCombatCoreComponent() : nullptr;
		if (!CombatCore)
		{
			return;
		}

		switch (EventType)
		{
		case EMCS_AnimEventType::HitboxWindow:
			if (UMCS_CombatHitboxComponent* HitboxComponent = OwningCharacter->GetCombatHitboxComponent())
			{
				HitboxComponent->StopHitDetection();
			}
			break;

		case EMCS_AnimEventType::ComboWindow:
			CombatCore->OnComboWindowEnd.Broadcast();
			break;

		case EMCS_AnimEventType::ParryWindow:
			CombatCore->OnParryWindowEnd.Broadcast(OwningCharacter);
			break;

		case EMCS_AnimEventType::DefenseWindow:
			CombatCore->OnDefenseWindowEnd.Broadcast(OwningCharacter);
			break;

		default:
			break;
		}
	}
}
