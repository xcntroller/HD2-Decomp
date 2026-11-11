# P22-RENDER source checkpoint — 2026-11-11

Implementation is **partial**, not complete P22-RENDER delivery. No application
activation, original engine execution, graphics present or desktop acceptance.
[Plan](p22-render-plan.md), [receipt](checkpoints/p22-render-containing-20261111.json).

## Current concrete call graph

FRAME/OWN caller → GameSessionLifecycle::render → render_session0069B870 policy
→ SAME parent GameScene::render → checked Scene78(scene,0),10046860 → registered
GameUiSceneCallbacks →006C9260. Existing lifecycle bind_presentation installs the
actual borrowed dispatcher; retire_presentation suspends/unbinds before UI/World
retirement. New entry rejects absent/finished presentation. Offline7/config170
policy and presentation ownership are unchanged. No scheduler was installed.

Stage1 retains HUD0049B9B0 → control/render stages → optional0079B760.
Stage2 retains HUD/full physical viewport (including old near/far), mission text,
RET00724CB0, gated session fade, GEffects post fade and process text overlay.
Existing text/font/fade/texture/camera owners and graphics device are preserved.

GEffects::render_before_scene invokes SAME C4 VolumetricLights with signed bits
of process224 and its concrete Startup-owned process0089B63C renderer.00482260
calls it before driver3=1/camera/traversal even with no volumes. This receiver now
has positive procedural geometry and real graphics resources; missing texture0
is the original legitimate early return, not an installed empty service.

## Implemented containing behavior and original evidence

Client SHA2568c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.
Original bytes read with objdump; no code executed. Existing C exports in main
WORK frame-effects-render-required and frame-overlay-and-volume-owners agree
with the inspected instructions at the following boundaries:

-00482274..004822AB booleanize driver3,Graph14,Graph13 independently; Graph14=0
 precedes process00482E50(time).004822D3 sets driver3=1 after it.
-004822F1/0048232D select Scene15C twice, update world matrix and read position70
 and forward60. This is not Sight's Scene128 camera.
-00482597 calls wrapped atan2 delta007885B0 on owner18/1C, updates accumulator10.
-004825DC removes a light whose108 relation is null;0048264B tests relationF8=5
 and160 bit1. Horizontal normalized direction, hemisphere dot and horizontal
 distance precede strict distance<volume30.00482843 updates3C/40 and44.
-00482891..00482904 swaps only visible pointer slots pairwise when the later
 distance is greater, mutating retained order; ties are not replaced by std::sort.
 Actual existing00480D80/00480390 leaves receive distance14, accumulated44,
 owner phase10, camera position and SAME retained particles/textures.
-0048290F..00482954 restores boolean states in driver3,Graph14,Graph13 order.
 Source CleanupErrors attempts every restoration and preserves primary failure;
 this exception behavior is a native safety extension, not original engine SEH.
-00482DCD..00482DE9 resets owner phase10 and previous direction18/1C=(1,0).
 Existing load/read clear reaches this reset. No random calls were added.

**History blocker:**00481320..00481372 initializes44 via shared RandFloat*2pi
 but never writes3C/40. Existing read/refresh do not write them either. Source
 represents these lanes as unknown optional state and fails at the first reached
 visible/in-range history read. This is an explicit safety rejection, not original
 successful first-frame behavior. Audit-only known history is not a production
 producer. Do not infer initialization from allocator reuse or captured field2C=0.

## Positive procedural process —00482E50/00483E50/00483DE0

`ProceduralClouds<ResourceOwner>` now lives ONCE in StartupSession, representing
process0089B63C independently of the per-world C4 volumes and224 clock. GEffects
virtualB8 (vtable008101F0+ B8=00485B50) now has a concrete configure_procedural
receiver. Original references contain no direct CALL00485B50; its one absolute
reference is vtable008102A8. The actual opcodeAD virtual caller is now source-bound through the existing
VM/runtime and containing module loan (see continuation below). Application
mission execution remains disabled; no enable call is invented.

