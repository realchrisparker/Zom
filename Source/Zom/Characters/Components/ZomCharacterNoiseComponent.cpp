// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/Characters/Components/ZomCharacterNoiseComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "TimerManager.h"
#include "Zom/Characters/Base/ZomCharacterBase.h"
#include "Zom/Characters/Components/ZomCharacterAudioComponent.h"
#include "Zom/Characters/Components/ZomCharacterMovementComponent.h"
#include "Zom/Libraries/ZomNoiseLibrary.h"
#include "Zom/Misc/ZomGameplayTags.h"
#include "Zom/Misc/ZomLogChannels.h"
#include "Zom/SubSystems/SurfaceAudio/Settings/ZomNoiseSettings.h"
#include "Zom/SubSystems/SurfaceAudio/ZomSurfaceAudioSubsystem.h"


namespace
{
	// Precedent for this project, which had no debug cvars before: Zom.Debug.<Feature>, mirroring the
	// gameplay tag root. ECVF_Cheat so it is unavailable in shipping builds.
	static TAutoConsoleVariable<int32> CVarZomDebugNoise(
		TEXT("Zom.Debug.Noise"),
		0,
		TEXT("Draws the noise emission rings around characters with a UZomCharacterNoiseComponent.\n")
		TEXT("  0: off\n")
		TEXT("  1: inner (origin) and outer (audible distance) rings\n")
		TEXT("  2: also the per-step readout"),
		ECVF_Cheat);
}


// Sets default values for this component's properties
UZomCharacterNoiseComponent::UZomCharacterNoiseComponent()
{
	// Surface sampling and footstep pacing both run on timers, and everything else is event-driven - no
	// per-frame work.
	PrimaryComponentTick.bCanEverTick = false;

	DefaultFootstepSoundTag = TAG_Zom_Audio_Footstep_Default.GetTag();
}

// Called when the game starts; caches the owner and subsystem and arms the surface poll timer
void UZomCharacterNoiseComponent::BeginPlay()
{
	Super::BeginPlay();

	OwningCharacter = Cast<AZomCharacterBase>(GetOwner());
	if (!OwningCharacter.IsValid())
	{
		UE_LOG(LogZomCharacter, Warning, TEXT("%s: owner is not an AZomCharacterBase; noise emission disabled."), *GetNameSafe(GetOwner()));
		return;
	}

	if (const UWorld* World = GetWorld())
	{
		if (const UGameInstance* GameInstance = World->GetGameInstance())
		{
			SurfaceAudio = GameInstance->GetSubsystem<UZomSurfaceAudioSubsystem>();
		}
	}

	// Project-wide debug default. The per-component flag and the console variable both override it.
	if (const UZomNoiseSettings* Settings = GetDefault<UZomNoiseSettings>())
	{
		bDrawDebug |= Settings->bDrawNoiseDebug;
	}

	// A missing socket is silent and disastrous: GetSocketLocation falls back to the component's own
	// location, so every surface trace would start at the pelvis and sample whatever is under the character
	// centre. Cheap to check once, expensive to debug later.
	if (const USkeletalMeshComponent* Mesh = OwningCharacter->GetMesh())
	{
		if (!Mesh->DoesSocketExist(LeftFootSocket) || !Mesh->DoesSocketExist(RightFootSocket))
		{
			UE_LOG(LogZomCharacter, Warning, TEXT("%s: foot sockets '%s'/'%s' not found on the mesh; surface traces will start from the mesh origin."),
				*GetNameSafe(GetOwner()), *LeftFootSocket.ToString(), *RightFootSocket.ToString());
		}
	}

	if (UWorld* World = GetWorld())
	{
		// Looping: the early-outs inside skip the trace, never the timer, so there is nothing to re-arm on a
		// movement mode change.
		World->GetTimerManager().SetTimer(SurfaceSampleTimerHandle, this, &UZomCharacterNoiseComponent::HandleSurfaceSampleTimer,
			FMath::Max(SurfaceSampleInterval, 0.02f), true);
	}

	// Sample once immediately so the first footstep is not attributed to the default surface.
	SampleSurface();

	if (FootstepTrigger == EZomFootstepTrigger::StrideTimer)
	{
		ScheduleNextStride();
	}
}

