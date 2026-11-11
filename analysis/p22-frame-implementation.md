# P22-FRAME implementation —2026-11-11

Fresh fetched/actual base05dbdee51cc366476317011b91b1a1863041dcee;
isolated feature/p22-frame. [Plan](p22-frame-plan.md). Source-only: application
loading/forwarding remains disabled in OWN; no original/Windows code or desktop
execution. RENDER54a93e7 and UI74220d4 were inspected read-only, not imported.
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

At inspected UI74220d4, `advance_hud(elapsed,GameUiHudFrameSources)` and
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

The independent Runtime11 continuation is currently under investigation:
005B7320/005B7480 call embedded RigidBody virtual24=004C9000, not module2 Physics.
Fresh read-only, transaction-rolled-back exports establish its virtual20=
004C8EE0 substep policy and positive integration004C94B0/004CB2B0 dependency.
Those integration producers own collision records5BC/5C4 and sleep/contact state;
they must not be replaced with an empty collection, a successful tick or module2.
Original004C9000 and005B7EC0 exports are evidence, not implementation credit.

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
sources/binaries will be fingerprinted/removed in the checkpoint receipt.