00483E50 first clears, then loads clouder.tga/clouder2.tga through SAME graphics
load_texture(primary,NULL,502,out), retaining actual out pointers even if returned
status is nonzero (native ignores it). It allocates two0x3CC00 buffers even with
missing textures. Texture0 absence legitimately returns without camera/draw;
texture1 absence still binds NULL if its positive batch exists. Partial prefix
initialization remains owned for clear/retry.00483DE0 releases texture0,texture1,
buffer0,buffer1. The production C4 owner now binds SAME process retirement into
its clear/load/destructor path before volume retirement (00482D70). The callback
is gated by process.world==this, so a rejected competing constructor cannot clear
an existing World's process resources while its members unwind. Startup has
an idempotent fallback before engine/device shutdown. No frame borrowers enter
this process owner, and no second resource cache was created.

Instruction trace resolves all former lost C operands:

-0048302D..00483084 truncates camera position*FLOAT1/16, then ascending X/Y/Z
 ranges centre-6..centre+5. Integer seed2X+5Y+3Z wraps in32 bits.
-00483149..004831AE computes extended sin/cos(seed), stored sine for X/Z jitter,
 size=sin*.75f*4.f+12.199999809265137f; positions=(X+sin,Y+cos,Z+sin)*16.
-004831B5..0048336B retains X displacement in x87, stores Y/Z, computes squared
 distance and normalized away direction, rejects back hemisphere against actual
 normalized Scene15C forward. Three actual camera samples provide eye/forward/up.
-0048337B..00483467 alpha: squared<=100→0, squared<400→(sqrt(squared)-DOUBLE10)
 *DOUBLE.1*DOUBLE.4000000059604645;400..2025→FLOAT.4; beyond2025→
 (1-(squared-2025)*FLOAT.00033057850669138134)*FLOAT.4. Clamp0..1, draw only>.001f.
-0048346D..004834A3 uses full SetDir(away,camera-up) after Identity, then actual
 centre in matrix translation lanes. Keep all16 lanes; no SetDir3 substitution.
-004834A8..004834F5 chooses texture0 for stored_sine>0, texture1 otherwise.
 Stored alpha*255 then current-mode FISTP forms ARGB white. Exact six-vertex
 order: (+,+)UV11,(+,-)UV10,(-,-)UV00,(-,+)UV01,copy0,copy2.
 Stack pushes temporarily change offsets; the apparent sine-valued quad corner
 in the old C export is actually the overwritten negative-size slot.
-00483D1C..00483DCA sets driver3=0 even for zero batches, then binds texture,
 identity world transform and triangle-list draw for batch0 then1.00482260
 restores boolean driver3/Graph14/13 even on propagated source endpoint errors.
 Passed clock is genuinely unused by00482E50. No randomness is added.

Malformed/nonfinite/out-of-range camera input and incomplete positive buffers
reject safely; these are source guards, not original successful branches.
Static constants/strings/matrices were read from fingerprinted PE sections;
actual engine-selected texture bytes remain unknown. Existing DATA stores record
actual texture out-pointer presence/load status, camera selection and draw calls;
World snapshots now record both process texture populations. These events never
claim selected-byte provenance that the engine API cannot provide.

## Debug skeleton and owned callback composition

0079B760/0079AC40 are now implemented over the actual Human geometry skeleton,
not independent joints or synthetic cameras. Original exports were regenerated
read-only with temporary thiscall(vertices,indices,count) ABI, transaction rolled
back; instruction ranges and script provenance are retained/fingerprinted.

Process0098AF90/94 lazy buffers are3980 bytes aligned16 and730 bytes of WORD
indices. Reverse actual identity-slot traversal uses00475D50 INCLUDING pending
reserved-ID lookup. Qualifiers: type3, not005A5D30 suspended, actual DD0, geometry
model08 epoch11C equals actual Scene11C. At most21 qualifying records are admitted.
Missing actual containing Human owner remains an explicit failure, not a fake
successful null-DD0 source. Existing actual null control is the native skip.