// Called when the component is removed from play; clears both timers
void UZomCharacterNoiseComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SurfaceSampleTimerHandle);
		World->GetTimerManager().ClearTimer(StrideTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

// Switches between the stride timer and anim notifies at runtime.
void UZomCharacterNoiseComponent::SetFootstepTrigger(EZomFootstepTrigger NewTrigger)
{
	if (FootstepTrigger == NewTrigger)
	{
		return;
	}

	FootstepTrigger = NewTrigger;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(StrideTimerHandle);
	}

	// Start from idle so FirstStepDelay applies. In AnimNotify mode ScheduleNextStride is a no-op, so the
	// stride timer simply stays cleared.
	bStrideTimerMoving = false;
	ScheduleNextStride();
}

// Surface poll timer callback.
void UZomCharacterNoiseComponent::HandleSurfaceSampleTimer()
{
	if (!ShouldSampleSurface())
	{
		return;
	}

	SampleSurface();
}

// Whether a surface sample is worth tracing for right now.
bool UZomCharacterNoiseComponent::ShouldSampleSurface() const
{
	const AZomCharacterBase* Character = OwningCharacter.Get();
	if (!Character)
	{
		return false;
	}

	// Airborne: there is no floor to sample. Keeping the last surface means the landing step sounds like the
	// ground it lands on, which is almost always the ground it left.
	const UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
	if (!Movement || Movement->IsFalling())
	{
		return false;
	}

	// Standing still: no footstep can fire, so a trace would be pure waste. DistSquared, never Dist.
	return Character->GetVelocity().SizeSquared2D() >= FMath::Square(MinEmitSpeed);
}

// Traces down from the planted foot and caches the surface.
void UZomCharacterNoiseComponent::SampleSurface()
{
	const AZomCharacterBase* Character = OwningCharacter.Get();
	UWorld* World = GetWorld();
	if (!Character || !World)
	{
		return;
	}

	const USkeletalMeshComponent* Mesh = Character->GetMesh();
	if (!Mesh)
	{
		return;
	}

	// Trace from the planted foot - the lower of the two - so mid-stride we sample the floor the weight is
	// actually on rather than a point under the swinging leg.
	const FVector LeftFoot = Mesh->GetSocketLocation(LeftFootSocket);
	const FVector RightFoot = Mesh->GetSocketLocation(RightFootSocket);
	const FVector PlantedFoot = (LeftFoot.Z <= RightFoot.Z) ? LeftFoot : RightFoot;

	const FVector Start = PlantedFoot + FVector(0.f, 0.f, SurfaceTraceStartOffset);
	const FVector End = Start - FVector(0.f, 0.f, SurfaceTraceLength);

	// ECC_Visibility rather than a new channel: the Pawn and CharacterMesh collision profiles already respond
	// Ignore to Visibility, so the character's own capsule, every zombie and every character mesh are
	// excluded for free. A fifth custom game trace channel would buy nothing.
	//
	// The character movement component already has a floor hit every frame, but ComputeFloorDist does not set
	// bReturnPhysicalMaterial, so CurrentFloor.HitResult.PhysMaterial is always null - there is no free ride
	// here, the trace has to be its own.
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ZomFootstepSurface), bTraceComplexForSurface, Character);
	QueryParams.bReturnPhysicalMaterial = true;

	FHitResult Hit;
	if (!World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, QueryParams))
	{
		// No floor found (ledge edge, thin geometry). Keep the previous surface rather than snapping to the
		// default for a single sample - one stale footstep sounds better than one wrong one.
		return;
	}

	const EPhysicalSurface HitSurface = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

