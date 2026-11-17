# P22-HUM H8 handoff — 2026-11-17

Actual worktree `/home/control/Documents/Projects/HD2-Decomp-p22-hum-h8`, branch
`feature/p22-hum-h8`; ALWAYS explicit workdir. Environment cwd may be retired
`.codex/worktrees/3472/HD2-Decomp-p22-hum-h8`/feature/p22-hum-h8-4. Preserve merged
H8.4 and other worktrees. PACK is finished.

USER asleep/unavailable: no questions, continue ALL H8 including merged H8.4
until complete or hard block on ANY progress. When H8 SOURCE-complete, sweep
P22-HUM BEFORE marking matrix resolved_source; then resume existing
`../HD2-Decomp-p22-frame`, feature/p22-frame, if still absent. Inspect checkout
first. No push/original EXE/DLL/Proton/Windows-fixture execution. Heavy compilation
and Ghidra SEQUENTIALLY, poll completion before next heavy job. No subagents or
permanent tests. Commit verified larger source checkpoints and CONTINUE. Source
commits need no invented desktop test; interactive delivery requires actual user
acceptance. H8.1/H8.3 source checked; H8.2/H8.4/H8.5/H8.6 and HUM matrix OPEN.

## Latest verified checkpoint

[Dog frame adapters](analysis/human-h8-dog-frame-source.md),
[receipt](analysis/checkpoints/p2.2-human-h8-dog-frame-20261117.json).
Actual Dog580/590/5A0 history005D64E0, Body turn005C89C0, gravity005C62C0/6250,
actual multisphere support005C6380 and signed movement limit005C8B10 now exist.
World/Runtime Dog Sight uses same optional544 marker; World SOUND uses actual
Body3EC minus snapshotted Actor position and fresh model-relative pointer.
Native1641 strict ASan/UBSan/leak checks PASS. Actual strict full World/i386
module + these APIs and forced O2 launcher/probe PASS (compile/link only).
Controller is an explicitly throwing compile-fixture endpoint, no live Dog
activation/controller/frame/motion closure claimed.6 frozen partials;34 complete/
3668 partial(client3663+engine5), frozen18783. Tracker exact+27 tests PASS.
Temps fingerprinted/removed. No heavy job running. Continue; no user questions.

Prior motion/bootstrap checkpoint e7e1635


[Dog motion/transform/Body bootstrap](analysis/human-h8-dog-motion-bootstrap-source.md),
[receipt](analysis/checkpoints/p2.2-human-h8-dog-motion-bootstrap-20261117.json).
Actual Dog owns clip005C71E0 displacement,005C7600 deceleration/getter005C75B0,
Body005C0F50/0D10/0D60 transform and005C8370/7D30 bootstrap composition. Shared
3E0 flags, forced-motion owner, five descriptors, eight marker lookups and SAME
seven damage-bone slots. Cold278/27C/280 priorities remain honestly unproduced;
bootstrap finite-position postconditions are proven independent of old3A0, not
an additional D11 default. Native319938+123365+9563 strict ASan/UBSan/leak checks
PASS; all21 bootstrap endpoint failures tested. Strict actual World module link
and forced O2 launcher/probe PASS; Windows not run.7 frozen partials, no complete/
absent/engine credit. Tracker exact +27 tests PASS;34 complete/3662 partial
(client3657+engine5), frozen18783. Temp audit pairs fingerprinted/removed.

Prior [collision/clip](analysis/human-h8-dog-collision-clip-source.md) checkpoint
8c743cb: actual Dog owns five same-grid descriptors, same-model mixer and actual
parentEC callbacks. Human Sight exclusions borrow same first two descriptors.
Native38153+309 checks and actual module/forced launcher PASS. Native names
`col dummy1`,`col dummy2`,`col dummy` (spaces, third unnumbered), Body404=`w`,
Body3EC=`d`. Do not repeat unchanged audit. No live activation enabled.

