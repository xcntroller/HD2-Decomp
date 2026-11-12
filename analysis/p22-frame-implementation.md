# P22-FRAME implementation —2026-11-12

Fresh fetched/actual base05dbdee51cc366476317011b91b1a1863041dcee;
isolated feature/p22-frame. [Plan](p22-frame-plan.md). Source-only: application
loading/forwarding remains disabled in OWN; no original/Windows code or desktop
execution. RENDER54a93e7, current UIebdcbf5 and masterb132e6d were inspected read-only; their unmerged changes are not imported.
The inherited master/Human handoff does not authorize editing that checkout here.

## Implemented session and control boundary

`GameSessionFrames<Runtime>` now supplies the containing0069A560/0069AF10/
0069B230 services over SAME World, Parent, actual activation SessionSelection,
Startup mode/status/time/Music D0, module façade, camera, Players control, engine
scene, actual animated-model refs, dubbing, subtitles, particles and Car pool.
The transient pass reuses the lifecycle's SAME GameBlastWaves/Startup process;
constructor identity checks reject a foreign World/parent/selection/blast or
external receiver. No new clock, camera, device, roster or resource owner.

`GameSessionLifecycle::bind_frames(external,runtime)` owns exactly one concrete
coordinator. Complete construction precedes publication; failed construction
leaves it unbound for retry. Duplicate binding rejects; explicit retire/rebind is
available before finish. Input/fixed/update forward to this coordinator, without
installing application activation. Finish and activation rollback retire/destroy
it before presentation callbacks, activation receivers or World teardown. A failed
finish retains the World for retry but forbids rebinding frames to the retiring
graph. No new callback registry or simulation/presentation ownership was created.
OWN must keep the supplied Runtime and external producers alive through retirement.

### Original0069A560 positive input policy

The original full1244-byte containing body was inspected in existing fingerprinted
C/JSON and raw instructions. Initial signed20>0 and any38 return before all input/
HUD access. Reached mode2 first calls0049FC10 with the two original input views.
Current application identity, offline7 and actual6E gate quick save/load.2F/30
are FLOAT lanes; NaN is active and negative zero inactive (TEST AH,44 parity).
Actual006B6700 is **difficulty**, not selected game mode: difficulty4 warns19C7;
difficulty3 with signed save-count10>=3 formats2536 with zero. Otherwise actual
selected root and ordinal decide the controller state2 request. The ordinal is
queried even for nonzero root. Mission-save blocking may instead admit state3
when key30 and actual0C are set. These are requests to OWN's controller, not
successful immediate save/load or activation implementations.

Non7 modal0049F090 precedes required0069AAB0. When6E is clear or first is null,
key2E plus full viewport calls actual00423EB0 BEFORE neutral substitution. Missing
either view otherwise uses BOTH existing SessionOwner Registry008AE6E0/008AE5E0
arrays; no new neutral fixture or extra input poll is installed.

Offline status1 uses SAME missionA8.related descriptors: empty/all4 admit;
otherwise an actual0 is required. A blocked objective gate skips HUD/player work
but still reaches the final2E/status14 check. Actual0049C3C0 controls precede the
selected-player query. Missing selected player or paused64 permits1A FPV toggle;
the concrete host calls the SAME view.enable and publishes its resulting58 OR59
to the SAME Startup first-person preference. Otherwise0049D300/result88 can pass
a null pair; selected Human is resolved AGAIN after UI calls, before required
005A3DE0→0053BF90. Non7 mode2/status1 uses the actual local-peer Human loan and3E
sets17 before the final2E/status14 branch. Missing external producers are required
interfaces, not successful defaults.

### Players control14/10 and surface flags

Original vtable0080FC20 identifies virtual14=0045DD90 **fixed**, virtual10=
0045DDC0 **presentation**.00461D10/20 are genuine RET4. Fixed runs0045E380:
selection38 ticks only WITHOUT mode2, then actual004622C0 live-count scan
(return unused), then0045EF00. Presentation invokes0045E3A0→0045E2F0 unconditionally.
This avoids double-aging selection in ordinary offline7 fixed+variable frames.

0045EF00 uses the same objective admission rule, reverse actual roster, nonnull
Humans and signed positive EC health. Surface flags are ORed, compared against
control34, then PUBLISHED before any result/hint callback.2000 invokes actual
MissionObjectiveStatus.fail_all(true); otherwise4000 selects hint1CEF, else
FFFFFFFF. Repeated identical aggregate flags do not repeat callbacks, including
a retry after a callback failure. Existing Script/world objective owner is reused.

