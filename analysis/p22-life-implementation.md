# P22-LIFE lifecycle contract

Agent #3, feature/p22-life, starting/inspected master
`49ff7dc12710cd645dc2224f9879b829cf055308`, 2026-11-11.
[Plan](p22-life-plan.md). Source implementation and native endpoint-double
verification are separate from application execution and desktop acceptance.
No other worktree was modified; research/p22 and Agent #2's d01face contracts
were inspected through Git. Application loading remains disabled.

## Original boundaries and evidence

Client fingerprint:
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
Research index was used before address research. Existing fingerprinted exports
and instructions were read, not executed. Local evidence root is
`/home/control/Documents/Projects/H&D2 Decomp 4/analysis/work`:

- `phase2-owner-20261103/exports/00698f80.{c,json}`: offline SAME live squad
  extraction precedes FPV, light restore, REVERSE presentation modules, dubbing,
  REVERSE authority modules, HUD, focus query, collision retirement, definition
  arrays0097B8A4 then0097B89C, status1→6, process006DF390(0,true), mission text,
  first navigation dataset runtime links, Car particle pool and Human faces.
- `mission-world-20261103/actor-base-lifetime/0069a3d0.{c,json}` and
  `mission-unload-order.asm`: status1 virtual08 finish BEFORE the loaded14 guard;
  CC/C8=-1, scene EC cleanup006E9B50, ASCENDING authority then presentation +18,
  REVERSE game48 model references, tables00523E10, engine Scene+F8 reset,
  resource6A release, loaded14=0 LAST. This is distinct from +20 finish.
- `mission-world-20261103/loader-module-composition/006ca780.{c,json}`:
  scratch/name-map owners retire before006CADE0 on both success and ordinary
  failure. Scene2's result is tested AFTER actor/sound/check loading. Tree
  allocation006EE570 occurs late; loaded14 publishes only after required effects.
- The paired006989B0 export has false-return graphics restoration; status5
  publishes BEFORE prepared-squad spawning, dubbing/text/music and the final
  loading-clock/context steps. An exception after status5 is still a partial
  activation, not a complete session. Original SEH/allocator ABI is not claimed.
- IMPORTANT address correction: actual bytes006987E0 are `MOV EAX,1; RET`.
  It is NOT destruction.00698810 is the deleting wrapper;00698830 clears queues,
  deletes HUD/FPV/text, calls0069A3D0 at006988D9, releases scene00699270, retracts
  current008AE7E4, then destroys tables/remaining allocations. Retained disassembly
  is `analysis/work/p22-life-20261111/destruction.asm`. No new credit is assigned to
  006987E0;00698830 retains its existing unloaded partial claim. The typed composition retires live World
  consumers before Parent destruction, a deliberate source lifetime guard.

Existing detailed owners: [loading](mission-loading-integration.md),
[activation](session-activation-integration.md),
[World](containing-world-loading.md), [modules](module-loading-lifetimes.md),
[blast](blast-wave-process.md). Research requirements remain on read-only
research/p22 under analysis/p2.2-critical-path, including blocker LIFE and its
reverse dependency into OWN/REPEAT.

## Concrete composition for P22-OWN

`host/game_session_lifecycle.h::GameSessionLifecycle<Controller>` owns ONE actual
GameMissionLoadingSession, GameSessionActivation, GameBlastWaves and
GameUiPresentation. It borrows the existing SessionOwner, Startup, actual
SessionActivationState, accepted request, mutable live squad, catalogue/suffix,
controller, engine Scene, graphics and clock. Existing constructors validate
those SAME identities. A new graph refuses to adopt another World's loaded14.
There is no replacement roster, readiness flag, clock or process resource list.
The only new state records entered/completed/retired ownership prefixes.

Construction unwinds unique members in reverse on exception. The parent has
its existing acquired-prefix construction guard; no load or activation occurs
in a constructor. `activate()` explicitly invokes the actual existing loader
and activation policy. A false/throw result retires that attempt. The original
exception is rethrown; a secondary cleanup exception remains available through
`rollback_failure()`. Graphics restoration also runs after a throwing setup
setter, preserves the primary exception and exposes a secondary restore error
through `activation_flag_failure()`. A missing graphics receiver never succeeds.

`finish()`, `unload()` and `close()` are separate typed operations. Finish retains
loaded14/file resources; unload performs real +18/Scene reset and clears loaded14
last; close destroys adapters/World before the caller may reset Parent. Status5
repeated activation returnsfalse without tearing down the existing active graph.
The same lifecycle permits repeated finish/unload/close; a NEW activation attempt
uses a new lifecycle after explicit close, with the same still-live Parent or a
new correctly constructed Parent. The application owns native status transitions.
A lifecycle accepts status1 only after its own completed activation; a fabricated
status1 over a cold graph is rejected. Repeated status1 finish after an already
consumed finish maps to6 without extracting removed Humans again.

P22-OWN must stop frame scheduling and destroy any frame coordinator borrowing
`world()` before close, then close LIFE before resetting SessionOwner/engine.
`world()`, `presentation()` and `blast_waves()` expose the actual owners needed by
FRAME/RENDER. `bind_presentation(GameUiRenderStages&)` owns the concrete existing
UI Scene callback adapter; real pre-scene/debug render receivers are mandatory.
The small GameScene bind/unbind/suspend seam lends that adapter to the existing
engine callback. Retirement disables the engine callback BEFORE destroying the
adapter, pages, Humans, HUD or scene; later binding reinstalls a suspended callback.
Callback errors are consumed once when delivered, so a previous render/loading
exception cannot repeatedly poison subsequent unload/reset/re-entry on the SAME
parent Scene. No successful empty render callback or application activation was added.