#if !UE_BUILD_SHIPPING
	if (HitSurface != CachedSurface)
	{
		UE_LOG(LogZomCharacter, Verbose, TEXT("%s: surface changed to %s"), *GetNameSafe(GetOwner()),
			*StaticEnum<EPhysicalSurface>()->GetNameStringByValue(static_cast<int64>(HitSurface)));
	}
#endif

	CachedSurface = HitSurface;

	// Resolve loudness and the sound tag here, at the poll rate, so a footstep costs zero subsystem lookups.
	if (const UZomSurfaceAudioSubsystem* Surfaces = SurfaceAudio.Get())
	{
		const FZomSurfaceNoiseEntry& Entry = Surfaces->GetSurfaceEntry(CachedSurface);
		CachedSurfaceLoudness = Entry.Loudness;
		CachedFootstepSoundTag = Entry.FootstepSoundTag;
	}
}

// Stride timer callback.
void UZomCharacterNoiseComponent::HandleStrideTimer()
{
	const bool bMoving = ShouldEmitFootstep();

	// Idle to moving: wait FirstStepDelay before the first step, so a tap of the stick is not always
	// audible. The next fire sees bStrideTimerMoving already set and steps normally.
	if (bMoving && !bStrideTimerMoving && FirstStepDelay > 0.f)
	{
		bStrideTimerMoving = true;
		ArmStrideTimer(FirstStepDelay);
		return;
	}

	if (bMoving)
	{
		NotifyFootstep(NextFoot);
		NextFoot = (NextFoot == EZomFoot::Left) ? EZomFoot::Right : EZomFoot::Left;
	}

	bStrideTimerMoving = bMoving;

	// Always re-arm, even when idle - a short check interval is what makes the first step after starting
	// to move prompt, rather than waiting out a long heartbeat.
	ScheduleNextStride();
}

// Arms the one-shot stride timer.
void UZomCharacterNoiseComponent::ScheduleNextStride()
{
	if (FootstepTrigger != EZomFootstepTrigger::StrideTimer)
	{
		return;
	}

	ArmStrideTimer(bStrideTimerMoving ? GetCurrentStrideInterval() : IdleCheckInterval);
}

// Sets the stride timer to fire once after Delay.
void UZomCharacterNoiseComponent::ArmStrideTimer(float Delay)
{
	if (UWorld* World = GetWorld())
	{
		// SetTimer treats a rate <= 0 as "clear", which would silently end the scheduler for good - keep it
		// positive, the same guard UZomCharacterAudioComponent's idle vocal scheduler needs.
		World->GetTimerManager().SetTimer(StrideTimerHandle, this, &UZomCharacterNoiseComponent::HandleStrideTimer,
			FMath::Max(Delay, 0.05f), false);
	}
}

// Re-arms the stride timer from the current tuning values.
void UZomCharacterNoiseComponent::RestartStrideTimer()
{
	// SetTimer on the same handle replaces the pending fire, so the new cadence applies to the next step.
	// Movement state is kept, so a restart while walking does not re-apply FirstStepDelay.
	ScheduleNextStride();
}

// The interval the stride timer would use right now.
float UZomCharacterNoiseComponent::GetCurrentStrideInterval() const
{
	const float Speed = OwningCharacter.IsValid() ? OwningCharacter->GetVelocity().Size2D() : 0.f;
	const float RawInterval = GetStrideDistance() / FMath::Max(Speed, 1.f) / FMath::Max(StrideRateMultiplier, 0.1f);

	// Max is taken against Min so a Blueprint that sets them the wrong way round cannot trip Clamp's ordering.
	return FMath::Clamp(RawInterval, MinStrideInterval, FMath::Max(MaxStrideInterval, MinStrideInterval));
}