0079AC40's frame offsets14+index*440 resolve to these20 bone pairs:
22-9,9-13,13-8,8-4,4-5,5-6,8-14,14-16,16-18,18-20,
8-15,15-17,17-19,19-21,22-11,11-3,3-1,22-10,10-2,2-0.
Every endpoint rereads actual world matrix70..7C (four lanes) even for repeated
bones; sequential WORD indices/count advance twice per line. Nonzero count sets
identity world transform and invokes checked driver30(vertices,count,indices,
count,RGB,16,alpha). Actual process colour00863620 starts00FF00FF; RGB uses
FLOAT.003921568859368563 and the original high-byte alpha is preserved.
No graphics/driver state changes beyond the original identity/draw are added.

GameRenderStages now implements both required UI seam methods over SAME World.
GameSessionLifecycle::bind_presentation() owns this adapter and the existing
GameUiSceneCallbacks. Engine callback unregistration/dispatcher retirement happens
before adapter retirement, pages, World/camera/textures. The external-receiver
overload remains available; no automatic application activation was introduced.

## Remaining boundary and external dependencies

The original per-volume3C/40 first-read state remains the sole OPEN RENDER body
boundary. No constructor/read/refresh producer is established. Existing source
rejects a reached read. A native fixture with known seeded history is only a
bounded source audit, never actual initialization or mission reachability.

UI base GameUiPresentation::command_before_scene rejects004B6720. UI74220d4
contains later command geometry/full HUD dispatch; inspected through Git only,
still unmerged. Its GameUiRenderStages seam is unchanged. UI's remaining Human
order, occupied actor/FPV and console producers remain its own contracts. No UI
code copied, receivers suppressed or successful replacements installed.
The exact base failure is stage2→render_hud→HudSceneLoading::render→hud_render:
after ordinary HUD/camera/no-GI gates, HUD.flags4 plus offline7 and visible command
panel require GameUiPresentation::command_before_scene/004B6720, which throws in
this implementation base. Stage1's concrete pre-scene service is independent.
Latest UI's positive replacement uses the same consumer; it is not imported here.

OWN must install/activate the lifecycle and caller; FRAME owns render-before-
update scheduling/camera simulation. The actual VM/configuration path now supplies virtual
GEffectsB8 behavior when opcodeAD is reached. Empty current textures do not prove that future
procedural branches are inactive. Live resource bytes/populations/branches and
user-operated desktop acceptance remain unobserved. No push or merge.

## Verification

Native ASan/UBSan/LSan: procedural positive generation, both batches, exact UV,
colour/capacity,100 resource lifetimes, absent texture0/1, partial configure/retry,
C4→process retirement, every reached operation failure prefix and malformed
camera. Separate skeleton audit checks20 lines/reverse/max21/WORD indices/fourth
lane/alignment/colour,100 lifetimes and40 bone failure/retry paths. Only engine/
Human endpoints are doubles; procedural geometry has no seeded collection.

Strict i386 compile/link instantiates actual World/GEffects positive configuration/
render, debug Human/registry/geometry/driver endpoints, DATA snapshot, concrete
GameRenderStages, lifecycle-owned callback install and lifecycle→actual Scene78.
Application-launch compile/link passes. No Windows output or original code ran.
Temporary audits/scripts/binaries fingerprinted then removed; concise logs and
static exports retained in ignored WORK. Six frozen inventory entries gain partial
source coverage; no complete promotion or absent-wrapper/delegated-engine credit.
Tracker render/check still encounters pre-existing missing ignored evidence
`analysis/work/multiplayer-rule-caret-20261101/rule-refresh.asm` (0062D880).

## Strict history continuation

