# GEffects1 loading and retirement — source checkpoint, 2026-11-10

Full first P2.2 checkbox remains open. No application world, first frame or new
desktop acceptance. [Receipt](checkpoints/p2.2-geffects-module-loading-20261110.json).

## Shared process and registered receivers

`EffectsModuleLoading` borrows the SAME Startup registration1, frame/actor store,
files, mode, shared CRT random, mission/detached camera, FPV and live profile.
Startup now owns EffectsLoadingProcess; GameWorldScene MUST receive that process
and borrow its environment and fade. Recreating a world no longer silently resets
these native GEffects process values. The shadow/Human/script/FPV consumers share
that exact environment/fade; constructor identity checks reject copied services.
Frame-bearing queues remain with the actual world facade and retire before scene
reset. This change is source-only; GameApplicationState still declines loading.

00488560(false): reset00488B80, load00482A00 volumetrics, refresh00482980(true)
AGAIN, then load00484260 mesh movie.004885A0(false): reset, clear volumes00482D70,
clear movie00484690→00484AE0. True calls return. Existing positive volume/movie
read/resource bodies and real engine endpoints are reused; no second actor store.

00488B80 resets RGB2F4/2F8/2FC to1, releases frame-fade200 records/storage,
then clears the Human vector228 (begin22C/end230, capacity retained), resets
flag220, sun1B0 and24 DWORDs25C..2B8. This confirms the Human queue's real module
reset, beyond the prior source lifetime guard. Positive fade-in210 and pose304
producers remain absent; no populated restoration handler is fabricated.

004885D0(true) enumerates borrowed birds. False enables BOTH actual scene1F8 and
200 sectors via94(1), resets, finds mirasun, links to root with preserve-world1,
updates and saves its local position. Bird enumeration on false occurs only when
!(mode&1). Load actual water settings and apply their reached effects; always load/
initialize decorations. Set2D4/2D0=0 and byte248=true only after those operations.

004888C0 first disables the SAME detached controller on BOTH passes. Its48 is
initialized false by00462650;00462770 query/00462AE0(false) is then a genuine RET.
No active toggle producer exists. False restores sun position, resets queues,
retires borrowed birds, restores water frame visibility and scene state, clears
actual decoration owners, writes2D4=0/2D0=-1000/248=false, resets scene water and
24C=false. Special-frame2E0 and water-camera1AC/pose304 are genuinely cold null/
empty; populated camera retirement rejects. Destruction uses checked fallback
cleanup and avoids repeating explicitly completed finish/unload callbacks.

## Borrowed birds, distinct from generated Fowl records

0047A790 scene enumeration mask80 uses lowercase NAME(E8), bounded256 bytes,
substring ptacekletacek.004797C0(false) draws shared CRT twice (timer40%5000+5000,
FLOAT48%20000+3000), sets speed5/state3/cold flags, preserves-world root link,
uses actual local position/rotation and publishes its own Bird EC pointer.
Object03/Object01 children are looked up with FFFF and hidden. A previous EC is
accepted only if it belongs to the SAME actual borrowed-bird owner; foreign
metadata rejects before reinterpretation. Actual host compilation caught/fixed
the native XYZ position setter versus stored four-component pose distinction.

004799E0(false) clears matching EC, restores XYZ/quaternion, hides/updates the
model, then transfers it to the actual deferred return list.0047AA10 clears birds
in reverse, then shows/updates deferred models in reverse. Extra retained source
frame references guard lifetime; they do not claim original ABI equivalence.
Generated flag60=true Fowl records still belong to their Fowl actor and use the
existing FowlBird owner; they are not appended to this borrowed global manager.
Mode0 Fowl transfer still requires its real producer and remains rejected.
Borrowed bird movement/spawn/runtime update/rendering are not supplied here.
Existing004797C0/004799E0 credits are extended, without double counting them.

## Water settings and concrete loading/finish effects

00489C60 zeros41 DWORDs at10C (A4 bytes), enters the first chunk WITHOUT an ID
test and reads version. Only version1 enters the next chunk; payload>=A0 reads
EXACT160 bytes and leaves the extra four bytes zero. A short settings payload is
not partly copied. Missing file/resource and unsupported version preserve zero
settings. Serialized scalar bytes are retained; enabled10C, scene-water10E,
name128/name168 and1A9 visibility policy are decoded for the real consumers.

When enabled,004885D0 saves actual active camera120/124/128 into11C/120/124 and
clears1A8. Optional10E invokes actual scene11C(0,0). Find a_nsea_/a_undw_ with
mask4 plus actual driver-path fallback; apply their show/hide BEFORE looking up
amb/sea_amb with mask2/fallback and applying show/hide. Name amb at00839754 was
verified from original bytes. Finish uses scene-only FFFF lookups for configured
above/below names, restores four cached frames in order, clears borrowers, and
00486520(true) reverses actual scene1FC child3C/40 queries and visibility calls
when10C AND1A9 are true. No substitute menu camera or water renderer exists.