#if WITH_EDITOR
// Re-arms the stride timer when a stride value is edited on a live PIE instance.
void UZomCharacterNoiseComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (!HasBegunPlay())
	{
		return;
	}

	static const TSet<FName> StrideProperties = {
		GET_MEMBER_NAME_CHECKED(UZomCharacterNoiseComponent, WalkStrideDistance),
		GET_MEMBER_NAME_CHECKED(UZomCharacterNoiseComponent, RunStrideDistance),
		GET_MEMBER_NAME_CHECKED(UZomCharacterNoiseComponent, SprintStrideDistance),
		GET_MEMBER_NAME_CHECKED(UZomCharacterNoiseComponent, CrouchStrideDistance),
		GET_MEMBER_NAME_CHECKED(UZomCharacterNoiseComponent, StrideRateMultiplier),
		GET_MEMBER_NAME_CHECKED(UZomCharacterNoiseComponent, MinStrideInterval),
		GET_MEMBER_NAME_CHECKED(UZomCharacterNoiseComponent, MaxStrideInterval),
		GET_MEMBER_NAME_CHECKED(UZomCharacterNoiseComponent, IdleCheckInterval),
		GET_MEMBER_NAME_CHECKED(UZomCharacterNoiseComponent, FirstStepDelay),
	};

	if (StrideProperties.Contains(PropertyChangedEvent.GetMemberPropertyName()))
	{
		RestartStrideTimer();
	}
	else if (PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(UZomCharacterNoiseComponent, FootstepTrigger))
	{
		// Editing the enum directly bypasses SetFootstepTrigger, so mirror what it does to the timer.
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(StrideTimerHandle);
		}
		bStrideTimerMoving = false;
		ScheduleNextStride();
	}
}
#endif

// Distance covered between footsteps for the current gait and stance.
float UZomCharacterNoiseComponent::GetStrideDistance() const
{
	const AZomCharacterBase* Character = OwningCharacter.Get();
	if (!Character)
	{
		return WalkStrideDistance;
	}

	if (Character->Stance == EStance::Crouch)
	{
		return CrouchStrideDistance;
	}

	switch (Character->Gait)
	{
	case EGait::Sprint:
		return SprintStrideDistance;

	case EGait::Run:
		return RunStrideDistance;

	default:
		return WalkStrideDistance;
	}
}

// Top speed for the current gait.
float UZomCharacterNoiseComponent::GetGaitMaxSpeed() const
{
	const AZomCharacterBase* Character = OwningCharacter.Get();
	if (!Character)
	{
		return 175.f;
	}

	// Only the player carries the custom movement component; anything else falls back to the engine's own
	// max speed rather than assuming the Zom gait speeds exist.
	const UZomCharacterMovementComponent* Movement = Cast<UZomCharacterMovementComponent>(Character->GetCharacterMovement());
	if (!Movement)
	{
		const UCharacterMovementComponent* BaseMovement = Character->GetCharacterMovement();
		return BaseMovement ? FMath::Max(BaseMovement->GetMaxSpeed(), 1.f) : 175.f;
	}

	switch (Character->Gait)
	{
	case EGait::Sprint:
		return FMath::Max(Movement->SprintSpeed, 1.f);

	case EGait::Run:
		return FMath::Max(Movement->RunSpeed, 1.f);

	default:
		return FMath::Max(Movement->WalkSpeed, 1.f);
	}
}

// How loud this character currently is before the surface is applied.
float UZomCharacterNoiseComponent::ComputeLocomotionWeight() const
{
	const AZomCharacterBase* Character = OwningCharacter.Get();
	if (!Character)
	{
		return 0.f;
	}

	// Crouch replaces the gait weight rather than scaling it: crouching is a deliberate stealth choice, and
	// a crouch-sprint should not inherit a sprint's loudness.
	float GaitStanceWeight = WalkWeight;
	if (Character->Stance == EStance::Crouch)
	{
		GaitStanceWeight = CrouchWeight;
	}
	else
	{
		switch (Character->Gait)
		{
		case EGait::Sprint:
			GaitStanceWeight = SprintWeight;
			break;

		case EGait::Run:
			GaitStanceWeight = RunWeight;
			break;

		default:
			GaitStanceWeight = WalkWeight;
			break;
		}
	}

	// Gait is set by input, not inferred from speed, so a character can hold Sprint while barely moving.
	// Scaling by how much of the gait's top speed is actually being used closes that gap, and is the only
	// proxy available for a partially-pushed stick - no analog input magnitude is stored anywhere.
	//
	// Read the velocity off the actor, not UZomAnimInstanceBase::Speed2D - that is written on a worker
	// thread and may be throttled for off-screen characters.
	const float SpeedRatio = FMath::Clamp(Character->GetVelocity().Size2D() / GetGaitMaxSpeed(), 0.f, 1.f);
	const float SpeedScale = FMath::Lerp(MinSpeedScale, 1.f, SpeedRatio);

	return GaitStanceWeight * SpeedScale * FMath::Max(FootwearWeight, 0.f);
}

