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
reference is vtable008102A8. The live virtual caller/configuration producer is
not yet bound by the reconstructed VM/application. No enable call is invented.

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

UI base GameUiPresentation::command_before_scene rejects004B6720. UI9ff6d0d
contains later command geometry/full HUD dispatch; inspected through Git only,
still unmerged. Its GameUiRenderStages seam is unchanged. UI's remaining Human
order, occupied actor/FPV and console producers remain its own contracts. No UI
code copied, receivers suppressed or successful replacements installed.

OWN must install/activate the lifecycle and caller; FRAME owns render-before-
update scheduling/camera simulation. VM/configuration must supply actual virtual
GEffectsB8 calls when reached. Empty current textures do not prove that future
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