[Allocator-to-first-read proof](p22-render-history.md) and
[receipt](checkpoints/p22-render-history-20261111.json) now establish the ordinary
non-clearing allocator path and residual3C/40 boundary. The user chose original
behavior, so no new initialization policy is installed. Actual render operation,
camera and reached light/history state diagnostics are bound to the existing DATA
store with primary-error preservation. This is not additional function credit,
full containing closure, original engine execution or desktop delivery.


## Configuration, special-frame and Scene continuation

Latest receipt: [bindings checkpoint](checkpoints/p22-render-bindings-20261111.json).
The previous configuration-caller exclusion is closed. The remaining RENDER-owned
body boundary is still the original residual3C/40 read, not a UI dependency.

### Actual procedural producer and lifetime

Original00416E10 dispatch byte53 corresponds to **opcodeAD**, not opcode53:
0041CDBC[AD-1]=53 and0041CBE0[53]=0041CAE6.0041CAF1 resolves the operand,
0041CAFF reads the full DWORD integer pool,0041CB0A tests it and SETNE AL feeds
GEffects virtualB8 before00416CF0's advance gate. The existing compiler already
recognizes CSC_RunCLouder with one integer. The actual context/runtime now
routes this command to the SAME effects façade, whose fixed borrowed service
is published by EffectsModuleLoading only after identity validation. It invokes
Startup's single process configure; no resource owner/cache/clock is duplicated.
An unbound/retired service rejects explicitly. Constructor failure or a competing
module cannot steal/clear the original loan. Close retracts it before retiring
resources, including configuration before activation; C4 clear occurs while the
module still owns process.world. Retry binds the same service again.

### Special2E0 service required by current UI

UI87d92dc first exposed the requirement; latest inspected UI74220d4 retains
GameUiCommandModalSource::retire_command_special_model(World&). Its native
operation is GEffects44/004870A0(false,0,true). The source now provides the full
positive receiver, not only a null-state success. The historical label “special
model” refers here to a **kind2 light frame**; MP_Lightning is the timed lookup.
Existing engine light creation (driver kind2) and original CMP WORD[F8],2 agree.

004870A0 (thiscall,ret0C) retains WORD-E4, saves raw160 mode, sets virtual94(11),
shows/updates, then reads168 power, clears2EC and stores2F0=-1. Active/non-null
input cannot replace an existing frame. Deferred false marks2EC; immediate false
restores94(mode),88(power), hides30(0), releases0 then clears2E0. Source partial-
initialization/cleanup guards restore only acquired state, attempt all retirement
operations and preserve the primary exception. These guards are not engine SEH.
The existing GEffects façade owns this state alongside its existing fade queues;
World/module close retracts it before frame/Scene destruction.

004871A0 (ret4,frozen148 bytes) resolves MP_Lightning/FFFF, checks kind2, uses the
same initialization and stores exact duration bits. Missing frame legitimately
returns. The native repeated/wrong-kind pointer-overwrite prefixes are unsafe;
source rejects explicitly instead of leaking/releasing an unacquired reference.
This limitation keeps the entry partial. No live weather/network caller is claimed.

00487416..004874BF runs before fade/time224. One SAME engine RandFloat call is
multiplied by saved power in x87. FSTS stores FLOAT for88 but the comparison uses
the extended product against actual FLOAT.5. Product>.5 enables if hidden, calls
88(stored amount), then24. Low/unordered products hide/update only when visible.
Deferred retirement follows flicker. Signed-positive2F0 subtracts wrapping elapsed;
expiry sets2EC for the NEXT update, which still consumes its random draw before
restoring. Remaining2F0 subtraction also occurs after same-pass retirement.
RandFloat1002C450's FILD*FLOAT return is extended ST0. The existing frame endpoint
now captures ST0 as long double; i386 cdecl uses ST0 for both result forms. This
removes an artificial FLOAT spill before0048134E/00487429, adds no random draw
and keeps existing explicit FLOAT consumers' boundaries.