// Whether the character is in a state where a footstep makes sense at all.
bool UZomCharacterNoiseComponent::ShouldEmitFootstep() const
{
	const AZomCharacterBase* Character = OwningCharacter.Get();
	if (!Character || Character->IsHidden() || Character->GetHealth() <= 0.f)
	{
		return false;
	}

	const UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
	if (!Movement || Movement->IsFalling())
	{
		return false;
	}

	return Character->GetVelocity().SizeSquared2D() >= FMath::Square(MinEmitSpeed);
}

// World location a footstep emits from.
FVector UZomCharacterNoiseComponent::GetFootLocation(EZomFoot Foot) const
{
	const AZomCharacterBase* Character = OwningCharacter.Get();
	if (!Character)
	{
		return FVector::ZeroVector;
	}

	if (const USkeletalMeshComponent* Mesh = Character->GetMesh())
	{
		if (Foot == EZomFoot::Left)
		{
			return Mesh->GetSocketLocation(LeftFootSocket);
		}

		if (Foot == EZomFoot::Right)
		{
			return Mesh->GetSocketLocation(RightFootSocket);
		}
	}

	// Unspecified, or no mesh: the capsule bottom is the closest thing to "where the character meets the floor".
	FVector Origin = Character->GetActorLocation();
	if (const UCapsuleComponent* Capsule = Character->GetCapsuleComponent())
	{
		Origin.Z -= Capsule->GetScaledCapsuleHalfHeight();
	}

	return Origin;
}

// THE footstep entry point.
void UZomCharacterNoiseComponent::NotifyFootstep(EZomFoot Foot)
{
	UWorld* World = GetWorld();
	AZomCharacterBase* Character = OwningCharacter.Get();
	if (!World || !Character || !ShouldEmitFootstep())
	{
		return;
	}

	// Rate guard. Three things need it: a stale notify left in an animation while the component is in
	// stride-timer mode, motion matching blending two databases that each carry the same notify, and a
	// montage restarting mid-stride.
	const double Now = World->GetTimeSeconds();
	if (Now - LastFootstepTime < MinTimeBetweenFootsteps)
	{
		return;
	}
	LastFootstepTime = Now;

	const FVector Origin = GetFootLocation(Foot);
	const float LocomotionWeight = ComputeLocomotionWeight();
	const float NoiseLevel = UZomNoiseLibrary::CalculateNoiseLevel(LocomotionWeight, CachedSurfaceLoudness, MaxNoiseLevel);

	const UZomNoiseSettings* Settings = GetDefault<UZomNoiseSettings>();
	const float DistanceMultiplier = Settings ? Settings->FootstepDistanceMultiplier : 1200.f;
	const float MinNoiseDistance = Settings ? Settings->MinNoiseDistance : 50.f;

	const float NoiseDistance = UZomNoiseLibrary::CalculateNoiseDistance(NoiseLevel, DistanceMultiplier);

	UZomNoiseLibrary::ReportNoise(this, Origin, NoiseDistance, Character, TAG_Zom_Noise_Footstep.GetTag(), MinNoiseDistance);

	// Audio goes through the audio component by tag, so UZomCharacterSoundSet stays the single source of
	// sound - this component decides which footstep, never what it sounds like.
	if (UZomCharacterAudioComponent* Audio = Character->GetCharacterAudioComponent())
	{
		const FGameplayTag SoundTag = CachedFootstepSoundTag.IsValid() ? CachedFootstepSoundTag : DefaultFootstepSoundTag;
		if (SoundTag.IsValid())
		{
			Audio->PlayCharacterSoundAtLocation(SoundTag, Origin);
		}
	}

	DebugWavePhase += 0.7f;
	DrawNoiseDebug(Origin, NoiseLevel, NoiseDistance);

	OnFootstepEmitted.Broadcast(Origin, NoiseLevel, NoiseDistance, CachedSurface);
}