005A9720 reads actual DD0754 contact and masks6000 while unattached. Attached
non-Car genuinely returns0. Attached Car reads its existing initialized wheel
owner;004DD090 visits wheels reverse, admits1FC bit8 and nonnull200 identity and
reads the LIVE triangle/shape/dynamic descriptor flags, not cached query_flags.
No second contact store or made-up material flags are supplied. Missing actual
DD0/initialized Car rejects explicitly. Actual collision identity lifetime stays
with the existing World/physics owners.

### Fixed/variable ordering and exit

Existing offline policies remain in place. The previously rejected mode2-clear
continuations now have actual source bindings: fixed additionally updates30/7C
when result88 is zero, then2C, authority modules/transients and Scene20/reverse
models; variable status5 becomes1 and visits actual identity slots freshly,
re-resolving before Human virtual14. This supports original callback mode changes
without inventing a nonoffline match/peer implementation.

Delayed offline objective exit uses the actual related descriptors and6F. It
clears88 BEFORE current-state identity, preserves foreign-current early return,
sets14 for actual saved-state/6F, otherwise uses0069A500: status6,20=500,24=1000;
its supplied stack100 is unused. A repeated status6 does not reset counters.
Current-player camera command2(1,0) is a genuine offline7 no-op. Non7 match/network
exit remains an explicit required failure after the original state prefix.
0049B7C0 is also an actual RET4, not a HUD flags1 setter; warming preserves it.

Ordinary presentation remains Players→ascending modules→FPV→engine Scene→reverse
models→dubbing→attachments→B0/status12→text→Car emitters. Paused64 remains camera→
paused module17 particles→text. Bucket recycling precedes both. Session20 reaches
zero one update BEFORE status5 transitions. Music D0, DWORD wrapping and byte/
word gates are retained. `advance_application_frame` still renders before the
fixed/update/control phases and presents afterward; FRAME neither polls devices
again nor owns graphics present.

## Exact external contracts and incomplete composition

The mandatory `GameSessionFrameExternal` interface exposes SAME World identity,
actual controller14/current identity and controller04 state request, save guard/
retained application descriptor, UI warning/filter/modal/control/HUD/FPV and
Human input, cutscene and non7 input/peer receivers. The concrete FRAME adapter
binds all available local owners; this interface has no successful defaults.

At current inspected UIebdcbf5, `advance_hud(elapsed,GameUiHudFrameSources)` and
`advance_first_person(elapsed)` already exist on unmerged GameUiPresentation.
Its `process_hud_controls(...,first,second,64)` exists too. UI/OWN must bind their
real order/command/occupied/console sources and the exact0049FC10/0049F090/
0049D300 contracts; these are distinct operations, not one generic input handler.
0053BF90 mapping belongs to the actual Human Player B0; current source supplies
later local-input/weapon frame policies but no positive mapping entry.00423EB0
must use its actual script process. No input suppression or dummy player is used.

Full Runtime remains OPEN. Actual GameMissionModuleFrames and Human B0/DD0
policies are instantiated; the compiler identifies exact outstanding Runtime
operations, rather than treating all Human frame source as absent. Reached Human
motion/action/attachment/Player weapon/formation, Dog/Sight and loose-item physics
consumers still need concrete positive services. Required boundary doubles in the
i386 audit throw if called; they are not production bindings or mission evidence.

### Runtime11 containing source and real packet/fade bindings

005B7320/005B7480 are now methods of the existing DynamicItemActor, dispatched
by GameSceneActorFrames through SAME GameRuntimeActors. Authority entry90 gate
precedes all work;6B0 wraps before conditional body24;6AC countdown restores via
005B7EC0 BEFORE publishing-1. Finite6A0 wraps and uses signed<=0; expiry calls
GEffects60, hides/updates the actual model, publishes the real module12 message,
then writes-1, and only then removes through the existing deferred registry when
mode2 is clear. No actor reads follow removal. Positive<=500 publishes6A8 before
the existing retained fade-out queue; failures preserve that prefix. Presentation
always calls body24 first, even with no item, and only then wraps/clamps6A4.

Raw00486E90 confirms GEffects60 is NOT fade cancellation: it invokes virtual50
with0, actual00486D30→model54(1). The concrete adapter uses existing
EffectsShadowModels.model_camera_amount;64 uses SAME frame_fades().fade_out,
remaining time and hide=false. Expiry leaves retained fade records alive.

