// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Chaos/ChaosEngineInterface.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Zom/Characters/Enums/ZomCharacterEnums.h"
#include "ZomCharacterNoiseComponent.generated.h"


// Forward declarations

class AZomCharacterBase;
class UZomSurfaceAudioSubsystem;


// Broadcast after a footstep has been reported, for HUD noise meters and anything else that wants to react
// to how much racket the player is making.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FZomOnFootstepEmitted, FVector, Origin, float, NoiseLevel, float, NoiseDistance, EPhysicalSurface, Surface);


/**
 * Makes a character audible to the AI. Noise is not the same thing as audio: this component decides how far
 * a sound carries to a zombie's hearing sense, while UZomCharacterAudioComponent decides what the player
 * hears. They diverge on purpose - a silenced weapon is loud to the player and quiet to the AI - so this
 * component never owns a sound, it only names a Zom.Audio.Footstep.* row and hands it to the audio component.
 *
 * Noise level is locomotion weight (gait, stance, speed, footwear) times the loudness of the surface
 * underfoot; noise distance is that level times a multiplier, which is the knob that lets a gunshot indoors
 * reach far beyond anything a footstep could. Both calculations live in UZomNoiseLibrary so non-movement
 * emitters can use them too.
 *
 * Runs on two timers and never ticks: one samples the surface underfoot, the other paces footsteps when the
 * stride-timer trigger is selected.
 */
