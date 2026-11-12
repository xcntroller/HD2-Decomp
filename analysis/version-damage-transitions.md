# Versions damage transitions — source checkpoint, 2026-11-12

This extends [cold Versions loading](version-module-loading.md) for reached
Human/Car/blast frame contacts. Full P22-HUM reactions/entry/frame and application
loading/desktop acceptance remain OPEN. [Receipt](checkpoints/p2.2-human-versions-transitions-20261112.json).

## Shared owners and lazy resources

0043C6A0's input is the SAME Versions Master from frameEC word5. Traverse parts
backwards, call0043B000, accumulate false and recheck the global Master slot after
EACH callback: it may delete both Part and Master. Empty/deleted slots remain
NULL; no swap-last compaction or copied registry. Checked source borrowers compare
owned pointers before dereference. World and Car contacts bind the actual registered
Versions7 AFTER module construction; both links unbind before that owner retires.
Blast's historical `blast_physics_impulse` name now calls this actual receiver.

0043B000 writes optional current frame BEFORE health work. Signed damage strictly
below remaining health subtracts with DWORD wrap and returns AL1. Otherwise lazily
0043CCA0 loads resources, preserving native partial fields on failure. No resource
resets count/stage0 and calls0043C710, then returnsfalse. Already terminal stage
returnsfalse. Advancing subtracts current health, increments stage and loads the
NEXT array20 health until remainder is below it or count reached. Array20[0] does
not overwrite scene-produced initial health.0043BA60 replacement precedes optional
frame output, module7 packet(master16/part8/stage8), presentation0043C040 and final
stage14 publication/0043C710. Required failures retain their preceding mutations.

0043CE40 strips the LAST dot from actual model134, then the LAST backslash, and
prefixes literal `d` (00832CD8=`d%s`). Original128-byte overflow rejects. Borrow
SAME existing category5 LoadedResources cache, search directories, entries,
scratch, capacity and reference policy; native open flags10. Existing generic
cache.get now accepts optional flags(default0); no changed flags for old callers.
Publish retained Master0C before actual0041D920 Script death event when modelEC1
is nonnull. This is DEATH, not initialization. Script authority/context/runtime are
shared. The gameplay-save suppression global has no exposed positive producer;
that future save/restore behavior remains outside this source route.

0043CCA0 finds resource frame by current part short name/maskFFFF. Each chain
frame supplies case-sensitive `version` default100 and `sound` default-1.
Reverse child scan0043CE10 retains the last visited kind6 debris frame and kind1
next frame. Append four separate arrays in native order40/30/20/50, then follow
next untilNULL; only then publish count5C and bound18. Children/arrays borrow the
retained resource model. Cycles reject safely instead of infinite native looping.
Raw prepares0043CE10's stack output BEFORE engine GetChild; it does not change
the existing engine+40 two-argument ABI. Dynamic ESP offsets also explain the
three CRT draw spills in0043B140; they are separate Z/Y/X draws.

## Frame replacement, effects and retirement

0043BA60 uses Driver50 old visual subtype and clones the selected resource visual.
Link to old parent mode0, direct local position/lock flags, existing exact local
forward normalization/quaternion gate, model1C(direction,0), copy local scale,
quaternion/flags and update. Publish SAME Master/Part metadata on new visual,
clear oldEC5/6, add new/remove old from actual model, clear old name and update.
Move PROJ children to new parent mode1. LMAP→LMAP copies current material/selection
fields, calls actual engine lightmap receiver with up(0,1,0), then updates. Release
old frame BEFORE publishing new owner and resetting new PROJ receivers. No separate
frame-data store. Native unowned-model branch leaks its new visual after metadata
changes; source releases factory reference while the shared sidecar keeps frame
allocation alive until its own retirement. Engine factory failures use existing
required source guards; no Windows/engine execution equivalence is claimed.

