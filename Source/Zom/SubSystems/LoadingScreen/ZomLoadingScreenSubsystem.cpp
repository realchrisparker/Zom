// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/SubSystems/LoadingScreen/ZomLoadingScreenSubsystem.h"
#include "Zom/Game/ZomGameInstance.h"
#include "Zom/Misc/ZomLogChannels.h"
#include "Blueprint/UserWidget.h"
#include "ContentStreaming.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "MoviePlayer.h"
#include "PipelineStateCache.h"
#include "ShaderCompiler.h"
#include "ShaderPipelineCache.h"


namespace ZomLoadingScreen
{
	// Everything must report ready for this many frames in a row - a single quiet frame can be a gap between
	// one batch of streaming/PSO requests and the next.
	static constexpr int32 RequiredReadyFrames = 5;

	// Above the HUD (0) and menu root (10).
	static constexpr int32 ViewportZOrder = 1000;
}

void UZomLoadingScreenSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FCoreUObjectDelegates::PreLoadMapWithContext.AddUObject(this, &UZomLoadingScreenSubsystem::HandlePreLoadMap);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UZomLoadingScreenSubsystem::HandlePostLoadMap);

	if (IsMoviePlayerEnabled() && GetMoviePlayer())
	{
		// MoviePlayer starts itself on PreLoadMap and asks for its widget through this if none is set up yet -
		// avoids depending on the order PreLoadMap listeners run in.
		GetMoviePlayer()->OnPrepareLoadingScreen().AddUObject(this, &UZomLoadingScreenSubsystem::HandlePrepareMovieLoadingScreen);
	}
}

void UZomLoadingScreenSubsystem::Deinitialize()
{
	FCoreUObjectDelegates::PreLoadMapWithContext.RemoveAll(this);
	FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);

	if (GetMoviePlayer())
	{
		GetMoviePlayer()->OnPrepareLoadingScreen().RemoveAll(this);
	}

	FinishHold();

	Super::Deinitialize();
}

ETickableTickType UZomLoadingScreenSubsystem::GetTickableTickType() const
{
	// The CDO is also an FTickableGameObject - keep it out of the tick list.
	return IsTemplate() ? ETickableTickType::Never : ETickableTickType::Conditional;
}

TStatId UZomLoadingScreenSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UZomLoadingScreenSubsystem, STATGROUP_Tickables);
}

// Phase 1 - blocking load (MoviePlayer)
// =========================================

void UZomLoadingScreenSubsystem::HandlePreLoadMap(const FWorldContext& WorldContext, const FString& MapName)
{
	// PIE can run several game instances; only react to our own travel.
	if (WorldContext.OwningGameInstance != GetGameInstance())
	{
		return;
	}

	// A new load during a hold (e.g. travelling straight on) - the old world's viewport widget is torn down with it.
	FinishHold();
}

void UZomLoadingScreenSubsystem::HandlePrepareMovieLoadingScreen()
{
	const UZomGameInstance* GameInstance = Cast<UZomGameInstance>(GetGameInstance());
	if (!GameInstance || !GameInstance->GetLoadingScreenClass())
	{
		return;
	}

	if (!MovieWidget)
	{
		MovieWidget = CreateWidget<UUserWidget>(GetGameInstance(), GameInstance->GetLoadingScreenClass());
	}

	FLoadingScreenAttributes Attributes;
	Attributes.WidgetLoadingScreen = MovieWidget->TakeWidget();
	Attributes.bAutoCompleteWhenLoadingCompletes = true;
	Attributes.bMoviesAreSkippable = false;
	// Must stay false: MoviePlayer spins the game thread until manually stopped, which would block phase 2.
	Attributes.bWaitForManualStop = false;

	GetMoviePlayer()->SetupLoadingScreen(Attributes);
}

// Phase 2 - hold until the level is ready
// =========================================

void UZomLoadingScreenSubsystem::HandlePostLoadMap(UWorld* LoadedWorld)
{
	if (!LoadedWorld || LoadedWorld->GetGameInstance() != GetGameInstance())
	{
		return;
	}

	StartHold(*LoadedWorld);
}