UCLASS(ClassGroup = (Zom), meta = (BlueprintSpawnableComponent, DisplayName = "Zom Character Noise Component"))
class ZOM_API UZomCharacterNoiseComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UZomCharacterNoiseComponent();

	// -------------
	// Functions
	// -------------

	// THE footstep entry point. The stride timer and UAnimNotify_ZomFootstep both land here, so the noise
	// maths, the audio call, the debug rings and the rate guard exist exactly once whichever trigger is
	// selected. Safe to call directly for a one-off scripted step (a cutscene, a ladder rung).
	UFUNCTION(BlueprintCallable, Category = "Zom|Noise", meta = (DisplayName = "Notify Footstep"))
	void NotifyFootstep(EZomFoot Foot = EZomFoot::Unspecified);

	// Reports a one-off noise that is not a footstep - a gunshot, a thrown bottle, breaking glass. Pass a
	// large DistanceMultiplier to reach past anything movement can produce. Returns the radius it carried,
	// or 0 if it was too quiet to report.
	UFUNCTION(BlueprintCallable, Category = "Zom|Noise", meta = (DisplayName = "Emit Noise At Location"))
	float EmitNoiseAtLocation(FGameplayTag NoiseTag, FVector Location, float NoiseLevel, float DistanceMultiplier);

	// How loud this character currently is before the surface is applied: gait and stance scaled by actual
	// speed and by footwear. Exposed because a HUD noise meter wants it every frame without emitting anything.
	UFUNCTION(BlueprintPure, Category = "Zom|Noise", meta = (DisplayName = "Compute Locomotion Weight"))
	float ComputeLocomotionWeight() const;

	// The surface sampled underfoot by the poll timer.
	UFUNCTION(BlueprintPure, Category = "Zom|Noise", meta = (DisplayName = "Get Current Surface"))
	EPhysicalSurface GetCurrentSurface() const { return CachedSurface; }

	// Switches between the stride timer and anim notifies at runtime, arming or clearing the stride timer to
	// match. Setting FootstepTrigger directly would leave the timer in whatever state it was already in.
	UFUNCTION(BlueprintCallable, Category = "Zom|Noise", meta = (DisplayName = "Set Footstep Trigger"))
	void SetFootstepTrigger(EZomFootstepTrigger NewTrigger);

	// -------------
	// Properties
	// -------------

	// Fired after a footstep is reported.
	UPROPERTY(BlueprintAssignable, Category = "Zom|Noise", meta = (DisplayName = "On Footstep Emitted"))
	FZomOnFootstepEmitted OnFootstepEmitted;

	// Whether footsteps are paced by a timer or driven by animation notifies. Use SetFootstepTrigger() to
	// change this at runtime.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zom|Noise", meta = (DisplayName = "Footstep Trigger"))
	EZomFootstepTrigger FootstepTrigger = EZomFootstepTrigger::StrideTimer;

	// The per-character knob an equipment system will drive: sneakers around 0.6, bare feet lower, boots
	// around 1.3. Multiplies the whole locomotion weight, so it scales every gait at once.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zom|Noise", meta = (DisplayName = "Footwear Weight", ClampMin = "0"))
	float FootwearWeight = 1.f;

	// -------------
	// Stride timer tuning
	// -------------

	// Everything below only matters while Footstep Trigger is Stride Timer. The interval between steps is
	//   Clamp(StrideDistance / Speed / StrideRateMultiplier, MinStrideInterval, MaxStrideInterval)
	// so a longer stride means slower steps, and a faster character means quicker steps. All of it is
	// editable live in PIE and from Blueprint; call Restart Stride Timer after changing values from Blueprint
	// so the new cadence applies to the very next step instead of the one after.
	//
	// Kept as flat properties (not a struct) so values already tuned on BP_Player are not reset.

	// Distance covered per footstep, per gait. Pacing by distance rather than a fixed rate keeps the cadence
	// correct when the player changes gait, climbs a slope or half-pushes the stick - which also covers the
	// fact that no analog input magnitude is stored anywhere in this project.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zom|Noise|Stride", meta = (DisplayName = "Walk Stride Distance", ClampMin = "1", Units = "cm"))
	float WalkStrideDistance = 85.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zom|Noise|Stride", meta = (DisplayName = "Run Stride Distance", ClampMin = "1", Units = "cm"))
	float RunStrideDistance = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zom|Noise|Stride", meta = (DisplayName = "Sprint Stride Distance", ClampMin = "1", Units = "cm"))
	float SprintStrideDistance = 195.f;

	// Used whenever the character is crouched, whatever its gait.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zom|Noise|Stride", meta = (DisplayName = "Crouch Stride Distance", ClampMin = "1", Units = "cm"))
	float CrouchStrideDistance = 70.f;

	// One knob to speed up or slow down every gait's cadence at once without re-tuning four distances:
	// 2 steps twice as often, 0.5 half as often. Good for matching the timer to an animation set by eye.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zom|Noise|Stride", meta = (DisplayName = "Stride Rate Multiplier", ClampMin = "0.1", ClampMax = "5"))
	float StrideRateMultiplier = 1.f;

	// Clamp on the moving interval. The minimum sits just under the anim instance's 0.16s sprint notify
	// window so both systems agree on what is too fast to be a real step; the maximum stops a very slow
	// creep from going almost silent between steps.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zom|Noise|Stride", meta = (DisplayName = "Min Stride Interval", ClampMin = "0.05", Units = "s"))
	float MinStrideInterval = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zom|Noise|Stride", meta = (DisplayName = "Max Stride Interval", ClampMin = "0.1", Units = "s"))
	float MaxStrideInterval = 1.2f;

	// How often the timer checks for movement while the character is standing still or airborne. This is
	// the latency of the first footstep after starting to move, so keep it short; each check is only a few
	// comparisons.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zom|Noise|Stride", meta = (DisplayName = "Idle Check Interval", ClampMin = "0.05", ClampMax = "1", Units = "s"))
	float IdleCheckInterval = 0.1f;

	// Delay before the first footstep once the character starts moving, so a single tap of the stick does
	// not always make a sound. 0 means the first step happens on the first check that sees movement.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zom|Noise|Stride", meta = (DisplayName = "First Step Delay", ClampMin = "0", Units = "s"))
	float FirstStepDelay = 0.1f;

	// Re-arms the stride timer from the current tuning values, so changes made from Blueprint take effect
	// on the next step. No-op unless Footstep Trigger is Stride Timer.
	UFUNCTION(BlueprintCallable, Category = "Zom|Noise|Stride", meta = (DisplayName = "Restart Stride Timer"))
	void RestartStrideTimer();

	// The interval the stride timer would use right now, given current gait, stance, speed and tuning.
	// For tuning readouts; also shown in the debug text.
	UFUNCTION(BlueprintPure, Category = "Zom|Noise|Stride", meta = (DisplayName = "Get Current Stride Interval"))
	float GetCurrentStrideInterval() const;

