// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Zom/Characters/Enums/ZomCharacterEnums.h"
#include "AnimNotify_ZomFootstep.generated.h"


/**
 * Fires a footstep on the owning character's UZomCharacterNoiseComponent at the exact frame a foot plants.
 * The alternative to that component's stride timer, selected by its Footstep Trigger property - placing this
 * notify costs animation authoring but is frame-accurate to the visible foot plant.
 *
 * Lives in the Zom module rather than the MotionCombatSystem plugin because its only job is to find a Zom
 * component; putting it in the plugin would invert the dependency. Kept stateless, because UAnimNotify::Notify
 * runs on the class default object rather than a per-instance copy.
 */
UCLASS(Blueprintable, ClassGroup = (Zom), meta = (DisplayName = "Zom Footstep"))
class ZOM_API UAnimNotify_ZomFootstep : public UAnimNotify
{
	GENERATED_BODY()

public:

	// Which foot planted. Sets the socket the noise emits from.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zom|Noise", meta = (DisplayName = "Foot"))
	EZomFoot Foot = EZomFoot::Left;

	// Logs each time this notify resolves a component and fires, for tracking down notifies that never land.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zom|Noise", meta = (DisplayName = "Debug"))
	bool bDebug = false;

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;

#if WITH_EDITOR
	virtual FLinearColor GetEditorColor() override { return FLinearColor(0.1f, 0.6f, 0.9f); }
#endif
};
