# Human and VM world composition — source checkpoint, 2026-11-10

Continue the full first P2.2 checkbox. Application loading remains disabled;
no loaded world, first frame or desktop acceptance. Extends [Human factory](human-factory-world-bindings.md)
and [script runtime](mission-script-runtime-composition.md).
[Receipt](checkpoints/p2.2-human-vm-world-20261110.json).

## One actual VM and Human dependency graph

MissionScriptWorldLoading now owns actual bindings, Human controls/lifecycle,
view commands, player predicates, experience/objective receivers, animation/model
visibility and MissionScriptLoadingComposition (runtime then module4). All
borrow the SAME Human factory/world services, Players/control, FPV/view, camera,
Sight, hearing, effects, HUD hint, collisions, shared CRT/guard, session/clock,
local packet streams and mission resource. It requires the existing process
Script registration, validated by ClientModuleBinding, instead of replacing a
Startup module slot. Mode/advance use Startup's actual00982100/008692D0 owners.
The new application_flags_owner accessor exposes the SAME mode field.

MissionScriptWorldLink breaks the type/construction cycle without owning a
context or copying authority. HumanScriptContextServices uses this same link as
its context lookup and authority provider. Bind happens only after the actual
module/runtime exist and agree with the same frame registry. Unbound calls
throw, including null-frame queries. Bound null/error/context results and live
authority come directly from the actual module/runtime. Duplicate binding or a
foreign module/runtime/registry rejects; unrelated unbind leaves the binding
intact. Context lookup still includes the module's existing pending-bind owner.

Close finishes module contexts while link and Human receivers still live, then
unbinds the link. The containing world must close this before Human/Items/scene
retirement. The embedded composition's later finish is an actual loaded-false
return. Failed finish is recorded through the actual frame cleanup endpoint;
no loading/guard/readiness success is supplied by this owner.

The actor backend is GameActorServices, sharing the actual derived
GameSceneActors instance. It must match HumanSceneActor's SceneActor backend;
SceneActor<GameSceneActors> is another source type and cannot replace it.
World services now expose checked actual inline/state/composition/selected
queries for these VM receivers. HUD hint uses the SAME small-panel owner;
0049DE30's native signed ID overload derives enabled from ID>=0 and adds no
HUD flags gate. The bool/ID source helper forwards the existing panel receiver.

## Verification

Focused native ASan/UBSan/LSan checks link identity, publication, unbound calls,
actual authority/null/error/context routing, duplicate bind, foreign unbind,
retirement and repeat. Module/context/frame endpoints are doubles; no context
body is executed in that new fixture. Existing context/compiler/body audits
were not repeated without changes.

Strict i386 constructs the actual new VM owner/module/runtime/contexts with
actual shared Human factory/DD0/E0/230/all B0 cases, ScriptContextServices using
the link, Players, HUD/hint/cursor, FPV/camera, derived hearing, Sight/animation,
SOUND/dubbing, collision/visibility, process registry/mode/advance/guard and
packet dependencies. Actual module activate/tick/finish and containing Human
construction/dispatch compile. The prior external VM authority declaration is
removed. Outer HUD objective-world and Players subgroup endpoints remain
external declarations; no containing application instance is created. Application
compile/link and27 tracker checks pass. No original binary or Windows fixture
execution, no desktop claim. Temporary fixtures/binaries hashed and removed.

## Next concrete activation boundary

A targeted original read found the stage13 observed pointer is supplied at
00698BB3..00698C09 after progress5=.8, before status5/spawning. Return-address
mapping00698C09→IAT0080F3EC resolves IsWindowVisible, using actual GetIGraph
007E4EAE→IAT0080F2CC and graph virtual48 window query.00698C5D→0080F3E0 is
GetCapture;00698AAE→0080F1B4 is GetTickCount before progress5=.05.
Initializer006E9EC2 also resolves GetTickCount. Fingerprint-checked mapping and
static original instructions are saved in WORK:
`session-activation-api-resolution.json`, `session-activation-api-boundaries.asm`,
`script-guard-slot13-direct-refs.asm`, `script-guard-slot13-api-resolution.json`.
Original006989B0 export remains in `loader-module-composition/006989b0.c`.

These resolve real Win32 payloads, NOT the private integrity-table side effects.
Startup tables remain uninitialized; no production equality or raw-API-result
substitution is introduced. Original-address/per-thread VM remains outside the
accepted [startup scope](startup-integration.md#resolving-the-opaque-import-boundary).
Continue actual activation API/observed producer, containing session/world,
ordered006CA780 loader, first camera/update/render, failure/repeat and desktop.
No tally additions/promotions:34 complete/1,894 partial/18,783 frozen.