// Reports a one-off noise that is not a footstep.
float UZomCharacterNoiseComponent::EmitNoiseAtLocation(FGameplayTag NoiseTag, FVector Location, float NoiseLevel, float DistanceMultiplier)
{
	AZomCharacterBase* Character = OwningCharacter.Get();
	if (!Character)
	{
		return 0.f;
	}

	const UZomNoiseSettings* Settings = GetDefault<UZomNoiseSettings>();
	const float MinNoiseDistance = Settings ? Settings->MinNoiseDistance : 50.f;

	const float NoiseDistance = UZomNoiseLibrary::CalculateNoiseDistance(NoiseLevel, DistanceMultiplier);
	if (!UZomNoiseLibrary::ReportNoise(this, Location, NoiseDistance, Character, NoiseTag, MinNoiseDistance))
	{
		return 0.f;
	}

	DebugWavePhase += 0.7f;
	DrawNoiseDebug(Location, NoiseLevel, NoiseDistance);

	return NoiseDistance;
}

// Whether the noise debug rings should be drawn.
bool UZomCharacterNoiseComponent::ShouldDrawDebug() const
{
#if ENABLE_DRAW_DEBUG
	return bDrawDebug || CVarZomDebugNoise.GetValueOnGameThread() > 0;
#else
	return false;
#endif
}

// Draws the origin ring, the audible-distance ring and the optional readout.
void UZomCharacterNoiseComponent::DrawNoiseDebug(const FVector& Origin, float NoiseLevel, float NoiseDistance) const
{
#if ENABLE_DRAW_DEBUG
	const UWorld* World = GetWorld();
	if (!World || !ShouldDrawDebug())
	{
		return;
	}

	// Lift both rings clear of the floor they were traced against so they do not z-fight it.
	const FVector RingCenter = Origin + FVector(0.f, 0.f, DebugRingZOffset);

	// Inner ring: the emission origin, where the foot actually planted. Fixed small radius and a damped
	// waveform, so it reads as an origin marker rather than as a second radius.
	DrawWaveformRing(RingCenter, DebugInnerRingRadius, FColor(80, 220, 120), DebugWaveAmplitude * 0.5f, 24, 2.f, DebugRingLifetime);

	// Outer ring: the computed audible distance - where a zombie whose HearingRadius equals the project's
	// ReferenceHearingRange hears this. Amplitude scales with the noise level, so a sprint on gravel is
	// visibly spiky while a crouch on carpet is nearly a smooth circle.
	const FColor OuterColor = FColor::MakeRedToGreenColorFromScalar(1.f - FMath::Clamp(NoiseLevel, 0.f, 1.f));
	// The fill's lifetime is capped at the current step interval in stride-timer mode, so fast cadences
	// replace the disc instead of stacking several translucent copies into an opaque one.
	float FillLifeTime = DebugRingLifetime;
	if (FootstepTrigger == EZomFootstepTrigger::StrideTimer)
	{
		FillLifeTime = FMath::Min(FillLifeTime, GetCurrentStrideInterval());
	}

	const uint8 FillAlpha = bDrawFilledDistance ? static_cast<uint8>(FMath::Clamp(DistanceFillOpacity, 1, 255)) : 0;
	DrawWaveformRing(RingCenter, NoiseDistance, OuterColor, DebugWaveAmplitude * NoiseLevel, DebugRingSegments, 1.5f, FillLifeTime, FillAlpha);

	if (bDrawDebugText || CVarZomDebugNoise.GetValueOnGameThread() > 1)
	{
		// Nothing in the zombie State Tree reacts to a heard noise yet, so this readout is the verification path.
		const FString Readout = FString::Printf(TEXT("Level %.2f (%.0f dB)  Radius %.0fuu  Weight %.2f  %s"),
			NoiseLevel,
			UZomNoiseLibrary::NoiseLevelToDecibels(NoiseLevel),
			NoiseDistance,
			ComputeLocomotionWeight(),
			*StaticEnum<EPhysicalSurface>()->GetNameStringByValue(static_cast<int64>(CachedSurface)));

		const FString FullReadout = FootstepTrigger == EZomFootstepTrigger::StrideTimer
			? FString::Printf(TEXT("%s  Stride %.2fs"), *Readout, GetCurrentStrideInterval())
			: Readout;

		DrawDebugString(World, RingCenter, FullReadout, nullptr, FColor::White, DebugRingLifetime, true);
	}
#endif
}

