# Zom

A third-person, single-player zombie survival action game built in **Unreal Engine 5.8**, C++-first. Zom is a learning/portfolio project: the goal is to build a small but complete vertical slice using production-grade Unreal systems — Gameplay Ability System, State Tree AI, and motion-matched animation — rather than shortcuts, so the underlying architecture is worth studying on its own.

## Goal

Deliver one tight gameplay loop: fight through a level of roaming zombies, complete a short chain of objectives, survive a boss encounter, and extract — with combat, AI, and animation all built on the same systems a shipped UE5 title would use.

## Gameplay Loop

1. **Fetch** — recover a flagged item
2. **Repair** — interact with a target to restore/unlock something
3. **Defend** — survive a wave triggered from a volume
4. **Boss** — a bespoke, non-pooled encounter gating extraction
5. **Extract** — reach the exit, locked until the above are complete

Progress is checkpointed and mirrored onto the player's ability system as Gameplay Tags, so both save/load and gameplay logic read from the same source of truth.

## Features

- **Full Gameplay Ability System (GAS) combat** — shared attribute sets (Health, Stamina, MoveSpeed, Damage as a meta-attribute) across player and AI, six core abilities (light/heavy attack, ranged shoot, reload, dodge, shove), and Gameplay Effects for damage, stagger, infection, and stamina drain.
- **Motion Combat System (MCS)** — a separate, free-to-be-released plugin also being developed and tested in this project. Every character (player, zombies, boss) carries the full MCS component set (attack core, hitbox, hit reaction, defense) alongside GAS, driving DataTable-authored, motion-matching-style attack selection, socket-based hit detection, and block/parry/dodge defense. Characters implement MCS's combat character interface to route hits into GAS damage and tag themselves (player/enemy/NPC) for the targeting subsystem's friend/foe filtering.
- **Data-driven zombie crowd AI** — `AAIController`-owned perception (sight, hearing, and a damage sense) driving a shared State Tree (`Idle → Patrol → Investigate → Chase → Attack → Staggered → Dead`). New zombie types (Walker, Runner, Auds, Eyes, Bloater) are authored as data assets, not new C++ classes.
- **Bespoke boss encounter** — separate State Tree, phase transition at a health threshold, dialogue/subtitle playback, not pooled or difficulty-scaled.
- **Zombie pooling & spawn direction** — pre-spawned/recycled actors with separate crowd and Bloater density budgets, tuned per difficulty tier via data assets.
- **Motion-matched locomotion** — Motion Matching / Chooser Framework / Motion Warping-driven animation (built on retargeted GASP animations), aim offsets, turn-in-place, and a dedicated Gameplay Camera rig.
- **Objectives & save/checkpoint** — a persistent objective subsystem, checkpoint-based restore on death or relaunch, with saved attributes reapplied through a Gameplay Effect rather than a raw write.
- **Inventory & items** — a shared data asset covering weapons (Machete, Fire Axe, Pistol, Pump Shotgun, Crossbow) and consumables (Medicine, ammo, Bandage).
- **Accessibility settings** — audio-cue captions, colorblind-safe HUD, screen-effects toggle.
- **Tag-driven character audio** — every character plays its sounds (idle growls, hit and death vocals, footsteps) by Gameplay Tag from a swappable sound set data asset, so a pooled zombie reactivated as a different type simply swaps sets. Idle vocals run on a staggered timer so a horde never growls in unison.
- **Stealth & noise** — the player's movement is audible to zombie hearing. How far a step carries depends on gait, stance, speed, footwear and the physical surface underfoot (grass is near-silent, metal carries), and one-off noises like a gunshot can be given a large range to pull in zombies from rooms the player has never seen. Hearing a noise sends a zombie to investigate the location rather than locking onto the player, because hearing ignores walls.

## Character Components

Components that live on the characters alongside the MCS combat set. All of them are timer or event driven — none tick.

