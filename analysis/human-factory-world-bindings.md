# Human factory and world bindings — source checkpoint, 2026-11-10

Full first P2.2 checkbox remains open. Application still declines mission loading;
no live containing world/session, first frame or desktop acceptance. This extends
[Human composition](human-world-composition.md), [visibility](human-selection-visibility-loading.md)
and [speech](human-speech-loading.md). [Receipt](checkpoints/p2.2-human-factory-bindings-20261110.json).

## Actual same-owner construction

HumanLoadingOwnerFactory now constructs HumanLoadingComposition from the SAME
scene frames/grid/exclusions/tree/resources, Startup morph/physics/action owners,
audio/random, animation callbacks, presentation, world services, Items/animations/
catalogue/sweep, input manager, second tables/navigation, input callbacks, VM
contexts, profiles, Sight, hearing, proximity, camera and Players control.
It checks containing frame/WorldActors identity before allocating a Human owner.
WorldActors still publishes the returned lifetime before either activation pass.
Audio supplies the SAME speech cache through its activation acquire/release API.

Startup now owns009832B0 HumanPhysicsGlobals, not an independent world counter;
the factory supplies that exact owner to all DD0 bindings. Actual physical
increment/decrement bodies remain unchanged. Startup also exposes0084E80C's
FLOAT1.4 scale. Static original direct references are reads; no guessed mutable
scale producer is supplied. Existing physics/equipment constants are unchanged.

HumanWorldLoadingServices now resolves same-owner B0 factories/states for hearing,
actual005A84F0 alignment from session34/roster/16C/configuration and clothing
item descriptor44, actual0045D4A0 Dog registration, shared HUD sound direction,
LS3DF RandFloat (distinct from CRT rand), attachments and actual Car5E0 physics.
Shared presentation reads the owned request's actual mission resource and the
original00785BE0→006B66B0 application fallback. Full construction exposed a
base-actor pointer in Active AI enemy replay: selection now checks the real
Human family before dispatch instead of depending on a narrower caller type.

HumanAnimationWorldBindings routes model EC actor identity to that Human's
published composition/DD0/tracks. Actual00774D80/007759D0 source callbacks now
have concrete per-Human queue, view/FPV, and shared SOUND50/54 endpoints.
007724E0's repeated005A0890 query now receives its actual model/Human identity;
the prior argument-free callback could not supply it. Query return remains
unused. HumanAnimationWorldSound borrows process Startup dubbing and actual
spatial SOUND, with no extra channels or pool.

Full FPV construction exposed HUD cursor mode3's incompatible item interface.
HudCursorLoading now uses FirstPersonHumanItems over the SAME Items owner to
read actual holder3 weapon and its second payload. HudPanels exposes its same
2E60 cursor for FPV borrowing; no duplicate cursor/artwork/roster is created.
HudScene forwards0049DE80 without adding a flags gate, as in the original thunk.

## Reached boundary and exclusions

Fresh B0 bind clears74/78;00537FB0 therefore has no effect on this loading
path. Populated stance commands explicitly reject until their actual command/
posture producers are reconstructed. Fresh Area8 bind skips previous-state
command-area lookup; a populated handover rejects without inventing B4 XYZ or
radius. Initial command flag14 is false. Dog005D5CB0 preserves only the genuine
null534 gate; positive control and nonoffline005A09A0 friendship reject. These
bounded guards are not new function implementations or tally credit.

## Verification and original evidence

Focused native ASan/UBSan/LSan passes actual factory borrower/identity validation,
per-Human animation marker routing into source track event queues, actual source
animation/SOUND wrapper dispatch, shared scale reads/player queries, common
alignment service lazy bindings, checked base-Human enemy conversion and actual
item-module HUD cursor lookup. Native actors, frames, outer owners/constructed
composition and process endpoints are explicit doubles.

Strict i386 compiles construction, both activation/deactivation passes and
WorldActors Human dispatch with actual Human, DD0/E0/230, all seven B0 cases,
input manager/callbacks, Startup/audio/actions/physics, tables/navigation,
ScriptContextServices, Sight/derived hearing router, process SOUND/dubbing,
Players module, shared presentation, HUD/cursor, camera, FPV/first-person player
lookup and player modes. It also constructs the actual shared callback/hearing/
FPV bridges. Containing VM authority/context runtime, outer HUD objective world
and Players subgroup endpoints remain declarations; this is compile-only and
does not create a containing application instance. Application link passes.
Temporary fixtures/binaries are hashed and removed; logs retained.

Original exports: WORK=`analysis/work/mission-world-20261103/`,
`human-active-ai-actions/00537fb0.c`,
`human-model-control-owners/00785be0.c`,
`human-sight-relationship-details/005a84f0.c`,
`human-sight-registration-dependencies/005d5cb0.c`,
`human-sound-state-dependencies/0049de80.c`,
`human-animation-loading-abi-verified/{00774d80,007759d0,007724e0}.c`.
New static `human-animation-process-scale-refs.asm` records original direct
0084E80C readers; PE bytes3333B33F confirm the FLOAT initializer.
No original EXE/DLL/Proton or Windows fixture was executed.

No new function credit or complete promotions:34 complete/1,894 partial
(1,889 client/five engine)/18,783 frozen. Continue actual VM/session aggregate,
real integrity observed publication, ordered006CA780 loading/postactivation,
first camera/update/render, failure/repeat and user desktop acceptance.
