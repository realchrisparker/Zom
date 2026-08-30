// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Zom/Characters/Base/ZomCharacterBase.h"
#include "ZomBoss.generated.h"


class UStateTreeComponent;
class UAudioComponent;
class UZomZombieAttributeSet;
class UZomBossData;
class UZomSubtitleWidget;
struct FZomBossDialogueBark;


/**
 * Bespoke, does not use UZombieTypeData; owns its own ASC/AttributeSet, same pattern as zombies -
 * reuses UZomZombieAttributeSet (confirmed, Section 4.1) rather than a third attribute set class. A bespoke
 * StateTreeComponent (separate asset from the crowd base tree). Does NOT get UZomZombieAIComponent
 * ([Proposed, flag if wrong] per the dev doc's own Section 5.5 note - the encounter is a single gated,
 * always-aware fight, not a detection problem). Explicitly not pooled and not difficulty-scaled (Section 6).
 */
UCLASS(Blueprintable, meta = (DisplayName = "Zom Boss"))
class ZOM_API AZomBoss : public AZomCharacterBase
{
	GENERATED_BODY()

public:
	AZomBoss(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;

	virtual void HandleDeath() override;

	void HandleHealthAttributeChanged(const struct FOnAttributeChangeData& Data);

	void PlayBark(const FZomBossDialogueBark& Bark);

	// -------------
	// Components
	// -------------
	// The ASC subobject created in the constructor is assigned into the inherited AbilitySystemComponent
	// pointer (AZomCharacterBase) - no separate member needed for it.

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zom", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UZomZombieAttributeSet> ZombieAttributeSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zom", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStateTreeComponent> StateTreeComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zom", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAudioComponent> DialogueAudioComponent;

	// -------------
	// Properties
	// -------------

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zom")
	TObjectPtr<UZomBossData> BossData;

	// Set externally (e.g. by the HUD once Section 12's UI exists) so PlayBark has somewhere to show subtitles.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zom")
	TObjectPtr<UZomSubtitleWidget> SubtitleWidgetInstance;

private:
	bool bEncounterStartBarkPlayed = false;
	bool bPhaseTwoEntered = false;
};