Prior checkpoints: a31f4dc [Body actions](analysis/human-h8-dog-actions-source.md)
(4575872 qualified checks, actual queue/priority/death/ground/playback/effects);
456e7fa [Human command/Active firing](analysis/human-h8-command-fire-line-source.md)
(selector22, removed invented pending whitelist, actual Active114 infinite line);
96182e2 [actual Dog orders](analysis/human-h8-dog-orders-source.md);27a5d89
[shared blast/Sight](analysis/human-h8-blast-sight-source.md) and [H8.3 closure](analysis/human-h8-projectile-closure.md);
5827965 [Player cover](analysis/human-h8-player-cover-source.md),6dbb440
[Player dispatch](analysis/human-h8-player-dispatch-source.md). Reuse these sources.

## Next — finish live Dog/remaining Human owners

Next concrete dependency is actual selected controller/activation and containing
frame collision-motion005C3420 plus005C43D0/2260/26D0, then remaining selected Human owners. Body and
clip initialization source now exist in dog_body_state.h/dog_body_transform.h;
use them rather than reimplementing. Runtime still fails live Dog entry.
Actual direction/body-action emission adapter and controller owner are needed.
Full005C3420 C is in human-h8-dog-body-dependencies (large positive motion body).

WORK=`analysis/work/mission-world-20261103`. Reuse h8-4-dog-{boundary,required,
control,live,typed-v2}-originals; human-h8-dog-{body-dependencies,
controller-body-dependencies,live-controller}.005C7D30 C is under REQUIRED,
005C8370 under BOUNDARY. Retained aligned motion/deceleration/transform/direction/
bootstrap raw and exact constants are linked in the current contract.
Original00811FBC=-10000.099609375 FLOAT, not timer-10000.

Fresh READ-ONLY containing motion exports are retained under
human-h8-dog-motion-typed-v2-originals, PASS7 bodies. Script
ExportH8DogMotionTyped.java aborts on absent starts and rolls back all changes.
Original typed-v1 had WRONG fastcall query/exclusion declarations; retain as
NONAUTHORITATIVE provenance. v2 correctly uses0071BAF0 thiscall(request), rays
007183A0 thiscall6args/sweep00718530 thiscall7args;006EFB10 thiscall(array,count,
NULL),006EFA70 thiscall(NULL,NULL). Typed v2 aliases are improved but x87 return
casts and register/stack-vector names still require raw instructions, especially
005C26D0 constructor/rotation helpers. Do not create absent original starts.

Full005C3420 existing C is495 lines. Reuses actual CollisionMotion<Hit> and
advance_standing_collision_motion (0071BAF0), SAME World human_geometry_ray/
sweep and SAME descriptor exclusions. Need complete setup/step/lateral/ground/
direction response, not another private solver. Read all005C43D0/2260/26D0.
005C6380 is now implemented against request.contacts with ALL contacts consumed,
not Body fields.005C62C0 pure copied-speed integrator and005C6250 actual FLOAT
speed/phase producer are now implemented, no need re-audit them.

Raw005C3420 scratch: ESPB0/B4/B8 local270/26C/268 normal is read by4001 and
passed to005C43D0; provenance NOT yet classified. Constructor00401000 initializes
10 records atrequest+100; main repstos zeroesrequest200 only. Do not blindlyzero
this separate local normal/flag or infer global unproducedness from C names.
Need aligned full raw before coding. Current partial objdump ranges starting
inside instructions are NOT authoritative; re-export from real aligned start.
Earlier Body278/27C/280 unresolved priority provenance also stays explicit.

Native core details: ActorBody+90; clipBaseBody+10=ActorA0, clip model10=ActorB0.
Body160 is actual Actor borrower. Body364=Actor3F4 action. Seven damage bones
Actor478..490=`f,d,c,h,p,t,l`; bootstrap lookup order differs and includes extra
`w`. Clip marker callback `a`:005C7960 and005C79B0 are absent frozen entries;
raw callback ABI is stdcall(Frame,event,text,time), return0, actual parent108→EC→
Actor then+A0. Event queues have THREE words before count; fourth write overlaps
counter. Current source guards that native-invalid state, adds no absent credit.
Clip bind uses alternating model slots0/1 and2/3. Tick first elapsed is unused,
second signed elapsed drives all; first four engine deltas scale UINT32 bits by
rates then64-bit __ftol/low32, last four use signed delta. Sum keeps exact signed
integer through comparison, FLOAT step/sum feed blend helper. Raw unordered
elapsed proceeds; normal positive-duration and falloff-weight gates are ordered.
Host dog_model_weight preserves ST0. Do not replace with a float-spilled getter.

