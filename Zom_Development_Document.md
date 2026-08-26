# Zom — Development Document

*Class & subsystem map for implementation. Derived from `Zom_Design_Document.md`. Unreal Engine 5.8, C++-first.*

---

## 0. How to Read This Document

Every entry below is one of three things, and it's labeled as such:

- **[Design]** — directly specified in the design document. Build it as written.
- **[Proposed]** — the design doc implied this was needed but never named it, or left a detail open. I've filled it in with a concrete recommendation. Confirm or override before coding against it.
- **[Open]** — a real decision the design doc deferred. Do not build speculative infrastructure for these until you decide.

**If you're an agent implementing directly from this document:** `[Design]` items are cleared to build. `[Open]` items and anything marked `[Flagged]`/"flag if wrong" are not yours to resolve, stop and ask rather than picking a reasonable-looking default. As of this revision the remaining genuinely open item is cross-zombie alerting (5.4) — confirmed as wanted, mechanism not designed, do not build it; not scheduled yet, revisit later. `[Proposed]` items are safe to build, but say so in your output, don't silently treat a proposal as if it were a confirmed decision. Also reconcile this document against whatever code already exists in the project before writing anything, existing code wins over what's written here if the two disagree.

**Naming note (resolved):** the project's actual code names the player pawn/base classes `AZomPlayerCharacter` / `AZomPlayerCharacterBase`, not the shorter `AZomCharacter` / `AZomCharacterBase` used in earlier drafts of this document. This revision updates every reference to match the code, per the "existing code wins" rule above. See Section 17 for the full build-status audit this revision is based on.

---

## 1. Naming Convention

| Prefix | Used for | Example |
|---|---|---|
| `A` | Actors | `AZomPlayerCharacter` |
| `U` (Object) | UObjects, Components, Subsystems, Data Assets | `UZomAttributeSetBase` |
| `E` | Enums (uint8-backed per project performance rules) | `EZomCheckpointID` |
| `F` | Structs | `FZomZombieSpawnRequest` |
| `GA_` | Gameplay Ability subclasses | `UZomGA_LightAttack` |
| `GE_` | Gameplay Effect subclasses | `UZomGE_Infection` |
| `Zom.` | Gameplay Tag root | `Zom.Status.Staggered` — every tag sits under one root so the tag picker groups them together instead of scattering across `Status.*`, `Objective.*`, `Perception.*` as separate trees |

---

## 2. Class Hierarchy (Actors)

```
ACharacter
└── AZomPlayerCharacterBase             [Design, confirmed] — base class for all humanoids; shared movement setup, declares IAbilitySystemInterface
    ├── AZomPlayerCharacter        [Design]   — player; ASC lives on AZomPlayerState, not here
    ├── AZomZombieBase             [Design]   — crowd zombies (Regular/Auds/Eyes/Bloater); owns its own ASC/AttributeSet
    │   └── (no C++ subclasses — type differs by UZombieTypeData, not inheritance)
    └── AZomBoss                   [Design]   — bespoke, does not use UZombieTypeData; owns its own ASC/AttributeSet, same pattern as zombies

APlayerState
└── AZomPlayerState                [Design, confirmed] — owns the player's UAbilitySystemComponent, UZomAttributeSetBase, UZomPlayerAttributeSet

APlayerStart
└── AZomCheckpoint                 [Design]

AGameModeBase
└── AZomGameMode                   [Design]   — overrides ChoosePlayerStart_Implementation

AActor
├── AZomToxicGasVolume             [Design]   — Bloater death hazard
├── AZomFetchItem                  [Proposed] — pickup actor for the Fetch objective
├── AZomRepairTarget               [Proposed] — interactable actor for the Repair objective
├── AZomDefendVolume               [Proposed] — trigger volume that starts the Defend wave
└── AZomExtractionPoint            [Proposed] — locked-until-objectives-complete exit trigger
```

**Naming note:** `AZomPlayerCharacterBase` is named for its origin (it started life as player-only scaffolding, now repurposed per Section 17 as the shared humanoid base), not for its final scope — despite the name, `AZomZombieBase` and `AZomBoss` both inherit from it too, exactly as `AZomCharacterBase` did in earlier drafts. Flag if this name should change to something scope-neutral once more of the hierarchy exists; not resolved yet, just carried forward as-is per the "repurpose the existing class" decision.

**Why `AZomPlayerCharacterBase` holds a cached ASC pointer instead of a fully-owned component:** this is the pattern Epic's own ActionRPG sample uses (`AAbilitySystemCharacterBase`), and it's the right fit here. `AZomPlayerCharacterBase` declares a protected `TObjectPtr<UAbilitySystemComponent>` member and implements `GetAbilitySystemComponent()` exactly once, so every subclass and every piece of external code (HUD widgets, ability code, GameplayEffect application) calls the same function without caring where the component physically lives. What differs per subclass is how that pointer gets populated:

