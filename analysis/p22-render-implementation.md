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

GEffects::render_before_scene now invokes its SAME C4 VolumetricLights with signed
bits of process224.00482260 receives a REQUIRED separate procedural process
receiver, invokes it before driver3=1/camera/traversal even for an empty population.
This is callable composition, **not a supplied positive procedural receiver**.
There is no empty successful implementation or forced texture/enable flag.

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

## Exact remaining work and dependencies

00482E50 positive geometry is still RENDER-owned work. Its current C export
loses x87/stack values (local280/local274/local294/unaff_ESI); reconstruct those
from instructions before translating geometry. Recover00483E50/00483DE0 and
configuration00485B50 over process0089B63C, textures clouder.tga/clouder2.tga
flags502, buffers0x3CC00. This is unfinished work, not a UI blocker. Do not install
a concrete GameUiRenderStages until that receiver exists.

0079B760 is also open RENDER work. New instruction inspection establishes reverse
actual actor registry0089B3A0 traversal, type3,005A5D30 gate, HumanA0→DC0, frame
08 epoch11C==Scene11C; at most21 qualifying records through0079AC40. It allocates
process buffers3980(alignment16)/730 and draws via driver30 only for nonzero WORD
count. Required positive0079AC40 geometry and actual DC0 producer must be checked
against current Human source. It is not an arbitrary debug triangle service.

UI base GameUiPresentation::command_before_scene rejects004B6720. UI9ff6d0d
contains later command geometry/full HUD dispatch; it is unmerged and was read
through Git only. GameUiRenderStages is unchanged there. UI's remaining actual
Human order sources, occupied actor/FPV and console producers remain its own
contracts. No UI code copied, receivers suppressed or successful replacements.

DATA's observation store/scopes and volume draw observation already exist in
base. Actual camera selection now records an execution event via SAME frames.
A selected camera event is not a completed render. No manually invoked hook or
fixture population is claimed as actual mission observation. Historical selected
bytes, static requirements, loaded populations and executed branches stay separate.
OWN still installs lifecycle/render stages; FRAME still schedules camera/update
and render-before-update; desktop acceptance remains user-operated.

## Verification

Temporary native ASan/UBSan/LSan audit passes positive loader→visibility→pairwise
swap→existing draw leaves with procedural/engine doubles and explicitly seeded
known history.25 injected operation positions include restoration failures;
restoration is attempted on every path, but a failing restore endpoint cannot
promise restored state. Unknown history explicitly rejects and restores states.
No resource/procedural positive or composed UI runtime claim.
Strict i386 concrete World/GEffects/frame endpoints and lifecycle→Scene/callback
entry compiled/linked; procedural is explicitly a throwing audit double. Existing
application-launch compile/link passed. Outputs were not executed. Temporary
sources/binaries fingerprinted in receipt and removed; logs retained in ignored
WORK. No ledger credit added pending full containing/history boundary completion.

Tracker validation remains blocked by existing absent ignored evidence
`analysis/work/multiplayer-rule-caret-20261101/rule-refresh.asm` (0062D880);
ledger unchanged. Diff check passes.