void UZomLoadingScreenSubsystem::StartHold(UWorld& World)
{
	const UZomGameInstance* GameInstance = Cast<UZomGameInstance>(GetGameInstance());
	if (!GameInstance || !GameInstance->GetLoadingScreenClass())
	{
		return;
	}

	HoldWidget = CreateWidget<UUserWidget>(GetGameInstance(), GameInstance->GetLoadingScreenClass());
	if (!HoldWidget)
	{
		return;
	}
	HoldWidget->AddToViewport(ZomLoadingScreen::ViewportZOrder);

	// Keeps zombies/AI and the player from acting while the screen is up. Rendering, streaming and PSO
	// compilation all continue while paused.
	bPausedByLoadingScreen = !UGameplayStatics::IsGamePaused(&World) && UGameplayStatics::SetGamePaused(&World, true);

	HoldWorld = &World;
	HoldStartTime = FPlatformTime::Seconds();
	ConsecutiveReadyFrames = 0;
	bHolding = true;
}

void UZomLoadingScreenSubsystem::Tick(float DeltaTime)
{
	UWorld* World = HoldWorld.Get();
	if (!World)
	{
		FinishHold();
		return;
	}

	const UZomGameInstance* GameInstance = Cast<UZomGameInstance>(GetGameInstance());
	const double Elapsed = FPlatformTime::Seconds() - HoldStartTime;

	FString PendingReason;
	const bool bReady = IsWorldReady(*World, PendingReason);
	ConsecutiveReadyFrames = bReady ? ConsecutiveReadyFrames + 1 : 0;

	if (GameInstance && Elapsed >= GameInstance->GetLoadingScreenTimeout())
	{
		UE_LOG(LogZomGame, Warning, TEXT("Loading screen timed out after %.1fs, still waiting on: %s"), Elapsed, *PendingReason);
		FinishHold();
		return;
	}

	const float MinDisplayTime = GameInstance ? GameInstance->GetLoadingScreenMinDisplayTime() : 0.f;
	if (Elapsed >= MinDisplayTime && ConsecutiveReadyFrames >= ZomLoadingScreen::RequiredReadyFrames)
	{
		FinishHold();
	}
}

bool UZomLoadingScreenSubsystem::IsWorldReady(const UWorld& World, FString& OutPendingReason) const
{
	TArray<FString> Pending;

	// Only non-zero in editor/uncooked builds - cooked builds ship compiled shaders.
	if (GShaderCompilingManager && GShaderCompilingManager->GetNumRemainingJobs() > 0)
	{
		Pending.Add(FString::Printf(TEXT("%d shader jobs"), GShaderCompilingManager->GetNumRemainingJobs()));
	}

	// Pipeline states for what's on screen - the source of first-view hitches in cooked builds.
	if (const uint32 PSORequests = PipelineStateCache::NumActivePrecacheRequests())
	{
		Pending.Add(FString::Printf(TEXT("%u PSO precache requests"), PSORequests));
	}
	if (const uint32 BundledPSOs = FShaderPipelineCache::NumPrecompilesRemaining())
	{
		Pending.Add(FString::Printf(TEXT("%u bundled PSO precompiles"), BundledPSOs));
	}

	// Textures and meshes still streaming in their required mips/LODs.
	if (const int32 WantingResources = IStreamingManager::Get().GetNumWantingResources())
	{
		Pending.Add(FString::Printf(TEXT("%d streaming resources"), WantingResources));
	}

	if (World.HasStreamingLevelsToConsider() || World.IsVisibilityRequestPending())
	{
		Pending.Add(TEXT("streaming levels"));
	}

	OutPendingReason = FString::Join(Pending, TEXT(", "));
	return Pending.IsEmpty();
}

void UZomLoadingScreenSubsystem::FinishHold()
{
	if (!bHolding)
	{
		return;
	}

	if (HoldWidget)
	{
		HoldWidget->RemoveFromParent();
		HoldWidget = nullptr;
	}

	if (bPausedByLoadingScreen)
	{
		if (UWorld* World = HoldWorld.Get())
		{
			UGameplayStatics::SetGamePaused(World, false);
		}
	}

	bPausedByLoadingScreen = false;
	HoldWorld = nullptr;
	bHolding = false;
}
