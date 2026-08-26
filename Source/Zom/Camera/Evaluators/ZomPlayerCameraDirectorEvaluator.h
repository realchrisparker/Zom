// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Directors/BlueprintCameraDirector.h"
#include "GameplayTagContainer.h"
#include "ZomPlayerCameraDirectorEvaluator.generated.h"

class AZomPlayerCharacter;
class UCameraRigAsset;

/**
 * Native base for CDE_Player, the Blueprint camera director evaluator asset driving AZomPlayerCharacter's
 * UGameplayCameraComponent. Reads AZomPlayerCharacter::CameraStateTag and activates the matching entry in
 * CameraRigsByTag. CDE_Player's RunCameraDirector graph only needs to call EvaluateCameraRigForCurrentTag -
 * RunCameraDirector/ActivateCameraDirector/DeactivateCameraDirector stay Blueprint-only (they're
 * BlueprintImplementableEvent on UBlueprintCameraDirectorEvaluator, not overridable from C++).
 */
UCLASS(Abstract, Blueprintable)
class ZOM_API UZomPlayerCameraDirectorEvaluator : public UBlueprintCameraDirectorEvaluator
{
	GENERATED_BODY()

public:

	/** Looks up AZomPlayerCharacter::CameraStateTag in CameraRigsByTag and activates the matching rig (falls back to DefaultCameraRig). */
	UFUNCTION(BlueprintCallable, Category = "Zom|Camera")
	void EvaluateCameraRigForCurrentTag();

	// UObject interface. Re-implemented (not just delegated to Super) because UBlueprintCameraDirectorEvaluator's
	// own GetWorld() body lives in GameplayCameras.dll without GAMEPLAYCAMERAS_API on that specific method, so a
	// derived class in another module fails to link against its vtable slot.
	virtual UWorld* GetWorld() const override;

protected:

	// Camera rig to activate per AZomPlayerCharacter::CameraStateTag value. Configure in CDE_Player's class defaults.
	UPROPERTY(EditDefaultsOnly, Category = "Zom|Camera")
	TMap<FGameplayTag, TObjectPtr<UCameraRigAsset>> CameraRigsByTag;

	// Fallback rig used when CameraStateTag has no entry in CameraRigsByTag.
	UPROPERTY(EditDefaultsOnly, Category = "Zom|Camera")
	TObjectPtr<UCameraRigAsset> DefaultCameraRig;

private:

	// Resolved once (lazily, on first EvaluateCameraRigForCurrentTag) and reused every tick after that, instead
	// of re-running FindEvaluationContextOwnerActor/Cast every frame for an owner that never changes.
	TWeakObjectPtr<AZomPlayerCharacter> CachedPlayerCharacter;
};