Native Body278/27C/280 priorities and new queued metadata can be read before
known production. Current optionals/provenance keep failures honest; common
005C5030 tail produces priorities0. Do not silently zero/extend D11 defaults or
claim global absence of aliases/writers. Body initialization must preserve the
actual callback and early-return order. Sound producer is actual default request,
Body3EC relative position, actual model relative, group11/sourceNULL; see raw.

Live controller graph already researched. human-h8-dog-live-boundary-graph.json
is STAGE1 direct calls only; fingerprinted, write a NEW revision if updating.
Kind4/5 vtable native0..70 is retained; do not extend kind1/7 beyond68 into next
table. Mixed controller-body export ended ERROR at absent005CF0B0;005CFA00 wasn't
exported. Both raw JMP005CE110, actual virtual6C needed within005CE190. Bind04 is
005D5B00→005CE5D0, not005CE110.005D5890 publishes NEW534 then ALWAYS invokes new.virtual00(actual Actor);
005CE630 stores Actor10 then virtual04→005D5B00→005CE5D0. Only if OLDnonnull,
new.virtual10(old) runs before old.virtual0C/08 close/delete.005D5390 common first, reverse tracks, controller0C before
native NULL check, Body retirement, queues/Sight/SOUND/cache/property/pose restore.
005C0F50 has ONE pointer (pushed earlier005C8069).005D64E0 retains native zero
previous-elapsed division. Activation bone b..y mask400 early return still falls
through caller Body init. Respect every reached required callback.

Other conditional owners: type34/36 look/target/sound, Aircraft20, nonHuman EC,
contact14/5 actor58; non7 Remote/transport/visibility/fall/respawn, debug A8 and
Animation ancillary wings/non7. Follow actual selected data/branches; no private
empty registry, fake actor or success. Tutorial historical baseline no Dog and
ALPS1 one is NOT fresh resolver/loaded-population proof. Type36 conditional
constructor/activation/frame read-only exports now exist under type36-live-
boundary-v2 (PASS); earlier v1 stopped at absent005E3EE0. No implementation/credit.
005E2CC0 actual material4C/4D continuation ray is exported; see idle-line-query.

D11 FOUR authorized corrections are source-resolved; don't reopen. PACK finished.
Source H1–H7 and prior H8 ordinary Human/Player/mounted/Car/timed/Animation/
ShotMgr are verified; no unchanged broad re-audits. H8.5 containing verification
and application/desktop still required. FRAME handoff91a7d33 has stale pre-H8
source blockers; don't blindly repeat them when moving there.

## Commands

Index BEFORE new exports: `python3 -B tools/research_index.py find ADDRESS...`.
Static image `analysis/work/hd2.unpacked.exe`, SHA2568c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.
Read-only/noanalysis Ghidra `../H&D2 Decomp 4/ghidra/support/analyzeHeadless`,
project `/home/control/Documents/Projects/HD2ResearchProject HD2Research`, process
hd2.unpacked.exe, tools/ExportFunctions.java. Check ERROR despite exit0.
Forced build: `make -B -C recovered -j1 application-launch startup-probe
WINDOWS_CXX=../.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++`.
Actual module fixtures include host/game_blast_waves.h and
host/game_mission_module_frames.h, tick both passes; compile/link only. Native
g++ ASan/UBSan/leak links+rpaths `.tools/hd2-sanitizer-runtime/usr/lib64`.
Fingerprint/remove temporary audits; tracker render/check+27 existing tests.
