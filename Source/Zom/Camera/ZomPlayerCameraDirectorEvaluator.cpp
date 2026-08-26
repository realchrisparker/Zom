// Fill out your copyright notice in the Description page of Project Settings.

#include "Zom/Camera/ZomPlayerCameraDirectorEvaluator.h"
#include "Zom/Characters/ZomPlayerCharacter.h"
#include "Zom/Misc/ZomLogChannels.h"
#include "Core/CameraRigAsset.h"

void UZomPlayerCameraDirectorEvaluator::EvaluateCameraRigForCurrentTag()
{
	// Lazily resolve the AZomPlayerCharacter owner once and cache it for future ticks, instead of re-running
	// FindEvaluationContextOwnerActor/Cast every frame for an owner that never changes.
	AZomPlayerCharacter* PlayerCharacter = CachedPlayerCharacter.Get();
	if (!PlayerCharacter)
	{
		PlayerCharacter = Cast<AZomPlayerCharacter>(FindEvaluationContextOwnerActor(AZomPlayerCharacter::StaticClass()));
		CachedPlayerCharacter = PlayerCharacter;
	}

	if (!PlayerCharacter)
	{
		return;
	}

	// Look up the camera rig for the current CurrentCamera, falling back to DefaultCameraRig if none is found.
	const TObjectPtr<UCameraRigAsset>* FoundRig = CameraRigsByTag.Find(PlayerCharacter->GetCurrentCamera());
	UCameraRigAsset* RigToActivate = FoundRig ? FoundRig->Get() : DefaultCameraRig.Get();

	// Activate the found rig (or DefaultCameraRig) if one was found, otherwise log a warning.
	if (RigToActivate)
	{
		ActivateCameraRig(RigToActivate);
	}
	else
	{
		UE_LOG(LogZomCharacter, Warning, TEXT("%s: no camera rig for tag '%s' and no DefaultCameraRig set."), *GetClass()->GetName(), *PlayerCharacter->GetCurrentCamera().ToString());
	}
}

UWorld* UZomPlayerCameraDirectorEvaluator::GetWorld() const
{
	if (HasAllFlags(RF_ClassDefaultObject))
	{
		return nullptr;
	}

	for (UObject* Outer = GetOuter(); Outer; Outer = Outer->GetOuter())
	{
		if (UWorld* World = Outer->GetWorld())
		{
			return World;
		}
	}

	return nullptr;
}
