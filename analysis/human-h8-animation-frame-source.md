# H8 live Animation frame and script start source

Animation damage's actual A3 now enters005B5190→005B55E0→005B50C0 on the SAME
World actor/loading owner. Original script opcode32 HUMAN_Kill also calls this
start receiver for Animation25 and the existing D4 damage owner for Human3.
This closes the previously unconditional positive Animation-start requirement;
H8 and the HUM matrix remain open for other reached dependencies.

## Original evidence

Research-index lookups preceded new read-only/noanalysis exports. WORK is
`analysis/work/mission-world-20261103`; original client SHA256 is
8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.
Reuse `animation-first-update-required/005b5190.c`,
`animation-loading-required/005b5000.c`, `script-animation-visibility-required`
005B3AC0/005B4DC0 and existing005B48B0 collision-update source.
New `human-h8-animation-live` contains005B55E0/005B50C0/005B5FB0;
`human-h8-animation-collision-query` contains006FB660. Both export logs pass.
`human-h8-animation-links/005b5550` exported successfully, but that export
script subsequently FAILED because005B3450 has no defined function start.
Its mixed log is retained, not reported as overall success. No program analysis
or function creation was performed. Raw suppression xrefs independently show
005B3453/005B36F1 bracketing008AC488; no frozen credit or persistence body claim.

`human-h8-animation-track-frame.asm` resolves the decompiler's unwritten
uStack38: push EDI changes SP, then005B5204 stores current+elapsed, which the
caller reads after the pop. Unsigned predicted time is compared with duration.
`human-h8-animation-script-{dispatch,kill}.asm`, operand-index and map evidence
resolve opcode32 via byte-table32−1→compressed case6→00419258. The map's opcode1
name is overwritten by a predicate-name collision; its byte mapping is usable,
not that name. Raw0041CF50 reads operand words,00420310 indexes frame pool90,
0041CF80 resolves frame EC actor0,0041D6F0 reads actor kind18. HUMAN_Kill has ONE
frame operand; no extra Boolean is inferred from the decompiler.

## Source behavior and ownership

005B55E0 returns immediately when A1 is set. Otherwise a nonempty interaction
name resolves through actual Scene7C/FFFF into typed borrowed98; matching static
collision flags gain20 BEFORE descriptor retirement.006FB660 scans only static
shape families0..4 in family/record order, exact frame equality. No triangle,
dynamic bucket, descendant or families5..7 are added. Missing tree returns0
without clearing caller output; a loaded tree clears it even with no matches.
005B5550 restore clears20 and preserves all other flags.

Descriptor clear precedes mode2/nonreplicated/nonrestoring SOUND and particle
emission. SOUND uses flags1, group6, actual A8, actor source and particle-or-
original relative frame; particle uses AC and actual particle world matrix,
null parent,true,false. Original frame NULL then returns. Helper clip0 is
queried twice; the returned clip's member08 becomes1 before A0 is set. A1 is
set when helper exists even without clip0. Show/update helper precedes
hide/update original; actual metadata-gated Script death then selected non7
publication follows. Replication suppresses effects, not script death. Mode and
process replay state are reread at native gates, including after effects.

Startup owns the SAME process008AC488 state across World lifetimes. Its BSS
initial false is retained; the only original writer bracket is the unrecovered
actor-persistence loader. No source loader is claimed to populate it and no
per-World private replay default substitutes for that process owner.

005B5190 clears A3 BEFORE calling start(false). Active helper/A0/clip0 compares
wrapping uint32 current+elapsed against duration even while paused; only a
nonterminal tick is pause-gated.005B50C0 clears descriptors, queries clip0 twice,
reads model current time then virtual clip duration, advances by wrapping
(duration-current)-1 and rebuilds BODYCOLB from the HELPER model. A0 clears only
after rebuild returns. A2 cold-or-countdown collision updates decrement nonzero
BC before collision update then wing update. Positive ancillary wings still
require their actual owner; activation already rejects that selected geometry.

005B5000 retains common-first deactivation and native authority/mode2 skip,
clears A0/A1, retires wings, restores interaction20 before clearing98, then
retires descriptors/helper events/model. Destructor retirement does not invent
the deactivation-only interaction restore. Event bindings retire before helper
unlink/release; descriptors retire before borrowed frames/dynamic buckets.

00416E10 opcode32 resolves its retained frame slot then actual EC actor0.
Human3 invokes SAME loading_lifetime D4(4,-1,zero,zero,NULL,0); Animation25 invokes
SAME loading.start(false). Only successful supported-family dispatch calls the
actual guarded context.advance. NULL slot/EC actor/other family returns the
native base false; containing Human/Dog dispatch retains its existing policy.
Missing required typed lifetime throws before PC advancement. The command is
base lifecycle dispatch, independent of the caller context's family.

## Verification and limits

[Receipt](checkpoints/p2.2-human-h8-animation-frame-20261117.json) fingerprints
source, original evidence, successful logs and removed temporary fixtures.
Native strict ASan/UBSan/leak audits PASS:26607 qualified policy/collision checks
cover flags, gate combinations, replay rereads, A3 prefix, unsigned time wrap,
pause/equality, helper rebuild and required-failure prefixes;19 actual lifecycle/
context/Animation actor checks cover Human damage parameters, Animation start,
authority/advance gates and callback failure PC ordering. Engine/World endpoints
and the native Human damage recipient are explicit doubles; these checks do not
execute those engines. Actual full World/module authority/presentation plus
Animation activation/damage/update/deactivation strictly compile/link for i386.
The first fixture link failed because it omitted existing model_resource_key.cpp;
linking that actual existing source resolves it. Launcher/probe are rebuilt
sequentially after heavy compilation, with no Windows execution. Exact tracker
and27 tracker tests pass. No desktop/application acceptance occurred.

Four new frozen partials005B55E0/005B50C0/005B5550/006FB660; existing005B5190,
005B5000 and00416E10 remain partial. No absent005B3450/005B5EB0 or delegated
engine credit. Non7 actor packet publication, persistence and escape-aircraft
wings keep named required failures; this checkpoint does not claim their
completion or establish selected mission absence.
