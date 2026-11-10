# Human/world lifetime and script binding — source checkpoint, 2026-11-10

The Human loading owners now have one concrete source composition and Scene2
type3 activation dispatch. **Application mission loading remains disabled.**
There is no activated world, first frame or desktop result. Continue the same
full first P2.2 implementation checkbox.

`human_inline_loading_factory.h` constructs actual HumanControlLoading DD0 and
HumanEquipmentLoading230 with a HumanEquipmentServices bound to that exact DD0
and E0. It requires the SAME frames, collisions/exclusions/tree/sweep/resources,
morph/physics globals, audio/rand/events/effects/presentation, Items/animations
and catalogue. The factory retains the230 service until the230 borrower retires;
replacing a service with a bound visibility receiver rejects.

`human_loading_composition.h` owns that factory, inline98, actual
HumanLoadingStateFactory/Owner, inventory commands and HumanLoadingActivation.
Inventory is published before B0 binding. B0 closes while inventory and inline
still live; DD0/230 clear before factory service retirement. Both actual
activation passes share these instances. WorldActors owns one lifetime per
Human, publishes it before activation, exposes a checked typed lookup and
retires it before authority Scene2 deletion, including failure. Close retires
Humans while actor identities, Items and shared dependencies still live, then
continues prior runtime/Physics/ordinary-actor teardown. The containing actual
consumer must construct this composition; no default successful factory exists.

A concrete failure defect exposed by composition is corrected:
HumanStateLoadingOwner::retire previously left B0 alive if close threw, while
HumanLoadingActivation continued into inline retirement. It now attempts both
retained state closes, deletes them and retracts publication before reporting
the error. Normal close/delete/publication order is retained.

The shared GameHumanSoundLoading bridge can use HumanSoundStateRouter to resolve
each receiver's own factory. A source factory includes a per-Human inline98;
using one factory for all listeners would route look reactions into the wrong
control. Routing validates the actual published state and factory/control Human
identity. Existing actual A8/AC/B0/B4 handlers then run with that factory; missing,
stale or wrong-owner bindings reject.

## Actual script context family and binding order

00427730 chooses Human00421BC0 or Dog00421930, otherwise base0041DCB0. New
`mission_script_actor_context.h` preserves actual borrowed actor10C and zero
byte108, base frame retention and SAME EC word1 publication. The original virtual
family getters are0/1/2 (0041DF10/00421BF0/00421960). These three getter entries
are absent from the frozen inventory and gain no credit. Actual Human00421C00/
Dog00421970 command dispatch remains required; selecting a family supplies no
successful command receiver.

`mission_script_actor_bindings.h` supplies original precompile/complete ordering.
For Human kinds1/4, an existing B0 receives005A3DF0(true): reset active AI orders
when state3, disable animation control, then select state5. A null B0 genuinely
makes no call. The state5 completion flag is captured **before** compilation;
completion rereads current B0 and calls005907C0 with the actual context or null.
Dog completion genuinely skips null534; its positive control virtual40 still
requires the actual Dog owner. Base contexts have no derived notification.

HumanStateLoadingOwner also implements005A3DF0(false), choosing player versus AI
from the SAME Players current-Human pointer. HumanScriptAIStateLoading supplies
the actual005907C0 context borrower setter. No empty successful script context
or state is manufactured.

The new context already owns frame EC word1 when precompile state5 binding reads
it, before it enters the module's vector. MissionScriptContexts therefore tracks
each bind's actual local owner in a scoped pending stack, as well as appended
owners. It validates both pointer and frame identity. On compile failure this
borrower and the context/EC reference retire **before** the captured failure
notification; successful append transfers ownership before begin/notification.
Nested binds retain their respective local lifetimes. Context construction no
longer hides state notifications inside an opaque factory: Runtime must provide
actual prepare_script_binding and captured-target completion receivers.

Cold initial module4 binding precedes Human activation, so B0 is still null.
Later Human activation sees the SAME completed EC context and selects state5.
Live ScriptAssign, context return/end and derived command/predicate receivers
remain part of the containing VM integration; source ownership is not execution
success.

## Evidence and verification

[Receipt](checkpoints/p2.2-human-world-composition-20261110.json). Original client
SHA256:8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.
Ignored durable WORK=analysis/work/mission-world-20261103. Existing originals:
`human-state-script-dependencies/{00421bc0,00421930}.c`,
`script-program-owner/00427730.c`, `human-derived-offline-loading/`,
`human-derived-state-factory.asm`, `human-state-script-dependencies/` and
`actor-activation/0059f2c0.c`. New read-only export
`human-script-binding-notifications/{005a3df0,005907c0}.c` and static vtable/
binding-call inspection confirm constructors, family values, notification
ordering and rereading current B0. No original binary is executed.

Focused native ASan/UBSan/LSan checks pass actual state-owner failed handover,
either close failure, destruction before inline retirement, publication/retry,
script enable/null/current-player selection, and multi-Human SOUND identity
routing. A second fixture runs actual Human/Dog/base actor/context/compiler/EC
and binding owners: cold null B0, pending context lookup during prepare,
success/failure ordering, missing/bad scripts, retain/prepare failure cleanup,
retry and nested binding. B0/inline/frame/file/lookup boundaries are explicit
doubles; this is not complete native Human/DD0/world execution.

Strict i386 compiles actual host frames, DD0/E0/230, all seven B0 factory cases,
inventory, Items, animation resources/events, Sight, process SOUND/proximity,
Players/camera and source context/compiler/actor bindings together. Session/VM/
HUD/presentation lookups remain required declarations in the compile-only
fixture. Application compiles/links and still declines loading. No Windows
fixture or game/DLL/Proton execution; no desktop claim. Temporary fixtures are
hashed/removed at this checkpoint, with original exports and logs retained.

Four frozen client entries gain partial status (005A3DF0,005907C0,00421BC0,
00421930); source compositions and absent getters gain none. Totals34 complete /
1,811 partial (1,806 client/five engine) /18,783 frozen, with zero complete
promotions. Continue actual VM/world consumer definitions, FPV/HUD/SOUND/module/
session construction, ordered loader/postactivation, first camera/update/render,
failure/repeat and user acceptance. [Human dependencies](human-state-loading.md),
[activation](session-activation-loading.md), [scripts](mission-script-loading.md).