publish_expiration writes through SAME Dependencies.broadcast_packets, which
already borrows Startup's process owner: begin(12,0), selector0/2 bits, actual
actor10 identity/10 bits, opcode0/**3 bits**, finish_broadcast. Original
005B73DC..005B7404 uses DOUBLE4/DOUBLE2 logarithms plus DOUBLE1 then __ftol=3;
Mine's two-bit command is a different grammar. Existing stream header5 makes
20 written bits. No duplicate packet, transport or stream registry is introduced;
no missing unmerged helper is assumed. This binds the actual production consumer,
while native execution uses labelled engine/physics boundaries.

Body24 is REQUIRED through advance_loose_item_body(body,elapsed,SAME WorldActors).
It is separate from module2 physics. The actual implementation is now committed
on masterc1908cf, **after this worktree's exact base**: RigidBody.update plus
GameWorldActors.update_detached_body and its ray/exclusion/actor/water endpoints.
Its callback contract also changes from Descriptor* to full ContactEvent*, so
import must include the matching DynamicItemActor and callers. No body/kernel
or ledger was copied. Its source proof confirms fresh part5F9 is unproduced;
004CB2B0 reads it before motion. False/zero/random initial state is forbidden.
Current master source is therefore an import dependency with a remaining original
state-provenance blocker, not proof that a live item can move. Wheel695 is a
separate confirmed original unknown on its reached launch path.

005B7EC0's original offline7 RET is bound. Positive non7 restore still needs the
SAME Runtime retained-items array, original pose/005B7DA0, quaternion inversion,
vector/direction quantizers and module12 publication. It remains an explicit
required Runtime receiver; no synthetic saved item or packet is installed.

### Existing Human leaves connected without taking over HUM

00768170 now calls the existing real007646F0 generic walk coordinator over SAME
DD0/Look/Human/World, with actual command publication and its boolean result;
the timer still publishes first. This removes a redundant whole-walk Runtime
callback, not the solver's positive HUM/geometry dependencies. Ladder0074B4F0/00768230 and step0074A690 route notifications to SAME existing
World.notify_human_ladder_probe, which still explicitly rejects the missing
non7 command15 producer00588D20. There is no successful notification substitute.
007646F0's reached native MessageBeep(-1) uses the actual GameActorServices Win32
endpoint through World; no Runtime debug-beep callback remains. Delegated Win32
execution earns no function credit and was not run.

### Mutable original input pair correction

Frame input, lifecycle forwarding, external HUD/Player interfaces and neutral
Registry access now preserve mutable uint32_t arrays. HUD controls consume the
same arrays before Player input and the final2E status gate; a const pointer
seam could not bind current UI.process_hud_controls correctly. No copies or
additional polls are introduced. OWN's still-disabled application forwarding
must carry the actual writable sampled views; it must not cast genuinely
read-only storage. Other application states and their interface are untouched.

### Remaining blockers, with owners and reached branches

[Exact current Runtime contracts](p22-frame-external-contracts.json) records54
required typed operations with their current consumer lines and gates. This is
an instantiation inventory, NOT54 observed Tutorial failures or a claim that all
callee bodies are missing everywhere. Four redundant boundaries were removed:
walk, Ladder notification, platform beep and whole loose-item update; the latter
is replaced by specific physics and positive restoration requirements. Packet
expiry is now concretely bound to the base's real process API.

- **HUM/physics import:** masterc1908cf actual kernels004C9000/004C94B0/
  004CB2B0 and SAME World adapters; original first5F9 read remains blocked even
  after source import. Actual bodies now exist elsewhere; do not repeat an
  across-all-branches missing-body claim.
- **HUM/Player/Items:** remaining B0 families, DD0 action/motion events,
  attachment/action/formation, weapon/item24, actual0053BF90 input mapping and
  Dog/Sight producers are not FRAME replacements. Native inputs/populations may
  determine which branches run, but absent captures do not prove inactivity.
- **PACK/Runtime:** positive005B7EC0 retained restore and non7 notification/input
  transport; actual stream ownership exists and is already reused for expiry.
- **UI import:** source is COMPLETE on unmergedebdcbf5, not missing. The base
  lacks advance_hud/advance_first_person/process_hud_controls and the updated
  command render receiver. Use GameUiHudFrameSources/GameUiHudControlSources,
  actual mutable64-word views, and real order/command/network/selector/console
  loans. GameSessionFrameExternal is the explicit consumer boundary; no false
  HUD suppression or successful external receiver was supplied.
- **RENDER import:**54a93e7 actual stages/callback/procedural/special-model source;
  strict initial volume3C/40 remains an original provenance blocker. General
  network overlay00724CB0 is still an actual RET, not a network prerequisite.
- **Conditional non7 session FRAME bodies remain OPEN:**0069AF10 requires
  actual config170 console/007447B0 messages,88=3 match-reset/peers/config238/
  224/27C,88=2 round/config234 and positive00699B50/00699320;0069B230 requires
  0069EED0 clock, config224/23C/240 and0072A0B0/0072DCD0/0072B1C0 command/peer
  producers plus90/9C pending queues. Those typed process/config/peer owners
  are not exposed by this base's session seam. Both paths fail explicitly after
  their preserved native prefix; no offline flag is forced to admit them.
- **OWN/LIFE:** actual controller identity/requests/saved state, script cutscene
 00423EB0, loaded install/application mutable input forwarding and scheduling;
  retire this coordinator before finish/rollback/World destruction. Existing
  render-before-fixed/update/present policy is unchanged.

FRAME is **blocked, not source complete overall**. Its offline session/control
and Runtime11 containing work and available base bindings are verified. Concrete
physics/positive restore and conditional non7 bodies above stay open; these are
not relabelled completed just because they need other component contracts.
Further progress needs those imports/producers or original state provenance,
not another successful boundary double. No merge, push or application enabling
was performed.

## Verification checkpoint scope

Native ASan/UBSan/LSan runs the actual session policies, actual borrowed
application status, actual control/roster/selection owners and original outer
application frame policy.100 input plus100 Players-control lifetimes cover
positive/warming/save/load/warnings/neutral pointer identity/NaN/negative zero/
objective/modal/cutscene/non7 input, ordinary/paused/state-change ordering, all
reached variable-service failure prefixes/retry and publish-before-callback flags.
Additional actual wheel/triangle/shape/dynamic descriptor records verify reverse
6000 admission and fresh identity flags. UI/controller/engine endpoints are doubles.

A separate native audit executes actual GameSessionLifecycle source with explicit
World/Frame/UI/platform doubles:100 construction-failure/retry, duplicate bind,
forward/retire/rebind, frame retirement before failed finish and World destruction,
rebind rejection during retirement and activation-primary/rollback-secondary cases.
This tests lifecycle ownership, not native64 execution of the Windows World.

Strict i386 compilation/link includes actual full World construction/destruction,
GameSessionFrames, both complete module passes, actual session policy/engine/
model/dubbing/camera/physics/Car/particle/control/DATA receivers and lifecycle
binding/forwarding/retirement. Required external UI/Runtime endpoints are explicitly
throwing, defined doubles, not unresolved declarations. No production closure is
claimed for those unavailable endpoints. Existing application link is checked
separately; no Windows fixture or original code is executed. Temporary audit
sources/binaries are fingerprinted/removed in the checkpoint receipts.


## Final containing-frame checkpoint

[Receipt](checkpoints/p22-frame-runtime-20261112.json) records actual source and
original export fingerprints. Native ASan/UBSan/LSan passes100 loose-item policy
families plus100 actual RuntimeActorRegistry creation/deferred-removal lifetimes,
actual RigidBody loading/geometry and shared retained EffectsFrameFades, actual
SessionBroadcastPackets bit serialization and partial body/fade/restore/packet
failure/retry prefixes. Engine geometry/visibility/math, positive physics, saved
item restore and sound emissions are explicit doubles. Native input audit passes
100 writable input pairs, UI mutation visible to Player/final session gate and
failed UI prefix. It does not load a real mission or seed production populations.
Strict i386 full actual World/lifecycle/module graph defines throwing external
UI/Runtime doubles and omits the removed Runtime methods; link succeeds through
the new actual containing/fade/packet/Win32 bindings. Existing application link
passes separately. Windows outputs are never executed. Temporary fixture
sources/binaries/scripts are fingerprinted and removed; concise logs retained.
Two new partial inventory entries005B7320/005B7480, coverage expansions only for
0069A560/00768170, and zero complete promotions. No kernel/delegated/absent-entry
credit. Existing full-ledger tracker is still limited by the unrelated missing
legacy0062D880 WORK evidence; focused changed-entry/inventory checks pass.