Original opcodeBB/THUNDERSTORM_SetOn is dispatch5F→00418576. It resolves the full
integer and retained frame slot, converts non-kind2 to null, invokes44(enabled,
frame,false), then advances. The existing runtime now supplies this concrete
producer. The unmerged UI modal integration can forward directly to
world.scene().effects().configure_special_model(false,nullptr,true); no UI
implementation was copied or successful placeholder receiver installed.
004888C0 finish restores2E0 after lensflare visibility and before sun/reset.
Source cold-close and WorldScene fallback also consume its frame loan.
DATA snapshots/events retain actual presence, mode/power bits, flicker operations
and timer/deferred state without additional engine samples. Historical resolver
bytes, static MP_Lightning requirements, actual populations and executed branches
remain separate. Native fixture populations are not actual mission reachability.

### Actual Scene callback guards

Native execution of the actual GameScene header found three source exception
bugs: registration failure stranded active_; failed renewal erased a prior
borrower; an endpoint exception after a latched C-callback exception masked the
primary and poisoned the next operation. The bridge now retracts failed initial
publication, restores a previous borrower on failed renewal, and consumes the
latched callback failure before propagating a secondary endpoint failure. Render,
advance, refresh, reset and mission-open use the same rule. Loading Reset RAII
and actual Scene78(scene,0) remain unchanged. Checked engine10046820 only stores
callback120/flags124 and has no positive failure/callee; these source guards do
not invent original engine behavior. No original entry receives credit for them.

### Verification and stop condition

Native ASan/UBSan/LSan passes actual compiler/context opcodeAD→actual module's
configuration loan→real procedural resource owner,100 module lifetimes, partial
texture retry, wrong process/competing constructor rejection, cold close, renewal
and post-retirement rejection. Separate procedural audit executes both generated
batches; it never seeds a fixture cloud collection. Special-frame audit executes
100 compiler/context opcodeBB lifetimes, visible/hidden/NaN/extended-versus-FLOAT
comparisons, exact random counts, deferred/timed/wrapped expiry, every reached
initialization/restoration/positive-update failure prefix and retry. Only engine/
resource/camera endpoints are explicit native doubles.

Actual GameScene native audit uses platform/checked-engine doubles and actual
GameScene/scene_callback/session_render source:100 lifetimes, nine-stage ordering,
offline/config/status/suppression/full-viewport gates, raw NaN/negative-zero near/
far payloads, competing scenes, replacement/retirement, six endpoint families,
primary callback versus secondary engine error and400 constructor failure/retry
prefixes. The initial failing assertions exposed the three defects above; final
runs pass. Native64 checks do not claim original32-bit ABI or engine execution.

Strict i386 compile+link instantiates full actual World construction/destruction,
module publication, VM commands, special/procedural services, DATA snapshot,
owned GameUiSceneCallbacks/GameRenderStages and lifecycle→Scene78. It is not
linking only declarations. The existing application compile/link also passes.
Windows outputs and original binaries were never executed. Temporary audit
sources/binaries are fingerprinted then deleted; concise logs/static instructions
are retained in ignored WORK. Ledger adds one **partial** frozen004871A0 entry
and expands existing00416E10/00487240 coverage. Absent004870A0/004888C0, borrowed
engine calls and diagnostic/exception guards receive no new function credit.

No other independently actionable RENDER item remains in this required call
graph. P22-RENDER is **blocked, source incomplete** at the original per-volume
unwritten3C/40 first read. This is a retained RENDER boundary; it is not reassigned
to UI/FRAME/OWN. UI's unmerged consumers, FRAME scheduling/camera/effects order,
OWN activation and later runtime/desktop evidence remain external. The user's
strict original-behavior decision rules out synthetic history. Continue the newly
authorized P22-FRAME work separately; do not enable application mission loading.
