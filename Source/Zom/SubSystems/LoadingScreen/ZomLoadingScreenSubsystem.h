// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "ZomLoadingScreenSubsystem.generated.h"


class UUserWidget;
struct FWorldContext;


/**
 * Shows UZomGameInstance::LoadingScreenClass across every level load, in two phases:
 *
 * 1. Blocking load - the game thread is stuck in LoadMap, so the widget is handed to the engine MoviePlayer,
 *    which draws it on its own thread. Only Slate-driven animation (e.g. a Throbber) moves here; UMG widget
 *    animations don't tick. MoviePlayer doesn't run in the editor, so in PIE this phase shows a frozen frame.
 *
 * 2. Hold - after the level has loaded (and BeginPlay has run) the same widget class is put on the viewport and
 *    the world is paused until shader compilation, PSO precaching, texture/mesh streaming and streaming levels
 *    have all settled, with a minimum display time and a timeout from UZomGameInstance.
 */
UCLASS()
class ZOM_API UZomLoadingScreenSubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintPure, Category = "Zom|Loading")
	bool IsLoadingScreenVisible() const { return bHolding; }

	// FTickableGameObject
	virtual void Tick(float DeltaTime) override;
	virtual ETickableTickType GetTickableTickType() const override;
	virtual bool IsTickable() const override { return bHolding; }
	virtual bool IsTickableWhenPaused() const override { return true; }
	virtual TStatId GetStatId() const override;

private:
	void HandlePreLoadMap(const FWorldContext& WorldContext, const FString& MapName);
	void HandlePostLoadMap(UWorld* LoadedWorld);
	void HandlePrepareMovieLoadingScreen();

	// Fills OutPendingReason with what is still loading, for the timeout log.
	bool IsWorldReady(const UWorld& World, FString& OutPendingReason) const;

	void StartHold(UWorld& World);
	void FinishHold();

	// Drawn by MoviePlayer during the blocking load.
	UPROPERTY()
	TObjectPtr<UUserWidget> MovieWidget;

	// On the viewport during the hold phase.
	UPROPERTY()
	TObjectPtr<UUserWidget> HoldWidget;

	TWeakObjectPtr<UWorld> HoldWorld;
	double HoldStartTime = 0.0;
	int32 ConsecutiveReadyFrames = 0;
	bool bHolding = false;
	bool bPausedByLoadingScreen = false;
};
