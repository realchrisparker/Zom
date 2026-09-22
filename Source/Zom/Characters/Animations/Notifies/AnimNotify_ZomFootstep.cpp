// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Characters/Animations/Notifies/AnimNotify_ZomFootstep.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Zom/Characters/Components/ZomCharacterNoiseComponent.h"
#include "Zom/Misc/ZomLogChannels.h"


// Fires a footstep on the owning character's noise component.
void UAnimNotify_ZomFootstep::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp)
	{
		return;
	}

	// Notifies also fire in the animation asset editor's preview viewport and in Sequencer, where there is
	// no gameplay, no perception system and no noise component. Without this the editor would log warnings
	// and, worse, a preview actor could report phantom noise into a PIE session sharing the process.
	const UWorld* World = MeshComp->GetWorld();
	if (!World || !World->IsGameWorld())
	{
		return;
	}

	const AActor* Owner = MeshComp->GetOwner();
	UZomCharacterNoiseComponent* NoiseComponent = Owner ? Owner->FindComponentByClass<UZomCharacterNoiseComponent>() : nullptr;
	if (!NoiseComponent)
	{
		// Expected for every character without the component (currently everything but the player), so this
		// is a silent no-op rather than a warning.
		return;
	}

	// A notify left in an animation while the component is set to pace itself would double up with the
	// stride timer. The component owns that decision, so honour it here rather than letting both run.
	if (NoiseComponent->FootstepTrigger != EZomFootstepTrigger::AnimNotify)
	{
		return;
	}

	if (bDebug)
	{
		UE_LOG(LogZomCharacter, Log, TEXT("UAnimNotify_ZomFootstep: %s foot on %s"),
			Foot == EZomFoot::Left ? TEXT("left") : TEXT("right"), *GetNameSafe(Owner));
	}

	NoiseComponent->NotifyFootstep(Foot);
}

// Name shown on the notify track in the animation editor.
FString UAnimNotify_ZomFootstep::GetNotifyName_Implementation() const
{
	switch (Foot)
	{
	case EZomFoot::Left:
		return TEXT("Footstep (L)");

	case EZomFoot::Right:
		return TEXT("Footstep (R)");

	default:
		return TEXT("Footstep");
	}
}