// Draws one jagged waveform ring as a closed line strip, optionally filled.
void UZomCharacterNoiseComponent::DrawWaveformRing(const FVector& Center, float Radius, const FColor& Color, float Amplitude, int32 Segments, float Thickness, float LifeTime, uint8 FillAlpha) const
{
#if ENABLE_DRAW_DEBUG
	const UWorld* World = GetWorld();
	if (!World || Radius <= 1.f || Segments < 8)
	{
		return;
	}

	const float AngleStep = 2.f * UE_PI / Segments;

	// Outline points, shared by the line strip and the fill so the disc follows the same jagged edge.
	// Index 0 is the centre (the fan hub), 1..Segments are the rim.
	TArray<FVector> Verts;
	Verts.Reserve(Segments + 1);
	Verts.Add(Center);

	for (int32 Index = 0; Index < Segments; ++Index)
	{
		const float Angle = Index * AngleStep;

		// Two out-of-phase harmonics give the jagged audio-meter silhouette without allocating per point or
		// generating a random number. Because the result is a pure function of the angle, the outline closes
		// cleanly back onto the first point.
		const float Wave = FMath::Sin(Angle * DebugWaveFrequencyA + DebugWavePhase) * 0.6f
			+ FMath::Sin(Angle * DebugWaveFrequencyB - DebugWavePhase) * 0.4f;

		const float WaveRadius = Radius * (1.f + Wave * Amplitude);
		Verts.Add(Center + FVector(FMath::Cos(Angle) * WaveRadius, FMath::Sin(Angle) * WaveRadius, 0.f));
	}

	// Flat in world XY, so on a staircase or a steep slope the ring will clip through geometry. Projecting
	// every vertex would cost one trace each - not worth it for a debug draw.
	for (int32 Index = 1; Index <= Segments; ++Index)
	{
		const int32 Next = (Index == Segments) ? 1 : Index + 1;
		DrawDebugLine(World, Verts[Index], Verts[Next], Color, false, LifeTime, 0, Thickness);
	}

	if (FillAlpha > 0)
	{
		// Triangle fan around the centre. Wound centre -> next -> current, the same order the engine uses for
		// the upward-facing side of DrawDebugSolidBox, so the disc is visible from above even if the debug
		// mesh material culls back faces. DebugMeshMaterial is translucent, so the colour's alpha is honoured.
		TArray<int32> Indices;
		Indices.Reserve(Segments * 3);
		for (int32 Index = 1; Index <= Segments; ++Index)
		{
			const int32 Next = (Index == Segments) ? 1 : Index + 1;
			Indices.Add(0);
			Indices.Add(Next);
			Indices.Add(Index);
		}

		FColor FillColor = Color;
		FillColor.A = FillAlpha;
		DrawDebugMesh(World, Verts, Indices, FillColor, false, LifeTime, 0);
	}
#endif
}