## Decorations: definitions, spatial byte mask and cold grid

0047C530 opens actual decorations.bin. Root ID is the format version; >1 reads
width4, copies height8 and densityC, then sparse group index + record chunks.
0047CC90 reads entry count, version>2 group10, >8 group14; entry string chunk,
weight/byte11, >3 field4, >4 byte12, >5 field40/14, >6 field18, >7 byte13.
Raw serialized byte flags retain ALL bits. Group defaults .95/.5 and entry
weight1,14=.2,18=1/flags follow actual constructors. Group replacement precedes
record parsing, preserving native ownership. Unknown chunk IDs are not invented
as validation requirements; overflowing indices/counts/truncation reject safely.

0047C930 then opens decorator_sl.bin: first chunk ID ignored, width8C/height90,
step88, origin74/78/7C and scalar80, then width×height bytes.0047D0C0 releases
the byte buffer while preserving metadata. Bad dimensions/short reads reject.
0047B0B0 walks sparse groups/entries in reverse, lowercases names, reuses or
creates the SAME definition objects and binds entryC; nonzero40 appends an actual
entry reference. These definition constructors contain names and empty template
vectors; they do not load engine models during this native loading stage.
0047E0F0 allocates width×height cold cells, initializes0/modelnull/identity-1/C0/
flag10false, resets cell coordinates100000 and actual scalar state, captures live
FPV58 OR59. Profile quality4 controls the actual enable flag.0047B010(false)
walks existing cell models in native y/x order; cold cells are genuinely empty.
Source offers no successful grass/template/cell-generation or rendering API.
0047B090 retires grid cells, entry/group/definition owners and byte mask in the
original phase order. Positive runtime requests/templates still need producers.

## Evidence, inputs and verification

WORK=analysis/work/mission-world-20261103. Read-only exports/logs:
remaining-module-world-boundary/, version-and-decoration-module-dependencies/,
remaining-module-enumeration-callbacks/, geffects-retirement-owners/,
geffects-cold-owner-dependencies/, decorations-loading-required/,
decorations-cold-defaults/, geffects-finish-scalar-receivers/,
geffects-global-finish-consumer/, geffects-movie-unload-receiver/.
Static geffects-module-finish.asm, geffects-module-unload-tail.asm,
water-child-visibility-abi.asm, decorations-grid-initialize-call.asm and
module-loading-retirement-map.json preserve exact original calls/slots.
First plain export stopped at absent004888C0; subsequent transient read-only
functions exported successfully. Inventory/project were not changed.

Source ISD0 extraction found positive BASE inputs: Tutorial water180/mesh71/
volumetrics978/decorations667/decorator_sl97405 bytes; ALPS1 decorations396/
decorator_sl350447. Tutorial grid87²,95 sparse slots/13 entries/10 definitions,
mask279×349; ALPS1 grid50²,93 sparse slots/7 entries/5 definitions, mask791×443.
`geffects-archive-census.json` records file hashes and archive provenance.
ISD1 language/Patch could not be decoded by the existing research tool; they
remain UNKNOWN. These BASE fixtures do NOT establish live runtime precedence or
file absence; production still uses actual initialized rw_data/loose overrides.
Original assets were only read. No original/DLL/Proton/Windows fixture ran.

Focused native ASan/UBSan/LSan: actual water/decoration/mask readers, definition/
cell ownership, malformed/missing inputs and quality/repeat; positive borrowed
Birds and all17 reached frame failure prefixes. Combined actual GEffects source
composition checks BOTH passes, reset of real frame/Human fade queues, sea
visibility before light lookup, sun/water/child visibility, shared process
fade/environment across repeat, retirement and ALL80 reached frame failure
prefixes. Files/frames/camera/profile are explicit doubles; positive volume/movie
endpoints deliberately reject in this fixture because those unchanged bodies
were already separately verified. No test claims an integrated loaded world.
Strict actual full i386 Human/VM/SOUND/GEffects/ShotMgr/Versions/Particles graph
and actual GameWorldScene process constructor compile; application link passes.
Temporary audits/binaries/transient Java hashed and removed; logs/research kept.
21 new partial bodies, no complete promotions:34 complete/1,936 partial(1,931
client/five engine)/18,783 frozen.27 tracker tests pass. Continue registered
module dispatch/retirement, containing session/loader and first camera/update/
render/failure/repeat. Private guard observations remain unresolved.