`make_overlay_page()` owns real PageOwner wrappers from Startup's process overlay.
Their unregister operation runs before row/page destruction. Session finish does
not close the process overlay or its editfont texture. Mission HUD/subtitle fonts
retain their independent actual owners and are retired through HUD/text cleanup.
External process pages remain their producers' responsibility; LIFE creates no
second registry and cannot adopt raw borrowed pages.

## Prefix and cleanup rules

`SessionActivationProgress` records entry BEFORE mutating calls and completion
only after return: definition/emitter initialization, lights, faces, each actual
registry slot on both activation passes, HUD, FPV, prepared spawning, dubbing and
mission text. It does not change any native status/loaded publication point.

Normal successful finish follows00698F80's exact receiver order above. Rollback
calls only entered/required cleanup receivers. GEffects is the important
cross-pass exception: authority activation loads birds but presentation finish
owns their retirement. Authority slot1 therefore records a REQUIRED presentation
cleanup without pretending presentation activation was entered/completed. Cold
GEffects construction now starts with its source finish guard already consumed;
it cannot invoke activated finish in a cold constructor/destructor tail.

The live squad is snapshotted before Human removal when spawning was entered.
Snapshot is attempted once, including on an exception, so a retry cannot overwrite
it from removed Humans. A snapshot error is preserved while ownership retirement
continues. Successful receiver retirements are consumed once; a failed mandatory
receiver stops dependent retirement and leaves the graph alive for explicit retry.
No tree/definition/emitter/face owner is freed under unfinished users. These are
source exception/idempotence guards, not claims of native SEH equivalence.

Script004270D0 can bind real contexts BEFORE native loaded10 publication. Its
source activation-start guard now makes00427B80/004285D0 retire that partial VM
prefix. Contexts exit/destroy before EC and retained frames go away. Scene2's
captured activation pass can include later unentered actors. Each actual actor
now carries a source two-bit entered-pass guard, including prepared Humans
activated after the module pass. An unsupported family (for example Dog) is
rejected before entry is recorded, so rollback can destroy its cold owner
without invoking a nonexistent positive deactivation receiver. This adds no Dog
simulation or activation implementation. For Humans, retirement skips
only a Human with NO published loading owner; a nonnull missing/foreign lifetime
still throws. Existing B0 retirement clears borrowed movement paths before B0
and inline DD0 destruction; Human/Guard/Area mechanics were not modified.

`GameSessionTransientFinish` is bound concretely to existing GameBlastWaves and
Startup's SAME008AED48 process, NOT proximity008AED54. Real process clear releases
retained node frames before recursive nodes and record hit-actor storage, in
reverse record order. The ordinary finish tail preserves collision, definition,
blast, text, navigation, emitter and face order. File retirement consults the
actual completed tail to avoid calling those receivers a second time.

`SessionFileProgress::dirty` covers unpublished partial loads after prior unload
and before resource/cache/scene mutation. Early006CA780 failure still executes
its existing cold module loading tail; rollback then uses FILE retirement, not
an indiscriminate fully activated finish. Both +18 passes use fresh ascending
registration reads. Successful slots/stages are consumed once on retry. Any
mandatory failure prevents parent Scene reset and loaded14 clearing.

Source file tails retire remaining runtime/Physics/item/model cache/entry and
frameEC sidecars after actual +18 actors/VM cleanup, before tree and parent reset.
Cold source-owned objects can exist before their activation module was entered.
Collision descriptors retire before the Human sweep unbinds; buckets detach
before actual tree destruction. Camera previous/main/target references retire
through the actual camera receiver; source look-target/sector/hit borrowers are
also cleared. Particle100C and mission-effects0089A8F4 loans, Car loading Scene,
reverse animated model references, callback/page loans and retained EC frames
are retracted before parent Scene reset. Navigation users are gone before table
storage is cleared. Partial-load process tails use actual definition/blast/text/
navigation/Car pool/face cleanup; no empty substitute finalizers exist.

`close()` propagates mandatory failures and keeps the graph for diagnosis/retry.
The destructor reports a remaining exception to stderr and lets existing guarded
RAII owners retire while Parent/Startup are still alive. That fallback is not a
successful close claim; callers needing a result must call explicit close.

## External dependencies and acceptance

LIFE does not supply Human simulation, integrity producers, HUD/FPV simulation,
FRAME/RENDER, rendering or successful GameApplicationState loading. Agent #1's
Human/Guard code and Agent #2's worktree remain untouched.

Agent #2's current contracts explicitly leave populated roll/signal retirement
and required Human/order/seat/focus producers open. If such a positive external
receiver is reached, cleanup propagates its real error and retains dependent
owners; LIFE does not substitute success. UI callbacks and pages themselves are
bound/retired concretely here, and its existing discard/actor borrower policies
remain compatible. Completion of that UI worker belongs to P22-UI, not LIFE.

Application activation/loaded failure/Main-return/re-entry, real engine behavior,
rendered mission frame and user-operated desktop acceptance remain pending.
Native fixtures use explicit engine/archive/graphics/clock/actor endpoint doubles;
i386 fixtures compile actual host owners but are never executed. No original
EXE/DLL, Windows fixture or Proton is run. Verification hashes and limitations
are recorded in the checkpoint receipt. No new complete-function credit or
reconstruction denominator change is claimed.