| Component | Lives on | Purpose |
|---|---|---|
| `UZomCharacterMovementComponent` | Player | Gait (walk/run/sprint), stance and rotation-mode locomotion feeding the motion-matching animation. |
| `UZomCharacterAudioComponent` | Every character | Plays sounds by `Zom.Audio.*` tag from a `UZomCharacterSoundSet` (per-sound volume/pitch randomisation, play chance, cooldown), on the head-attached voice or as a one-shot in the world. Runs the idle vocal scheduler and maps hit-reaction severity to vocals. |
| `UZomCharacterNoiseComponent` | Player | Makes movement audible to AI. Samples the surface underfoot, emits a footstep noise on each step, reports it to AI hearing and hands the footstep sound to the audio component. |

Audio and noise are deliberately separate: audio is what the player hears, noise is what the AI hears. They diverge on purpose — a silenced weapon is loud to the player and quiet to a zombie.

### How the noise system fits together

| Piece | Type | Role |
|---|---|---|
| `UZomNoiseLibrary` | Blueprint function library | The reusable maths: `CalculateNoiseLevel` (locomotion weight × surface loudness, 0–1) and `CalculateNoiseDistance` (level × multiplier, in cm), plus `ReportNoise`, the project's single call into AI hearing. Usable by anything — a gunshot ability or a thrown bottle doesn't need the component. |
| `UZomSurfaceAudioSet` | Data asset (`DA_SurfaceAudio`) | Per physical surface: how loud it is underfoot and which footstep sound it plays. |
| `UZomSurfaceAudioSubsystem` | Game instance subsystem | Loads that asset once and flattens it into a fixed array indexed by surface type, so a lookup is a single array index. |
| `UZomNoiseSettings` | Project Settings > Game > Zom Stealth | Project-wide tuning: the surface set, footstep range, minimum reportable noise, and the reference hearing range. |
| `UAnimNotify_ZomFootstep` | Anim notify | Frame-accurate alternative to the component's stride timer; which one drives footsteps is a setting on the component. |

Surface types (Concrete, Wood, Metal, Grass, Dirt, Gravel, Water, Glass, Carpet, Tile) are defined in `DefaultEngine.ini`. A zombie type's `HearingRadius` acts as its hearing sensitivity: at the project's reference range (600) it hears a noise at exactly the computed distance, at double it hears twice as far.

**Debugging:** `Zom.Debug.Noise 1` in the console (or the `ZomDebugNoise` cheat) draws each footstep as two waveform rings — one at the foot, and one filled disc at the distance a zombie can hear it, coloured green to red by loudness. `Zom.Debug.Noise 2` adds a readout of noise level, dB, radius, surface and stride interval.

## Tech Stack

- **Engine:** Unreal Engine 5.8
- **Language:** C++ (Blueprints kept thin — wrappers and content-only assets)
- **Key plugins:** GameplayAbilities, GameplayStateTree / StateTree, Chooser, AnimationWarping, BlendStack, MotionWarping, MotionTrajectory, PoseSearch, SmartObjects, Mutable, CurveExpression, MotionCombatSystem (in-house, in development)

## Fab Assets Used

- [Game Animation Sample Animations Retargeted to UE5 Mannequin animations only](https://www.fab.com/listings/259f8545-f820-47b3-8fc1-e8ec5458214d) - Free - Used to create unarmed Motion Matching locomotion.
- [Modular Zombie Mega Bundle](https://www.fab.com/listings/551e8dfe-bb57-4e48-ae71-ed077fbf79a7) - Paid - Used to demostrate Mutable and spawn variation.
- [Fighting Animset Pro](https://www.fab.com/listings/950a94a9-b25d-4bad-a108-ba190ac91387) - Paid - Used to test Motion Combat System.

## Project Status

This is a work-in-progress solo project, not a finished game. See [`Zom_Development_Document.md`](Zom_Development_Document.md) for the architecture and [`Zom_Build_Checklist.md`](Zom_Build_Checklist.md) for build status.

The C++ gameplay/combat/AI scaffolding is largely in place; content authoring (State Tree assets, zombie type data, weapon/item data, physical material and surface audio assets, footstep sounds, UI widgets, main/pause menus) is still in progress.

## Getting Started

1. Requires Unreal Engine **5.8** and a C++ toolchain (Visual Studio 2022 on Windows).
2. Clone the repo and generate project files, or open [`Zom.uproject`](Zom.uproject) directly and let the editor prompt to build missing modules.
3. Compile the `Zom` module, then open the project in-editor.