protected:
	// Called when the game starts; caches the owner and subsystem and arms the surface poll timer
	virtual void BeginPlay() override;

	// Called when the component is removed from play; clears both timers
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

#if WITH_EDITOR
	// Re-arms the stride timer when a stride value is edited on a live PIE instance, so tuning is immediate
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:

	// -------------
	// Functions
	// -------------

	// Surface poll timer callback: samples the floor unless the character is airborne or standing still.
	void HandleSurfaceSampleTimer();

	// Whether a surface sample is worth tracing for right now.
	bool ShouldSampleSurface() const;

	// Traces down from the planted foot and caches the surface, its loudness and its footstep sound tag.
	void SampleSurface();

	// Stride timer callback: emits a footstep, alternates feet and always re-arms.
	void HandleStrideTimer();

	// Arms the one-shot stride timer: the moving interval while stepping, the idle check interval otherwise.
	void ScheduleNextStride();

	// Sets the stride timer to fire once after Delay (clamped positive).
	void ArmStrideTimer(float Delay);

	// Distance covered between footsteps for the current gait and stance.
	float GetStrideDistance() const;

	// Top speed for the current gait, used to scale loudness by how hard the stick is actually pushed.
	float GetGaitMaxSpeed() const;

	// Whether the character is in a state where a footstep makes sense at all.
	bool ShouldEmitFootstep() const;

	// World location a footstep emits from - the named foot's socket, or the capsule bottom if unspecified.
	FVector GetFootLocation(EZomFoot Foot) const;

	// Whether the noise debug rings should be drawn, resolving the component flag against the console variable.
	bool ShouldDrawDebug() const;

	// Draws the origin ring, the audible-distance ring and the optional readout for one emission.
	void DrawNoiseDebug(const FVector& Origin, float NoiseLevel, float NoiseDistance) const;

	// Draws one jagged waveform ring, the audio-meter look, as a closed line strip. When FillAlpha is above
	// zero the same outline is also filled with a translucent disc of that opacity.
	void DrawWaveformRing(const FVector& Center, float Radius, const FColor& Color, float Amplitude, int32 Segments, float Thickness, float LifeTime, uint8 FillAlpha = 0) const;

	// -------------
	// Properties - noise tuning
	// -------------

	// Per-gait base loudness before stance, speed and footwear. Editable so a character can be tuned without
	// touching code.
	UPROPERTY(EditAnywhere, Category = "Zom|Noise", meta = (DisplayName = "Walk Weight", ClampMin = "0"))
	float WalkWeight = 0.35f;

	UPROPERTY(EditAnywhere, Category = "Zom|Noise", meta = (DisplayName = "Run Weight", ClampMin = "0"))
	float RunWeight = 0.7f;

	UPROPERTY(EditAnywhere, Category = "Zom|Noise", meta = (DisplayName = "Sprint Weight", ClampMin = "0"))
	float SprintWeight = 1.f;

	// Replaces the gait weight entirely while crouched, rather than multiplying it - crouching is a
	// deliberate stealth choice and should not inherit a sprint's loudness.
	UPROPERTY(EditAnywhere, Category = "Zom|Noise", meta = (DisplayName = "Crouch Weight", ClampMin = "0"))
	float CrouchWeight = 0.15f;

	// Floor for the speed scale. Gait is set by input rather than inferred from speed, so without a speed
	// term a character could tap Sprint while barely moving and emit a full-volume step; without a floor,
	// creeping forward would be completely silent. Both are exploits, so the scale spans this to 1.
	UPROPERTY(EditAnywhere, Category = "Zom|Noise", meta = (DisplayName = "Min Speed Scale", ClampMin = "0", ClampMax = "1"))
	float MinSpeedScale = 0.5f;

	// Upper bound on a footstep's noise level, before the distance multiplier.
	UPROPERTY(EditAnywhere, Category = "Zom|Noise", meta = (DisplayName = "Max Noise Level", ClampMin = "0"))
	float MaxNoiseLevel = 1.f;

	// Below this 2D speed the character counts as standing still: no footsteps, no surface traces.
	UPROPERTY(EditAnywhere, Category = "Zom|Noise", meta = (DisplayName = "Min Emit Speed", ClampMin = "0", Units = "cm/s"))
	float MinEmitSpeed = 10.f;

	// Guards against a footstep firing twice in a frame or two. Needed because motion matching can blend two
	// databases that each carry the same notify, and because a stale notify may sit in an animation while the
	// component is in stride-timer mode. Sits under UZomAnimInstanceBase::GetMMNotifyRecencyTimeOut's 0.16s
	// sprint window, so it never swallows a legitimate step.
	UPROPERTY(EditAnywhere, Category = "Zom|Noise", meta = (DisplayName = "Min Time Between Footsteps", ClampMin = "0", Units = "s"))
	float MinTimeBetweenFootsteps = 0.08f;

	// Footstep row used when the surface names none.
	UPROPERTY(EditAnywhere, Category = "Zom|Noise", meta = (DisplayName = "Default Footstep Sound Tag", Categories = "Zom.Audio.Footstep"))
	FGameplayTag DefaultFootstepSoundTag;

	// -------------
	// Properties - surface sampling
	// -------------

	// How often the floor is sampled. At sprint speed this is one trace per 150 cm travelled, finer than any
	// authored surface patch, and it is one line trace for one actor - far cheaper than tracing per footstep.
	UPROPERTY(EditAnywhere, Category = "Zom|Noise|Surface", meta = (DisplayName = "Surface Sample Interval", ClampMin = "0.02", Units = "s"))
	float SurfaceSampleInterval = 0.25f;

	// Foot sockets the surface trace starts from. Editable because a different skeleton may name them
	// differently; BeginPlay warns once if they are missing, since GetSocketLocation would otherwise silently
	// return the component's own location and every trace would start at the pelvis.
	UPROPERTY(EditAnywhere, Category = "Zom|Noise|Surface", meta = (DisplayName = "Left Foot Socket"))
	FName LeftFootSocket = TEXT("foot_l");

	UPROPERTY(EditAnywhere, Category = "Zom|Noise|Surface", meta = (DisplayName = "Right Foot Socket"))
	FName RightFootSocket = TEXT("foot_r");

	UPROPERTY(EditAnywhere, Category = "Zom|Noise|Surface", meta = (DisplayName = "Surface Trace Start Offset", ClampMin = "0", Units = "cm"))
	float SurfaceTraceStartOffset = 20.f;

	UPROPERTY(EditAnywhere, Category = "Zom|Noise|Surface", meta = (DisplayName = "Surface Trace Length", ClampMin = "1", Units = "cm"))
	float SurfaceTraceLength = 80.f;

	// Landscapes carry their per-layer physical material only on complex collision, while a static mesh floor
	// carries one material on its simple collision and may have no complex collision at all. One bool,
	// expected to be flipped per level type.
	UPROPERTY(EditAnywhere, Category = "Zom|Noise|Surface", meta = (DisplayName = "Trace Complex For Surface"))
	bool bTraceComplexForSurface = false;

	// -------------
	// Properties - debug
	// -------------

	// Per-Blueprint override for the debug rings. The Zom.Debug.Noise console variable turns them on
	// regardless, and UZomNoiseSettings supplies the project-wide default at BeginPlay.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zom|Debug", meta = (DisplayName = "Draw Debug", AllowPrivateAccess = "true"))
	bool bDrawDebug = false;

	// Adds the per-step readout (level, radius, weight, surface) next to the rings.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zom|Debug", meta = (DisplayName = "Draw Debug Text", AllowPrivateAccess = "true"))
	bool bDrawDebugText = false;

	UPROPERTY(EditAnywhere, Category = "Zom|Debug", meta = (DisplayName = "Debug Ring Lifetime", ClampMin = "0.05", Units = "s"))
	float DebugRingLifetime = 0.35f;

	UPROPERTY(EditAnywhere, Category = "Zom|Debug", meta = (DisplayName = "Debug Ring Segments", ClampMin = "8"))
	int32 DebugRingSegments = 64;

	UPROPERTY(EditAnywhere, Category = "Zom|Debug", meta = (DisplayName = "Debug Inner Ring Radius", ClampMin = "1", Units = "cm"))
	float DebugInnerRingRadius = 25.f;

	// Fills the audible-distance ring with a translucent disc, so the area a zombie can hear the step from
	// reads at a glance instead of only as an outline.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zom|Debug", meta = (DisplayName = "Draw Filled Distance", AllowPrivateAccess = "true"))
	bool bDrawFilledDistance = true;

	// Opacity of that disc, 0-255. Keep it low: the disc sits on top of the floor it describes, and at sprint
	// cadence consecutive discs can briefly overlap.
	UPROPERTY(EditAnywhere, Category = "Zom|Debug", meta = (DisplayName = "Distance Fill Opacity", ClampMin = "1", ClampMax = "255", EditCondition = "bDrawFilledDistance"))
	int32 DistanceFillOpacity = 40;

	// How far the waveform deviates from a clean circle, scaled by noise level at draw time so a loud step is
	// visibly spikier than a quiet one - the jaggedness itself encodes loudness.
	UPROPERTY(EditAnywhere, Category = "Zom|Debug", meta = (DisplayName = "Debug Wave Amplitude", ClampMin = "0", ClampMax = "1"))
	float DebugWaveAmplitude = 0.06f;

	// Two deliberately mismatched harmonics, so they do not line up into an obviously regular flower shape.
	UPROPERTY(EditAnywhere, Category = "Zom|Debug", meta = (DisplayName = "Debug Wave Frequency A"))
	float DebugWaveFrequencyA = 9.f;

	UPROPERTY(EditAnywhere, Category = "Zom|Debug", meta = (DisplayName = "Debug Wave Frequency B"))
	float DebugWaveFrequencyB = 23.f;

	// Lifts the rings clear of the floor they were traced against, so they do not z-fight it.
	UPROPERTY(EditAnywhere, Category = "Zom|Debug", meta = (DisplayName = "Debug Ring Z Offset", Units = "cm"))
	float DebugRingZOffset = 3.f;

	// -------------
	// Properties - runtime state
	// -------------

	// Owner, cached once. Weak because the component does not own the character.
	TWeakObjectPtr<AZomCharacterBase> OwningCharacter;

	// Surface table, cached once. The subsystem outlives the component (game instance scoped).
	TWeakObjectPtr<UZomSurfaceAudioSubsystem> SurfaceAudio;

	// Last sampled surface and the values resolved from it, so a footstep costs no subsystem lookups.
	EPhysicalSurface CachedSurface = SurfaceType_Default;
	float CachedSurfaceLoudness = 1.f;
	FGameplayTag CachedFootstepSoundTag;

	// Whether the stride timer was pacing steps (true) or idling (false) when it last armed, so the idle to
	// moving transition can apply FirstStepDelay.
	bool bStrideTimerMoving = false;

	// Which foot the stride timer emits next.
	EZomFoot NextFoot = EZomFoot::Left;

	// World time of the last emitted footstep, for the rate guard.
	double LastFootstepTime = 0.0;

	// Advanced on every emission so consecutive rings do not look identical.
	float DebugWavePhase = 0.f;

	FTimerHandle SurfaceSampleTimerHandle;
	FTimerHandle StrideTimerHandle;
};