- `AZomZombieBase` and `AZomBoss` call `CreateDefaultSubobject<UAbilitySystemComponent>()` in their own constructors (they own it, it's a real subobject on their actor), then assign it into the inherited pointer and call the shared `InitializeAbilitySystem(this, this)` from their own `BeginPlay`.
- `AZomPlayerCharacter` never creates an ASC. It assigns the inherited pointer to whatever `AZomPlayerState::GetAbilitySystemComponent()` returns, and calls `InitializeAbilitySystem(PlayerState, this)` from both `PossessedBy()` and `OnRep_PlayerState()`.

**Implementation note, worth getting right the first time:** add a protected `InitializeAbilitySystem(AActor* InOwnerActor, AActor* InAvatarActor)` helper on `AZomPlayerCharacterBase` that does the pointer assignment and calls `ASC->InitAbilityActorInfo(InOwnerActor, InAvatarActor)`. For AI, `OwnerActor == AvatarActor == self`. For the player, `OwnerActor` is the PlayerState, `AvatarActor` is the pawn, called from both `PossessedBy()` (server/local) and `OnRep_PlayerState()` (matters under replication; worth structuring correctly now since it's the exact pattern you'd need if this ever goes multiplayer). `AZomPlayerState` should still implement `IAbilitySystemInterface` itself too, returning its own local component, since some GAS utility functions resolve the ASC by calling the interface directly on whatever actor they're handed, and that actor is sometimes the PlayerState, not the pawn.

**Why the four objective actors are named here:** the design doc describes objective *behavior* (pick up a flagged item, interact with a target, trigger volume, locked exit) but never names the actor classes that implement it. Something has to own that collision/interaction logic, and it shouldn't be `UZomObjectiveSubsystem` itself, that subsystem should own state and broadcast delegates, not BeginOverlap logic. These four are my proposal for where that logic lives.

---

## 3. Core Framework

| Class | Base | Responsibility |
|---|---|---|
| `AZomPlayerCharacterBase` | `ACharacter` | [Design, confirmed] Shared movement setup. Implements `IAbilitySystemInterface::GetAbilitySystemComponent()` once, returning a cached `TObjectPtr<UAbilitySystemComponent>` populated by subclasses (Section 4.5). Also hosts shared helpers (`InitializeAbilitySystem()`, `GrantAbilitySet()`) so combat code doesn't care where the ASC physically lives. `PrimaryActorTick.bCanEverTick = false` at this level; subclasses opt back in only if they have a proven per-frame need. |
| `AZomPlayerState` | `APlayerState` | [Design, confirmed] Owns `UAbilitySystemComponent`, `UZomAttributeSetBase`, and `UZomPlayerAttributeSet` for the player as real subobjects. Implements `IAbilitySystemInterface` directly (some GAS lookups resolve the ASC by calling the interface on whatever actor they're handed, which is sometimes the PlayerState, not the pawn). |
| `AZomPlayerCharacter` | `AZomPlayerCharacterBase` | [Design] Player character/pawn. Creates no ASC of its own, calls `InitializeAbilitySystem(PlayerState, this)` from `PossessedBy()`/`OnRep_PlayerState()` to populate the inherited cached pointer. Adds `UZomInventoryComponent`, handles Enhanced Input binding. |
| `AZomZombieBase` | `AZomPlayerCharacterBase` | [Design] Creates its own `UAbilitySystemComponent` and `UZomZombieAttributeSet` as real subobjects, calls `InitializeAbilitySystem(this, this)` from its own `BeginPlay`. Sets `AIControllerClass = AZomZombieAIController::StaticClass()` and `AutoPossessAI = PlacedInWorldOrSpawned` (Section 5.5, revised) rather than owning perception/State Tree itself, and holds a `TObjectPtr<UZombieTypeData>` reference. No per-type C++ subclassing, behavior differences come from data and from which senses are configured on the possessing `AZomZombieAIController`. |
| `AZomBoss` | `AZomPlayerCharacterBase` | [Design] Same ASC pattern as `AZomZombieBase`, including reusing `UZomZombieAttributeSet` (confirmed, see 4.1). Bespoke `UStateTreeComponent` (separate asset from the crowd base tree), owns `UZomBossData`, dialogue playback, phase-transition tag. Explicitly **not** pooled and **not** difficulty-scaled. |
| `AZomGameMode` | `AGameModeBase` | [Design] Constructor sets `DefaultPawnClass = AZomPlayerCharacter::StaticClass()` and, critically, `PlayerStateClass = AZomPlayerState::StaticClass()`, without this the engine silently spawns a default `APlayerState` and the entire ASC-on-PlayerState design from Section 2 quietly does nothing. `PlayerControllerClass` stays the engine default per the `AZomPlayerController` row below. Also overrides `ChoosePlayerStart_Implementation` to select the `AZomCheckpoint` matching the loaded save's checkpoint ID, and loads the save slot on `InitGame`/early `PostLogin`. |

**Blueprint wrapper convention:** `AZomGameMode` is a C++ class, but the asset actually assigned in Project Settings → Maps & Modes (and any World Settings override) should be a thin Blueprint child, `BP_ZomGameMode`, with no graph logic of its own. All behavior lives in the C++ base; the Blueprint exists only because that's the conventional assignment point and it costs nothing to leave the door open for a later per-level override without a new C++ class. Since the constructor sets `DefaultPawnClass`/`PlayerStateClass` directly rather than relying on Blueprint defaults, the game behaves correctly even if `BP_ZomGameMode` is never touched beyond existing. **Status note:** the actual project's Blueprint is currently named `BP_GameMode`, not `BP_ZomGameMode` — see Section 17. Not yet resolved whether to rename the asset or update this convention; flagged, not decided.
| `AZomPlayerController` | `APlayerController` | [Design, exists] Already built in the project (`Source/Zom/Characters/ZomPlayerController.h/.cpp`). Currently minimal: caches the possessed pawn as `AZomPlayerCharacter*`, empty `BeginPlay`/`SetupInputComponent` overrides. Enhanced Input mapping context still needs to be added here (or on the pawn) — not yet wired in either place. See Section 17. |

---

## 4. Combat / GAS Layer

Full GAS per your resolved decision (design doc: "teaching GAS itself is worth the added complexity"). This is the one system explicitly exempted from the "small enough to read end to end" constraint.

**Status note:** as of this revision, the `GameplayAbilities`/`GameplayTasks` modules are not yet referenced in `Zom.Build.cs`, and no Gameplay Tags are registered anywhere in the project (no `DefaultGameplayTags.ini`). This entire section is unbuilt — see Section 17. Adding the GAS module dependency is a prerequisite step before any class in this section can compile.

### 4.1 Attribute Sets

| Class | Owner | Attributes |
|---|---|---|
| `UZomAttributeSetBase` | `AZomPlayerState` (player) / `AZomZombieBase` or `AZomBoss` directly (AI), one instance per owning actor | `Health`, `MaxHealth`, `Damage`, `MoveSpeed`, `MaxMoveSpeed` |
| `UZomPlayerAttributeSet` : `UZomAttributeSetBase` | `AZomPlayerState` only | `Stamina`, `MaxStamina` |
| `UZomZombieAttributeSet` : `UZomAttributeSetBase` | `AZomZombieBase` / `AZomBoss` | `AttackDamage` [Proposed, see note] |

**Why `Damage` has to stay on the base, not split off as player-only or zombie-only:** this isn't a style call, it's a GAS mechanic. `Damage` here is a meta-attribute, `UZomGE_Damage` writes a magnitude into it via a Modifier, and `PostGameplayEffectExecute` converts that into a `-Health` delta and resets `Damage` to zero. A single `UZomGE_Damage` effect class needs to target the *same* `(AttributeSet class, property)` pair regardless of whether the victim is the player or a zombie, that only works if both inherit `Damage` from one shared ancestor. If you split `Damage` into two separate properties on two separate subclasses, you'd need two separate damage GameplayEffects (or a branch inside one), which is exactly the duplication the base class exists to avoid. Health has the same requirement for the same reason, you already had that right.

**Why `UZomZombieAttributeSet` proposes `AttackDamage` and not something else:** you asked for zombies to have their own attribute set, but didn't say what goes in it. Right now nothing *requires* a zombie-specific attribute, `UZombieTypeData` already carries a static damage value per type. What `AttackDamage` as a real GAS attribute buys you is that `UZomDifficultyData` (Section 10) could scale zombie damage at runtime with an Infinite GameplayEffect applied on spawn, the same production pattern you're already using for infection duration, instead of needing a second data-asset override path. That's a genuine capability, not just symmetry with the player class. If you don't want that capability, say so and `UZomZombieAttributeSet` becomes an empty extension point for now, which is still a legitimate class to have, it's just inheriting everything and adding nothing until you need it.

**`AZomBoss` reuses `UZomZombieAttributeSet`**, confirmed, rather than getting a third subclass, since the Boss is AI-controlled like a zombie and nothing about it currently needs an attribute a zombie doesn't have.

**Naming it `MaxMoveSpeed`, not `MaxSpeed`:** matches the `Health`/`MaxHealth` convention already in this table, a `Max` prefix on the thing it caps. Rename it back if you had a specific reason for the shorter form.

**Resolved: `MoveSpeed` moved up to the base, both properties now live together.** Zombies get GAS-modifiable speed, not just the player, opening the door to slow/haste GameplayEffects that work on any humanoid (a player ability that slows a zombie, a difficulty tier that hastens the crowd, whatever comes up later). `PostGameplayEffectExecute` clamps `MoveSpeed` between `0` and `MaxMoveSpeed` for every combatant, one implementation, same as the `Damage`/`Health` clamp below.

**This creates a new single-source-of-truth question, worth resolving now rather than leaving it to surface later:** `UZombieTypeData` (Section 5.1) already carries a static `speed` field per zombie type. Now that `MoveSpeed` is a real GAS attribute every zombie owns, there are two places speed could live, the data asset's static field and the runtime attribute, and they will drift if both are treated as authoritative. Resolution: `UZombieTypeData::Speed` is the *initial* value, read once at `BeginPlay` to set the zombie's `MoveSpeed` attribute (the same way `UZombieTypeData` already seeds Health), after that the attribute is the live value and GameplayEffects modify it, not the data asset. The data asset defines what a fresh Walker or Runner starts at, it was never meant to be re-read mid-fight.

**Shared conversion logic lives once, on the base:** `UZomAttributeSetBase::PostGameplayEffectExecute` does the `Damage → -Health` conversion and clamps `Health` between `0` and `MaxHealth`, and clamps `MoveSpeed` between `0` and `MaxMoveSpeed`, for every combatant, one clamp implementation, not per-subclass copies. Same principle as `GetAbilitySystemComponent()` in 4.5, implement it once at the base, every subclass inherits correct behavior for free instead of three copies of the same clamp logic.

### 4.2 Abilities

| Class | Base | Trigger | Notes |
|---|---|---|---|
| `UZomGameplayAbility` | `UGameplayAbility` | — | [Proposed] Shared ability base. Sets `InstancingPolicy = InstancedPerActor` and `NetExecutionPolicy = LocalPredicted` once in the constructor, consistent with the earlier full-GAS decision to teach production patterns even though prediction goes unused in a singleplayer build. Also sets `ActivationBlockedTags` to include `Zom.Status.Staggered` at this level, so no individual ability class has to remember to block itself while staggered, the base handles it for all six. |
| `UZomGA_LightAttack` | `UZomGameplayAbility` | Input | [Design] |
| `UZomGA_HeavyAttack` | `UZomGameplayAbility` | Input | [Design] Gated by Stamina cost |
| `UZomGA_RangedShoot` | `UZomGameplayAbility` | Input | [Design] |
| `UZomGA_Reload` | `UZomGameplayAbility` | Input | [Design] |
| `UZomGA_Dodge` | `UZomGameplayAbility` | Input | [Design] Gated by Stamina, applies `Zom.Status.Dodging` tag |
| `UZomGA_Shove` | `UZomGameplayAbility` | Input | [Design] Signature ability, staggers a zombie and creates distance |

### 4.3 Gameplay Effects

| Class | Base | Purpose |
|---|---|---|
| `UZomGameplayEffect` | `UGameplayEffect` | [Proposed] Shared effect base. Worth being direct about what this actually buys you: GameplayEffects are mostly data (duration policy, modifiers, tags), not code, so unlike `UZomGameplayAbility` there's little runtime logic to hoist here. Its real value is a single place to enforce naming/tag conventions (e.g., every Zom effect stamps a consistent `SetByCaller` tag scheme) so the four effects below stay consistent without a shared base actually *doing* much. Don't expect this class to save you the way the ability base does. |
| `UZomGE_Damage` : `UZomGameplayEffect` | [Design] Applied by melee/ranged hits, writes to the shared `Damage` meta-attribute (4.1) |
| `UZomGE_StaminaDrain` : `UZomGameplayEffect` | [Design] Sprint and heavy-action cost |
| `UZomGE_Infection` : `UZomGameplayEffect` | [Design] Shared by bite infection (persistent, cleared by Medicine) and Bloater gas infection (5s duration, extends rather than stacks). Needs a custom duration-extend helper, stock GAS duration-refresh policy resets rather than adds remaining time. |
| `UZomGE_Stagger` : `UZomGameplayEffect` | [Design] Applied by `UZomGA_Shove` and by heavy player hits |

### 4.4 Data & Tags

| Item | Type | Notes |
|---|---|---|
| `UZomAbilitySetData` | `UPrimaryDataAsset` | [Design] Grants an ability/effect set to a character on BeginPlay. The extensibility hook for combat. |
| `Zom.Status.Attacking`, `Zom.Status.Staggered`, `Zom.Status.Dodging` | Gameplay Tags | [Design] Persistent, live on the ASC's tag container. Drive animation and ability gating instead of bools. Renamed from `Zom.State.*`, "State" already means something else in this project (State Tree states, Section 5.2), `Status` is the GAS-conventional term and doesn't collide. |
| `Zom.Objective.Fetch.Complete`, `Zom.Objective.Repair.Complete`, `Zom.Objective.Defend.Complete`, `Zom.Objective.Boss.Complete`, `Zom.Objective.Extracted.Complete` | Gameplay Tags | [Design, confirmed] Extended to cover all five `EZomObjectiveStep` values. `UZomObjectiveSubsystem` sets both together in one function on step completion, never one without the other, that's what keeps them from drifting apart. The enum and the tags aren't actually duplicating the same job: the enum drives the subsystem's own internal sequencing and the checkpoint mapping (`EZomCheckpointID` comparison in `AZomGameMode::ChoosePlayerStart_Implementation`), the tags are the GAS-queryable mirror of that same progress, since GAS's own `ActivationRequiredTags`/effect tag requirements can't read a raw `uint8` enum at all. Mechanically: the subsystem resolves the player's ASC via `AZomPlayerCharacterBase::GetAbilitySystemComponent()` (4.5) and calls `AddLooseGameplayTag()` on the matching tag, that's the piece that was missing before, "enables GAS gating" only means something once a tag actually lands on an ASC somewhere. |
| `Zom.Perception.Sight.TargetSeen`, `Zom.Perception.Sight.TargetLost`, `Zom.Perception.Hearing.NoiseHeard`, `Zom.Perception.Damage.Taken` | Gameplay Tags | [Proposed] Transient, never touch a tag container, passed through `FStateTreeEvent` once via `SendStateTreeEvent` and gone. Raised by `AZomZombieAIController` (Section 5.5, revised), consumed by Event-triggered transitions on the crowd State Tree. `Damage.Taken` added post-hoc (not in the original draft) so a zombie can react to being shot/hit even from outside sight/hearing range - see 5.5. |

**Tag taxonomy, not just a flat list:** the three rows above split by *mechanism*, not just by which system happens to raise the tag. `Status` tags persist on the ASC and get queried repeatedly (ability gating, animation state). `Perception` tags are one-shot signals that exist for the instant `SendStateTreeEvent` fires, then nothing holds a reference to them. `Objective` tags are the GAS-queryable mirror described above. Keeping these as separate branches under one `Zom.` root, rather than flattening everything, is what lets a `Zom.Perception` hierarchical match catch both `Sight` and `Hearing` children without the two mechanisms bleeding into each other.

### 4.5 Shared Helpers on `AZomPlayerCharacterBase`

The concrete home for the cached-pointer pattern from Section 2, and where the "extensibility hook for combat" from 4.4 actually gets consumed:

| Method | Access | Purpose |
|---|---|---|
| `GetAbilitySystemComponent()` | `public`, `override` | [Proposed] Single implementation for every humanoid. Returns the cached pointer, never re-resolves it. |
| `InitializeAbilitySystem(AActor* OwnerActor, AActor* AvatarActor)` | `protected` | [Proposed] Caches the pointer and calls `ASC->InitAbilityActorInfo(OwnerActor, AvatarActor)`. Called once by each leaf class at the right point in its own lifecycle (Section 2). |
| `GrantAbilitySet(const UZomAbilitySetData* AbilitySetData)` | `protected` | [Proposed] Iterates the ability classes and starting effects on a `UZomAbilitySetData` asset and grants/applies them through the cached ASC. This is the actual implementation behind "grants an ability/effect set to a character on BeginPlay" from 4.4, previously that line described behavior without a home; now it has one. |
| `ApplyGameplayEffectToSelf(TSubclassOf<UGameplayEffect> EffectClass, float Level = 1.f)` | `protected` | [Proposed] Small wrapper around the `MakeOutgoingSpec`/`ApplyGameplayEffectSpecToSelf` boilerplate, since every subclass that applies a self-effect (stagger, stamina drain) would otherwise repeat it. |
| `GetHealth()` / `GetMaxHealth()` | `public`, `const` | [Proposed] Convenience getters that pull from whichever `UZomAttributeSetBase` instance the cached ASC has registered, via `ASC->GetAttributeSet<UZomAttributeSetBase>()`. HUD widgets and damage logic call these instead of caring where the AttributeSet lives. |

**One real risk to test for, not just a style note:** for the player, `GetHealth()` and any other ASC-dependent call are only safe *after* `InitializeAbilitySystem()` has run in `PossessedBy()`/`OnRep_PlayerState()`. If anything in `AZomPlayerCharacter::BeginPlay()` reads Health before possession completes, the cached pointer is still null. Zombies and the Boss don't have this problem, their `BeginPlay()` is also where they call `InitializeAbilitySystem()`, so ordering is self-contained. The player is the one place a null check actually matters.

### 4.6 Death Handling

The design doc already establishes that Boss phase transitions are "toggled by a Gameplay Tag off the AttributeSet's health-change delegate" (Section 6). Death should use the same delegate, one mechanism, two consumers, rather than inventing a second way to react to Health hitting zero.

| Item | Notes |
|---|---|
| `UZomAttributeSetBase::OnHealthChanged` (or the standard `FOnGameplayAttributeValueChange` delegate GAS already exposes) | [Proposed] `PostGameplayEffectExecute` broadcasts this after clamping Health. Both the Boss phase-tag logic and death detection subscribe to the same signal instead of each polling Health independently. |
| `AZomPlayerCharacterBase::HandleDeath()` | [Proposed] `virtual`, `protected`. Bound to the health-changed delegate, fires once Health reaches zero. Empty at the base, each subclass overrides it for consequences that are actor-level, not attribute-level, spawning `AZomToxicGasVolume` isn't something an AttributeSet should know how to do. |
| `AZomPlayerCharacter::HandleDeath()` override | [Proposed] Triggers the save/respawn flow from Section 11 |
| `AZomZombieBase::HandleDeath()` override | [Proposed] Returns itself to `UZomZombiePoolSubsystem` instead of being destroyed; if it's a Bloater, also spawns `AZomToxicGasVolume` first (Section 5.3) |
| `AZomBoss::HandleDeath()` override | [Proposed] Plays the death bark (Section 6), unlocks `AZomExtractionPoint` |

Same shape as `GetAbilitySystemComponent()` and the Damage-to-Health conversion: one shared trigger point on the base, subclass-specific consequences layered on top through a virtual override instead of three unrelated death-detection implementations.

---

## 5. Zombie AI System

### 5.1 Base Class & Data

| Item | Type | Notes |
|---|---|---|
| `AZomZombieBase` | Actor | See Section 3 |
| `UZombieTypeData` | `UPrimaryDataAsset` | [Design] Health, speed, per-sense detection radius, damage, attack cooldown. One asset per type below. New crowd types are content, not code. |
| `EZomZombieCategory` | `enum class : uint8` | [Proposed] `{ Crowd, Bloater, Boss }` — used by the pool subsystem to enforce the two separate density budgets (5-15 crowd, 1-2 Bloater) called out in the design doc. Not explicitly named there, but the doc requires two independently-tracked counters, and an enum is cleaner than a bool flag. |

### 5.2 Zombie Types (all `UZombieTypeData` instances, no C++ subclass)

| Type | Perception | Behavior |
|---|---|---|
| Regular — Walker | Balanced sight + hearing | Slow chase, low aggression |
| Regular — Runner | Same data family as Walker, differs only in Speed/Aggression values | Fast, aggressive chase |
| Auds | Hearing only, long range, very sensitive | Investigates sound aggressively, blind to pure stealth |
| Eyes | Sight only, long range, wide angle | Detects at range on sight, punishes exposed movement |
| Bloater | Balanced, short range, very slow | Tank, heavy attacks, death-triggers gas hazard |

Shared State Tree states: `Idle → Patrol → Investigate → Chase → Attack → Staggered → Dead`.

### 5.3 Bloater Death Hazard

| Class | Notes |
|---|---|
| `AZomToxicGasVolume` | [Design] Radial trigger volume, spawned at Bloater death, persists 3s, applies `UZomGE_Infection` on overlap (extends existing infection rather than stacking) |

### 5.4 Cross-Zombie Alerting — [Open, wanted, not yet scheduled]

Confirmed as a feature you want; the mechanism isn't decided yet, and this is explicitly a "later" item, not on the current build path. Not building any of it now. When you're ready to design it, the tagged events in `AZomZombieAIController` (5.5) are the natural payload to rebroadcast to nearby zombies, no new detection architecture needed, the plumbing already exists once 5.5 is built. Resist the urge to add a "just in case" broadcast delegate today; a delegate shaped around a guess at the design would likely need reworking once the real mechanism is chosen.

### 5.5 `AZomZombieAIController` [Revised — was `UZomZombieAIComponent`]

**Revision note:** the original draft of this section proposed a `UActorComponent` living on `AZomZombieBase` to own perception and raise State Tree events, with `UStateTreeComponent` living directly on the pawn alongside it. On explicit direction, this was rebuilt as a dedicated `AAIController` subclass instead, matching UE 5.8's own intended State Tree AI pattern: `UStateTreeAIComponent`/`UStateTreeAIComponentSchema` (in `GameplayStateTreeModule`) are explicitly documented as "designed to be run on an AIController" and guarantee the State Tree's bindings access to the controller (and, through it, the possessed pawn). Perception now lives on the controller too, since it's a "brain" concept, not a "body" one. `AZomZombieBase` no longer owns either piece directly, only `AIControllerClass = AZomZombieAIController::StaticClass()` and `AutoPossessAI = PlacedInWorldOrSpawned`.

| Item | Notes |
|---|---|
| `AZomZombieAIController` : `AAIController` | [Design, revised] Possesses `AZomZombieBase` via `AutoPossessAI`. Owns perception (Sight, Hearing, and Damage senses) and the crowd `UStateTreeAIComponent`. Event-driven off the perception delegate exclusively; the State Tree component itself is what ticks (not this controller directly). |

**What it owns and does:**

- Assigns a `UAIPerceptionComponent` subobject into `AAIController`'s own inherited `PerceptionComponent` pointer via `SetPerceptionComponent()` — that pointer already exists on the base engine class, a same-named member on the subclass would be a UHT shadowing error.
- Owns a `UStateTreeAIComponent` (not the plain `UStateTreeComponent` originally proposed) — the AI-specific subclass, using `UStateTreeAIComponentSchema` so the State Tree asset's bindings can reach this controller and its possessed pawn.
- **Three senses**, not two: Sight and Hearing are radius-based (`UAISenseConfig_Sight`/`UAISenseConfig_Hearing`, tuned per `UZombieTypeData`); **Damage** (`UAISenseConfig_Damage`) is new, added post-hoc, and works differently — it has no radius or affiliation filter at all (`UAISenseConfig_Damage` doesn't declare one, confirmed against the 5.8 header), it's purely an explicit `UAISense_Damage::ReportDamageEvent()` call. `AZomZombieBase` binds to its own `UZomZombieAttributeSet::OnDamageTaken` (Section 12's damage-indicator delegate, reused here) and reports the hit on every damage instance, so a zombie can react to being shot/hit even from outside sight/hearing range. The reported "sensed actor" on the resulting stimulus is the damage *instigator* (confirmed via `UAISense_Damage::RegisterWrappedEvent` — `RegisterStimulus(Event.Instigator, ...)` on the damaged actor's own listener), not the zombie itself, so the controller reacts to "who hit me," not "I got hit."
- `OnPossess(APawn* InPawn)` reads sight/hearing radii and sensitivity off the newly-possessed zombie's `UZombieTypeData` (via a getter on `AZomZombieBase`, the controller doesn't hold a second reference to the same asset) and calls `ConfigureForType()`, which in turn calls `UAIPerceptionComponent::ConfigureSense()` for `UAISenseConfig_Sight` and `UAISenseConfig_Hearing` (Damage needs no per-type configuration). This is the concrete implementation behind the Section 5.2 claim that Auds and Eyes are "the same component with one sense's config zeroed out."
- `ConfigureForType()` is re-callable, not just an `OnPossess`-time thing: `AZomZombieBase::InitializeForType()` calls back into it whenever `UZomZombiePoolSubsystem` reactivates a pooled zombie as a (possibly different) type, since the controller possesses once and is never re-spawned alongside pooling — pooled reactivation only hides/shows the pawn, so re-seeding perception on every activation has to be an explicit, separately-callable path.
- `PauseBrain()`/`ResumeBrain()` call the State Tree component's own `StopLogic()`/`StartLogic()` (confirmed present on `UStateTreeComponent` in `Components/StateTreeComponent.h`) — called by `UZomZombiePoolSubsystem` on release/reactivation so a hidden pooled zombie's brain doesn't keep running while deactivated.
- Subscribes to `OnTargetPerceptionUpdated`. On a successful sense, raises a tagged State Tree event via `UStateTreeComponent::SendStateTreeEvent(Tag, Payload, Origin)` (confirmed against the UE 5.8 API), rather than caching state for the tree to poll. The tag identifies *which* sense fired, so the tree doesn't need a separate query to find out:

| Gameplay Tag | Raised when |
|---|---|
| `Zom.Perception.Sight.TargetSeen` | Sight sense successfully senses the player |
| `Zom.Perception.Sight.TargetLost` | Sight sense loses the player |
| `Zom.Perception.Hearing.NoiseHeard` | Hearing sense registers a stimulus (footsteps, gunfire) |
| `Zom.Perception.Damage.Taken` | The zombie takes damage (via `UZomZombieAttributeSet::OnDamageTaken` → `ReportDamageEvent`), regardless of sight/hearing range |

  (Added to the tag list in 4.4, same table as `Zom.Status.*`/`Zom.Objective.*`.)

- Payload struct, `FZomPerceptionEventPayload` [Proposed], carries `TWeakObjectPtr<AActor> SensedActor` and `FVector Location`, wrapped via `FConstStructView::Make(Payload)` when calling `SendStateTreeEvent`. On the State Tree asset itself (content, not C++), a transition's Trigger Type is set to **Event** with a Required Event Tag matching one of the tags above, that's the enter condition you were asking about.
- Still exposes `HasValidTarget()`, `GetCurrentTarget()`, `GetLastKnownTargetLocation()` as plain getters, caching the player as `TWeakObjectPtr<ACharacter>` per the Section 5.2 performance note. These serve a different job than the events above: **events drive transitions** (something happened, move states now), **these getters back property bindings for tasks in an already-active state** (Chase needs the target's current location every tick it's active, not just a one-time notification). Both patterns coexist in State Tree by design, don't conflate them.

**Dropped from the previous draft:** `EZomPerceptionStimulus`/`GetLastStimulusType()`. The event tag itself now answers "which sense fired", a separate polling enum for the same question would just be a second way to ask something the tag already tells you.

**Boss doesn't get this controller.** [Proposed, flag if wrong] The encounter is a single gated, always-aware fight, not a detection problem, `UZomBossData` has no per-sense radius fields to configure against, and the design doc already treats Boss AI as bespoke rather than data-driven. `AZomBoss` keeps its bespoke `UStateTreeComponent` directly on the pawn (Section 6), not this AIController pattern — not revisited as part of this section's rebuild, since the ask was scoped to the crowd zombie AI class specifically. If the Boss needs to notice the player at all before the fight starts, a trigger volume is simpler than standing up a full perception setup for a one-off encounter.

---

## 6. Boss

| Item | Type | Notes |
|---|---|---|
| `AZomBoss` | Actor | [Design] See Section 3 |
| `UZomBossData` | `UPrimaryDataAsset` | [Design] Health, damage, phase threshold, dialogue audio cues. Tuning only, behavior stays in code, unlike crowd types, the Boss is intentionally not fully data-driven. |
| Boss State Tree asset | Content | [Design] Separate from the shared crowd tree |
| `UZomSubtitleWidget` | `UUserWidget` | [Design] Displays subtitle text for Boss barks, triggered off the same Audio Component playback, not a separate dialogue system |

Boss specifics: fast 2-3 hit melee combo plus a ranged option, one health-threshold phase transition (~50%) toggled by a Gameplay Tag off the AttributeSet's health-change delegate, three bark minimum (encounter start, mid-fight taunt, death line), placed as a single instance gating Extraction. Not pooled, not difficulty-scaled.

---

## 7. Inventory & Items

| Class | Notes |
|---|---|
| `UZomInventoryComponent` | [Design] Attached to `AZomPlayerCharacter` |
| `UZomItemData` | [Design] `UPrimaryDataAsset`, covers both weapons and consumables |

### Weapon list — [Design, confirmed]

The design doc left this open (Section 16: "specific weapon list... still undefined"). Confirmed set, mapped to the abilities in 4.2:

| Weapon | Ability slot |
|---|---|
| Machete | Melee light/heavy |
| Fire Axe | Melee heavy alternate |
| Pistol | Ranged |
| Pump Shotgun | Ranged alternate |
| Crossbow | Ranged, silent (relevant against Auds) |

### Consumables — [Proposed]

| Item | Function |
|---|---|
| Medicine | Clears `UZomGE_Infection` |
| Pistol Ammo | Ranged resource |
| Shotgun Shells | Ranged resource |
| Bandage | Direct Health restore, no infection interaction |

Build `UZomItemData` assets against this list.

---

## 8. Objectives

| Item | Type | Notes |
|---|---|---|
| `UZomObjectiveSubsystem` | `UGameInstanceSubsystem` | [Design] Owns the objective state machine, broadcasts delegates on state change, and mirrors progress onto the player's ASC as Gameplay Tags (see 4.4, `Zom.Objective.*.Complete`) so abilities/effects can gate on it. Persists across `OpenLevel` calls, which is required since checkpoints reload the level. |
| `EZomObjectiveStep` | `enum class : uint8` | [Proposed] `{ Fetch, Repair, Defend, Boss, Extracted }` |
| `AZomFetchItem` | Actor | [Proposed] See Section 2 |
| `AZomRepairTarget` | Actor | [Proposed] See Section 2 |
| `AZomDefendVolume` | Actor | [Proposed] See Section 2 |
| `AZomExtractionPoint` | Actor | [Proposed] Locked until objective chain completes |

**[Future] Objective plugin, not yet built.** Everything in this table is the working interim implementation, build it as documented. The intent going forward is to extract this into a standalone, reusable Objective plugin (generic, tag-identified, no dependency on GAS or on anything "Zom"), for eventual free release on FAB. That plugin doesn't exist yet, deliberately not scheduled in Section 16's build order, this note exists so the intent isn't lost between now and whenever that work starts. When it does happen, the rough shape discussed was: `UObjectiveDefinition`/`UObjectiveChainData` as data assets a game authors, a generic `UObjectiveSubsystem` with tag-based `CompleteObjective(FGameplayTag)`/`FailObjective(FGameplayTag)` and dependency-driven activation, and `EZomObjectiveStep` retired in favor of Zom authoring its five steps as tagged `UObjectiveDefinition` content instead of a hardcoded enum.

**Resolved from the earlier open question in 4.4:** the objective tags now cover all five steps and are confirmed, not just proposed, see 4.4 for the mechanism.

---

## 9. Zombie Spawning & Pooling

| Item | Type | Notes |
|---|---|---|
| `UZomZombiePoolSubsystem` | `UWorldSubsystem` | [Proposed type] Design doc names the class but never specifies subsystem lifetime. `UWorldSubsystem` is correct here, not `UGameInstanceSubsystem`, because it owns actual pooled actors that live in one world/level, unlike the Objective and Save subsystems, which track state that must survive a level reload. Pre-spawns and recycles pooled zombie actors via `SetActorHiddenInGame`/`SetActorEnableCollision`, never `SpawnActor()`/`Destroy()` at runtime. Tracks Crowd and Bloater budgets separately via `EZomZombieCategory`. |
| `UZomZombieSpawnDirector` | Plain `UObject`, owned by the pool subsystem | [Design] Decides which pooled zombie to activate, where, and of which type. Reads `UZomDifficultyData` for the active tier. |

Pool sizing: crowd pool sized to the 15-zombie ceiling plus headroom (design doc suggests 20), separate small Bloater pool capped at 2. Boss is a single placed instance, not drawn from this pool.

---

## 10. Difficulty

| Class | Notes |
|---|---|
| `UZomDifficultyData` | [Design] `UPrimaryDataAsset`, one asset per tier, read by `UZomZombieSpawnDirector`. A new tier is a new asset, not new code. Explicitly does **not** scale the Bloater cap or the Boss encounter, only crowd density/cadence. |

---

## 11. Save & Checkpoint

| Item | Type | Notes |
|---|---|---|
| `UZomSaveGame` | `USaveGame` | [Design] Checkpoint ID, player attribute snapshot, objective step, resource/inventory flags |
| `AZomCheckpoint` | `APlayerStart` | [Design] Carries `EZomCheckpointID` |
| `EZomCheckpointID` | `enum class : uint8` | [Design] `{ Entry, PostFetch, PostRepair, PreBoss }` |
| `AZomGameMode::ChoosePlayerStart_Implementation` | Override | [Design] Selects the `AZomCheckpoint` matching the loaded save |

Restore flow (identical path for death and cold relaunch): load save slot on `InitGame` → `ChoosePlayerStart_Implementation` finds the matching checkpoint → saved Health/Stamina reapplied via a Gameplay Effect, not a direct attribute write, to stay consistent with GAS usage elsewhere → `UZomObjectiveSubsystem` resumes from the saved step, read explicitly every time rather than trusting in-memory state.

**Deliberately not persisted:** zombie kill state. A level reload recreates every pooled actor from default state at zero cost; per-zombie persistence would solve a problem the architecture already solves for free.

---

## 12. UI / HUD

| Class | Notes |
|---|---|
| `UZomHUDWidget` | [Proposed] Root widget, composes the rest |
| `UZomObjectiveTrackerWidget` | [Design] Bound to `UZomObjectiveSubsystem` delegates |
| `UZomAbilityBarWidget` | [Design] Bound to ASC cooldown delegates |
| `UZomInventoryWidget` | [Design] Grid view over `UZomInventoryComponent` |
| `UZomDamageIndicatorWidget` | [Design] Directional, since the third-person camera doesn't always show the threat |
| `UZomSubtitleWidget` | [Design] See Section 6 |

All bound to delegates, no per-frame polling, per project performance rules.

**Status note:** `AZomHUD : public AHUD` already exists (`Source/Zom/Game/ZomHUD.h/.cpp`) as a bare `DrawHUD()` override with no widgets. None of the `UUserWidget` classes above exist yet.

---

## 13. Accessibility

| Class | Notes |
|---|---|
| `UZomAccessibilitySettings` | [Design, detail Open] Audio-cue captions, colorblind-safe HUD, screen-effects toggle. The design doc names the feature set but not the base class. Recommend a `UGameUserSettings` subclass so it persists through the standard engine settings save path rather than piggybacking on `UZomSaveGame`, which is checkpoint/run state, a different lifecycle than user preferences. |

---

## 14. Subsystem Lifetime Summary

| Subsystem | Base | Survives level reload? |
|---|---|---|
| `UZomObjectiveSubsystem` | `UGameInstanceSubsystem` | Yes, required |
| `UZomZombiePoolSubsystem` | `UWorldSubsystem` | No, recreated per level, by design |
| `UZomAccessibilitySettings` | `UGameUserSettings` | Yes, via engine config save |

---

## 15. Cross-Reference to Performance Rules

Already baked into the design, called out here so it isn't lost during implementation:

- `AZomPlayerCharacterBase`, `AZomZombieBase`, `AZomBoss` disable Tick by default; State Tree and GAS event-driven logic cover behavior without polling.
- `AZomZombieAIController` doesn't tick itself, target caching is driven off `OnTargetPerceptionUpdated`, not a per-frame distance scan; the State Tree component it owns is what actually ticks.
- `AZomPlayerState` and the AI actors each own their own ASC instance; no cross-actor lookups needed to find "the" ASC, `GetAbilitySystemComponent()` on any humanoid resolves it locally.
- Zombie pool never calls `SpawnActor()`/`Destroy()` at runtime.
- Player/zombie cross-references use `TWeakObjectPtr` to avoid GC circular holds.
- Detection and attack range checks use `DistSquared()`, never `Dist()`.
- Gameplay Tags (FName-backed) drive state instead of string or bool comparisons.
- `EZomZombieCategory`, `EZomObjectiveStep`, `EZomCheckpointID` are all `uint8` enums per project convention.

---

## 16. Suggested Build Order

Matches the design doc's milestone sequence. **Updated per the Section 17 audit** — a GAS module/dependency step is now called out explicitly as step 0, since it's a hard prerequisite that wasn't previously listed as its own line item.

0. Add `GameplayAbilities`/`GameplayTasks`/`GameplayTags` module dependencies to `Zom.Build.cs`; create `Config/DefaultGameplayTags.ini` (or native `FNativeGameplayTag` registration) for the `Zom.*` tag root. Nothing in step 2 onward compiles without this.
1. Repurpose `AZomPlayerCharacterBase` into the real shared humanoid base (ASC pointer, `IAbilitySystemInterface`, `InitializeAbilitySystem`/`GrantAbilitySet`/`ApplyGameplayEffectToSelf`/`GetHealth`/`HandleDeath`); make `AZomPlayerCharacter` inherit from it instead of `ACharacter` directly. `AZomPlayerState` core movement/camera pieces already exist and stay as-is; add ASC/AttributeSet ownership to `AZomPlayerState`. Enhanced Input binding still needs wiring on `AZomPlayerCharacter`/`AZomPlayerController` (module is linked, no bindings exist yet).
2. GAS integration: `UZomAttributeSetBase`, `UZomPlayerAttributeSet`, `UZomZombieAttributeSet`, `UZomGameplayAbility`/`UZomGameplayEffect` bases, base abilities.
3. `AZomZombieBase`, `AZomZombieAIController` (revised from a `UActorComponent` to an `AAIController` — see Section 5.5) + Regular type (Walker/Runner) on State Tree.
4. Auds, Eyes, Bloater + `AZomToxicGasVolume`, `UZomZombiePoolSubsystem`, `UZomZombieSpawnDirector`.
5. `UZomInventoryComponent`, `UZomItemData` (resolve the weapon list first, Section 7).
6. `UZomObjectiveSubsystem` + the four objective actors.
7. `AZomBoss`, `UZomBossData`, subtitle widget.
8. `UZomDifficultyData` tiers.
9. `UZomSaveGame`, `AZomCheckpoint`, finish `AZomGameMode` (`ChoosePlayerStart_Implementation` + save-load in `InitGame`/`PostLogin` — constructor pawn/state/controller class assignments already done).
10. UI pass, then `UZomAccessibilitySettings`.
11. Art/audio, performance profiling pass, extensibility documentation pass.

**Not scheduled above, deliberately:** extracting Section 8's Objective system into the standalone plugin, and cross-zombie alerting (5.4). Step 6 still gets built as `UZomObjectiveSubsystem` + `EZomObjectiveStep`, the interim implementation, when the plugin work actually starts it's a separate, later effort, not a line item in this list.

---

## 17. Current Build Status (audit as of 2026-08-24; C++ layer completed same day per `agile-percolating-tiger.md`)

Legend: ✅ built & compiles · 🟡 partial · ⬜ not built · 🔶 requires editor-side content (data assets, State Tree assets, Blueprint children, Widget Blueprints, Input Action/Mapping Context assets) before it's usable in-game, even though the C++ compiles clean.

**Headline: every class/enum/struct/subsystem/native tag in Sections 2–13 now exists in `Source/Zom/` and the project compiles clean (`ZomEditor Win64 Development`) after all 12 phases.** What remains is entirely editor-side content authoring (data asset instances, State Tree assets, Animation/Widget Blueprints, Input Action/Mapping Context assets, GA/GE Blueprint children with tuned numbers, level placement of objective/checkpoint actors) — none of it C++, all of it necessary before the game is actually playable end to end. New source folders: `Abilities/` (+ `AttributeSets/`, `GA/`, `Effects/`), `AI/` (+ `Components/`, `Enums/`), `Items/`, `Objectives/`, `UI/`, `Accessibility/`.

### Corrections made during implementation (the doc's assumptions vs. what UE 5.8 actually required)

- **`GameplayAbilities` needed a `.uproject` Plugins entry**, contrary to this doc's Phase-0 assumption — it's a plugin in 5.8 (`Engine/Plugins/Runtime/GameplayAbilities`), not a bare engine module. Added; `GameplayTasks` is a bare module, no plugin entry needed for it.
- **`UZomGE_Infection`'s custom duration-extend need was outdated.** UE 5.8's `EGameplayEffectStackingDurationPolicy` already has a native `ExtendDuration` value — no custom `UGameplayEffectComponent` was needed. `UZomGE_Infection` leaves `DurationPolicy` unset in C++ specifically so two Blueprint children can pick different policies (`GE_Infection_Bite` = Infinite, `GE_Infection_Gas` = HasDuration + `ExtendDuration` stacking).
- **`AZomPlayerState` gets a single `UZomPlayerAttributeSet` subobject, not two separate instances.** The doc's literal "owns `UZomAttributeSetBase` and `UZomPlayerAttributeSet` as real subobjects" would register two ASC-attached instances that both `IsA(UZomAttributeSetBase)`, making GAS's modifier-target resolution ambiguous. `UZomPlayerAttributeSet` already inherits everything from the base, so one instance covers both.
- **`WalkToRunSpeedThreshold` was added then removed** during the Gait work preceding this build pass — `WalkSpeed` itself is the Walk/Run boundary now, per explicit correction.
- **New classes/tags not in the doc's original tables**, added where the doc named a requirement but not the mechanism: `UZomGE_RestoreFromSave` (Section 11's "via a Gameplay Effect, not a direct attribute write"), `FOnZomDamageTaken` delegate on `UZomAttributeSetBase` (Section 12's "directional" damage indicator needs a data source), `TAG_Zom_Boss_Phase2` (Section 6's unnamed phase-transition tag), `TAG_Zom_SetByCaller_Magnitude`/`Duration`/`RestoreHealth`/`RestoreStamina` (Section 4.3's "consistent SetByCaller scheme").
- **`AZomPlayerCharacterBase` naming stays "Player"-scoped** despite being the shared base for `AZomZombieBase`/`AZomBoss` too — carried forward as flagged, not resolved.
- **`BP_ZomGameMode` vs. actual `BP_GameMode`** — per the "existing code wins" precedent, treat `BP_GameMode` as correct; the C++ base works regardless of the Blueprint asset's name.
- **`UZomZombieAIComponent` rebuilt as `AZomZombieAIController` (post-hoc, on explicit direction).** Perception and the State Tree moved off a `UActorComponent` on the pawn and onto a dedicated `AAIController` subclass, matching UE 5.8's own `UStateTreeAIComponent`/`UStateTreeAIComponentSchema` pattern (in `GameplayStateTreeModule`), which is explicitly documented as designed to run on an AIController. Also surfaced that `AAIController` already declares a `PerceptionComponent` member — the subclass assigns into it via `SetPerceptionComponent()` rather than redeclaring one (a UHT shadowing error). See Section 5.5.

### Section 2/3 — Core Framework

| Item | Status | Notes |
|---|---|---|
| `AZomPlayerCharacterBase` | ✅ | Repurposed: `IAbilitySystemInterface`, cached ASC pointer, `InitializeAbilitySystem`/`GrantAbilitySet`/`ApplyGameplayEffectToSelf`/`GetHealth`/`GetMaxHealth`/`HandleDeath`, health-changed delegate binding. Tick disabled by default. |
| `AZomPlayerCharacter` | ✅ | Reparented onto the base. Inventory component added. Full Enhanced Input wiring (Move/Look/Jump/Sprint/Crouch/six ability inputs) — 🔶 needs `IA_*`/`IMC_Default` assets under `Content/_Game/Input/`, none exist yet. |
| `AZomZombieBase` | ✅ | Own ASC + `UZomZombieAttributeSet`. `AIControllerClass = AZomZombieAIController`, `AutoPossessAI = PlacedInWorldOrSpawned` (revised, see Section 5.5 — perception/State Tree moved off the pawn onto the controller). `InitializeForType()` re-seeds attributes on `BeginPlay` and pooled reactivation, and nudges the possessing controller to reconfigure perception. 🔶 needs a crowd State Tree asset, `BP_ZombieBase` wrapper. |
| `AZomZombieAIController` | ✅ | New (Section 5.5, revised from `UZomZombieAIComponent`). Owns `UAIPerceptionComponent` (via `AAIController`'s inherited slot, Sight + Hearing + Damage senses) + `UStateTreeAIComponent`, raises `Zom.Perception.*` events (incl. `Damage.Taken`, added post-hoc), `PauseBrain()`/`ResumeBrain()` wired into pool release/reactivation. `AZomZombieBase` reports damage via `UAISense_Damage::ReportDamageEvent()` off its own `OnDamageTaken` delegate. |
| `AZomBoss` | ✅ | Own ASC reusing `UZomZombieAttributeSet`, bespoke `UStateTreeComponent` directly on the pawn (not the `AZomZombieAIController` pattern — explicitly not revisited, per the doc's own flag), no perception component, phase-2 tag toggle, dialogue `UAudioComponent`. 🔶 needs a Boss State Tree asset, `BP_Boss`, `UZomBossData` instance. |
| `AZomPlayerState` | ✅ | Owns ASC + single `UZomPlayerAttributeSet` (see correction above). |
| `AZomCheckpoint` | ✅ | `APlayerStart` + `EZomCheckpointID`. 🔶 needs instances placed in the level(s). |
| `AZomGameMode` | ✅ | `ChoosePlayerStart_Implementation`, `InitGame` (loads save), `PostLogin` (reapplies Health/Stamina via `UZomGE_RestoreFromSave`, resumes objective step) all implemented. |
| `BP_ZomGameMode` | 🟡 | Still exists as `BP_GameMode` — naming divergence noted above, not renamed. |
| `AZomToxicGasVolume` | ✅ | Sphere overlap, 3s lifespan, applies infection via `SetByCaller`. |
| `AZomFetchItem` / `AZomRepairTarget` / `AZomDefendVolume` / `AZomExtractionPoint` | ✅ | All four built; `AZomDefendVolume` spawns a wave via the pool/spawn-director and polls clearance on a 1s timer (not per-frame); `AZomExtractionPoint` listens for `Zom.Objective.Boss.Complete`. 🔶 needs level placement + mesh/VFX dressing. |
| `AZomPlayerController` | ✅ | Adds `DefaultMappingContext` via the Enhanced Input subsystem in `SetupInputComponent`. |

### Section 4 — Combat/GAS: ✅ complete
`UZomAttributeSetBase`/`UZomPlayerAttributeSet`/`UZomZombieAttributeSet` (incl. `AttackDamage`, built per explicit confirmation), `UZomGameplayAbility`/`UZomGameplayEffect` bases, all six abilities (`LightAttack`/`HeavyAttack`/`RangedShoot`/`Reload` are structural shells pending weapon/montage content; `Dodge` is timer-driven; `Shove` has real sphere-sweep+stagger+launch logic), all four effects, `UZomAbilitySetData` + `GrantAbilitySet()`, all native tags. 🔶 GA/GE Blueprint children for tuned numbers, `UZomAbilitySetData` asset instance(s).

### Section 5 — Zombie AI: ✅ complete
`UZombieTypeData` (+ `EZomZombieCategory`), `AZomZombieAIController` (revised from a component to an `AAIController` — perception config, event-raising, `ConfigureForType`/`PauseBrain`/`ResumeBrain` re-callable for pooling), `FZomPerceptionEventPayload`. 🔶 crowd State Tree asset, `UZombieTypeData` instances for all five types. Cross-zombie alerting (5.4) remains explicitly unbuilt, confirmed open.

### Section 6 — Boss: ✅ complete (`AZomBoss`, `UZomBossData`, `UZomSubtitleWidget`)

### Section 7 — Inventory: ✅ complete (`UZomItemData`, `UZomInventoryComponent`). 🔶 item data asset instances, weapon meshes.

### Section 8 — Objectives: ✅ complete (`UZomObjectiveSubsystem`, `EZomObjectiveStep`, all four actors). Plugin extraction still explicitly out of scope.

### Section 9 — Spawning/Pooling: ✅ complete (`UZomZombiePoolSubsystem`, `UZomZombieSpawnDirector`, owned relationship wired via `Initialize()`).

### Section 10 — Difficulty: ✅ complete (`UZomDifficultyData`, spawn director reads `TargetActiveCrowdCount`). 🔶 per-tier asset instances.

### Section 11 — Save: ✅ complete (`UZomSaveGame`, `EZomCheckpointID`, full `AZomGameMode` restore flow).

### Section 12 — UI: ✅ complete (all five widgets + `UZomSubtitleWidget`, `AZomHUD` creates/adds the root widget). 🔶 every `WBP_*` Widget Blueprint — the largest remaining content lift.

### Section 13 — Accessibility: ✅ complete (`UZomAccessibilitySettings : UGameUserSettings`).

### Undocumented code present (not in this document, kept for context)

- `UZomCharacterMovementComponent` — full ALS/Lyra-style locomotion (`Gait`/`RotationMode`/`Stance`, custom `PhysCustom`, landing detection). Substantial real work.
- `ZomCharacterEnums.h` — `EGait`, `EMovementDirection`, `ECharacterMovementMode`, `EMovementState`, `ERotationMode`, `EStance`.
- `UZomAnimInstanceBase` — custom anim instance base.
- `AZomGameState`, `AZomGameSession`, `UZomGameInstance`, `UZomCheatManager` — constructor-only engine-class stubs.
- `ZomLogChannels.h/.cpp` — custom log categories.
- `Content/_Game/Characters/Player/Mutable/` — Customizable Object character system.
- Plugin stack: Mutable, SmartObjects, MotionWarping, PoseSearch, StateTree/GameplayStateTree, Chooser, etc., now joined by `GameplayAbilities` (added in Section 17's build pass).
- `DefaultGame.ini` still points `GameDefaultMap` at the stock `/Engine/Maps/Templates/OpenWorld` template map.

---

*Prepared for the Zom sample project, Unreal Engine 5.8, C++-first implementation. Companion to `Zom_Design_Document.md`.*
