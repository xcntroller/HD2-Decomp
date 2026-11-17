# Loading decorations and late-spawn retirement — 2026-11-17

Base e3f3a84, feature/p22-frame. The user's ef21bbe0 candidate passed HUD timer
binding and reached the end of the loading bar. Activation then failed with
`Decoration update requires actual0047BCC0 spatial/model producers`. Rollback
logged live collision descriptors, and final owner destruction faulted at rebuilt
0051F722 in HumanColdCommandControl::clear_empty with null ECX. This is a rebuilt
client address, not original0051F722. Exact executable and three logs are retained
in WORK/loading-decoration-crash; symbol mapping and original evidence are in the
[receipt](checkpoints/p22-frame-loading-decoration-20261117.json). That failed candidate did not activate or complete a gameplay frame.

## Correct the actual retirement cause

Original00611C90 directly activates prepared Humans AFTER Scene2's captured module
activation pass (authority first, then late presentation mode gate). Source
GameWorldActors::activate_human already journaled module-driven activation, but
these direct HumanSceneActor::activate calls bypassed it. Scene2's source partial
activation guard consequently skipped their derived retirement. Original00476260
then removes reserved slots, destroying their command/storage owners while the
external HumanLoadingComposition map still retained them and their collision
borrowers. Later containing-owner destruction called into the already destroyed
Human. Null-checking the command would leave the ownership defect intact.

HumanSceneActor now records the actual entered pass before calling its required
containing lifetime, including a receiver that mutates then throws. It still
rejects an absent lifetime before recording entry. Both genuine mode passes and
existing last-pass map retirement remain unchanged. This lets Scene2 retire the
same B0/DD0/inventory/collision composition before deleting its reserved Human.
No opposite-mode pass, synthetic actor, skipped teardown or swallowed failure is
introduced. Native ASan/UBSan/leak checks exercise the actual Human dispatcher,
SceneActorRegistry and deactivate_human_loading_owner with explicit lifetime/frame
endpoints: late-spawn, modes1/2/3/7, success, both throwing activation passes,
required missing lifetime, and lifetime destruction before actor command deletion.
61 checks passed; full production Human composition is compile/link only here.

## Restore the required decoration loading path

Original0047BCC0 checks profile4, enable state, then the group vector at98/9C
(previous notes incorrectly called that the definitions vector). It captures the
actual scene15C listener, conditionally requeries the current Player after the
actual detached-camera getter, stores elapsed, calls grid0047EB80, then request
update0047F150. Nonnull dynamic mesh deformation remains a required boundary.

The reconstructed grid implements native signed truncation plus decrement on
EVERY negative coordinate, fractional strips, both rolling offsets/wraps, and the
large-move complete rebuild/early return.0047E930 visits columns ascending and
rows ascending. Each0047E2D0 seeds the SAME process CRT using native trigonometric
cell coordinates and density, draws Z then X jitter, and performs the actual
static World ray from Y100 downward500 with exclusions FFFFFFFF/210000. It
retires the prior cell model only after that query. Actual hit material and slope
select the existing group; absent/unsuitable ground genuinely releases a model.

0047B500 uses native cumulative random weights. It returns pooled models in LIFO
order; the final retained template is cloned rather than consumed. First loading
uses the exact entry resource, records the native per-entry failure latch, binds
the actual scene root, configures the first actual visual/LOD/group material, and
inserts a retained template before cloning.0047E240 shows the acquired model;
0047E290 hides and returns the SAME model to its definition pool. Actual position,
orientation, scale XYZ writes, update, material flags/setters and mode-gated
model50 opacity use existing engine endpoints. Model EC is the native base-opacity
float, distinct from Scene2's ActorFrameData allocations. No model substitution or
successful missing-resource service is supplied.

0047D110 samples the actual byte mask bilinearly with its native1/256 factor.
Coordinates outside the owned allocation fail explicitly instead of reproducing
an undefined native read. Native first-person/posture transition bookkeeping and
stationary nonprone Player RET are recovered. Actual positive Player bending,
vehicle wheel bending and dynamic wind mesh capture remain explicit required
errors; no request records are fabricated and no source-complete claim is made
for those producers. This is the reached loading grid, not full grass interaction.

The host adapter borrows the SAME World tree/buckets/frames/Players/camera and
process flags. GEffects owns its receiver; actual live models retain their engine
references while borrowing those Frames until normal/partial module retirement
clears the process pools. No world/readiness/activation flag is manufactured.

## Focused verification and next execution boundary

The installed Tutorial definitions have an87x87 grid, four material groups and
zero nonzero wind modes. Read-only decoding verified all ten named4DS resources
have actual geometry, LOD0, groups and material data. Native checks parse the
actual definitions/mask bytes, exercise all7569 populated cells, positive model
acquisition/reuse, enable/disable, both directions and wraps, negative exact
boundaries, large teleports, no-hit recycling, failure latch and throwing-query
prefix cleanup. Mask sampling uses a nonconstant quad in the actual installed
allocation; missing/invalid samples and positive contact failures remain honest.
22,868 ASan/UBSan/leak checks passed with explicit ground/model/engine endpoints.

Full actual World/Startup/Human/FRAME/application/probe i386 O2/Werror compile/link
passed and a revised isolated candidate was staged without execution. The receipt
records exact hashes and the recoverable complete review overlay. Temporary
native fixtures were fingerprinted and removed. OWN dirty source and its earlier
staged binary remain unchanged. Actual engine/Proton/Windows execution is solely
the user's next desktop test. FRAME/HUM/OWN remain partial; no new inventory or
whole-function credit, completed mission, presentation or desktop claim.

WORK=analysis/work/p22-frame-closure-20261117. Research index preceded each new
export batch. Read-only ABI overrides are retained with raw original assembly;
decompiler stack/register artifacts were checked against raw calls and literals.
The earlier failed ABI script-path invocation produced no usable evidence; only
the successful final export directory is cited.

## Actual revised user run

The user subsequently tested cb8b636c and reported a crash after Space. Its
actual logs show loaded14=1/status5 activation, SAME-World FRAME binding, then
completed Scene78 render and fixed-update calls. The process then handled
`Game input requires the actual two mutable64-word controller views` and shut
down with result1. Startup reports63 bindings and63/63 output words. The old
decoration failure, Human null fault and collision-retirement errors are absent
from this run. Exact binary/logs:WORK/loading-first-frame-input-return. This
establishes real owner activation and calls, not accepted scene presentation,
input/update, a stable playable frame or complete desktop delivery. Next work is
the original input output-allocation/64-word consumer boundary; do not disable
the guard or fabricate an extra binding.