0043C040 ignores nonadvancing stages; negative stage means visual count. On
presentation-only, or mode7 without effects, replace visual. Other authority
branches use the already replaced frame. Effects request actual0043C000 particle
at current world matrix, parentNULL/orientationtrue/flagfalse; prepare debris even
without emission; sound0043B9D0 uses fresh actual request group7/flags1/null source/
relative, default voice4, current world position and signed entry(skip-1). Native
presentation selector is true for reason0, false for2, otherwise masterID%3==0.
SAME Startup particle process/SOUND/session owners remain borrowed.

0043B140 first aligns debris-group parent with current part world position,
normalized WORLD forward through0C, world/local scale gate, rotation/flags/update.
Effects emit kind1 children backwards: create matching visual subtype, clone,
clear visual2000, retain original world position before root link, publish pose/
scale, update/place/update. Actual shared Physics creates type4, first-list/sight
true, native record10=part material, native18 remainsZERO. The source construction
record now retains material10 distinctly. Consume three SAME CRT draws Z/Y/X,
range1.5 minus.75, before world/master reads and optional impulse. Force is three
times offset from Master plus random; point is fragment position plus random;
scale1. Factory reference retires after Physics/impulse. Physics/fades/handles are
existing owners, not a Versions simulation. FLOAT stores/x87 intermediates follow
raw; unwritten fourth vector lanes use source guard0.

0043C710 marks changed28 BEFORE actual0041DBB0 hit event, removes terminal Part
leaving NULL slot, refreshes collision and removes Master if no part remains.
Refresh recomputes bounds/radius/centre, copies current world/inverse and moves
SAME descriptor. Unlike initial construction, native refresh does not normalize
local scale or multiply radius by world scale. Invalid bounds radius becomes0;
undefined native centre retains prior source position. Part0043AEF0 clearsEC,
releases frame, THEN frees50/40/30/20 arrays. Master0043C1A0 retires reverse parts,
resource0C then model08.0043C320 retires/appends collision, clears metadata and
queues changed model for reload.0043E2C0 retires reverse Masters, reverse deferred
collisions, THEN reverse borrowed model reloads via actual004290A0(model134,0),
ignoring negative loader result as original. No extra owned model/cache graph.

0043F1E0 is absent frozen and earns ZERO credit. It returnsfalse without consuming
if !modebit2 or NULL packet; otherwise reads SIGNED16 master, SIGNED8 part/stage,
then rereads authoritybit1. Presentation-only checks indexes as unsigned, skips
missing slots, calls0043C040(stage,true), reloads actual global slots and invokes
0043C710. Modebit1 and invalid/missing indexes still returntrue after32 bits.
The native NULL-Master dereference after failed resource retirement rejects as a
required source error, preserving the failed prefix instead of using freed memory.

## Evidence and verification

WORK `human-containing-versions-*`: indexed prior originals, new workers/resources
C/JSON exports, bounded raw damage/bind-resources/master-transition/replacement/
debris/receive assembly. Read-only Ghidra and heavy compilation were sequential.
Existing C exports retain indirect-engine stack aliases: bounded instructions,
RET counts and existing verified engine ABIs establish reached argument ordering.
No original EXE/DLL/Proton/Windows outputs executed.

ASan/UBSan/LSan PASS400 actual Master/Part/ActorFrameDataStore/DynamicCollisionBuckets
lifetimes across1/2/3/7, health/lazy chains, stage/packet/failure prefixes, metadata
clear before reference release, terminal Part/Master deletion, contact borrower
rejection and final reload ordering. Actual generic resource cache checks flags10,
unchanged default0, callback14, hit reuse and full resource release. Actual debris
producer exercises record material10/default18, create→CRT→impulse and disabled/
missing-handle branches. Frame/resource/engine/script/effect/Physics endpoints are
EXPLICIT doubles. Strict actual full World/Car/blast/Versions/VM/cache/Physics/
particle/sound/module i386 and launcher/probe compile/link pass. Source-only;
no native full World trajectory, engine execution, live transport or desktop claim.
