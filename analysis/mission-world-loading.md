# Full mission loading — work in progress

2026-11-09: [shared scene and actor ownership](world-scene-ownership.md) now
composes file stages, the same frame/table/effect owners, collision tree/grid
and Items/runtime/Physics lifetimes. Physics1/3/4 source is verified;6 still
rejects. This is source-only composition, uncalled by GameApplicationState.
Required containing session/Human/presentation/VM receivers and first frame remain.

P2.2 is in progress. P2.0/P2.1 committed as `f9674b3` after the user's
constructor-log confirmation. No loaded world delivery or desktop verification
is claimed here. The user explicitly chose to bring actor/collision dependencies
forward and preserve the full loading checkpoint, rather than a static-scene
checkpoint. Keep both P2.2 items open until their respective boundaries are met.

## Original boundary

Reuse [receiver/owner research](game-owner-start.md#ordered-loading-work-retained-for-p22)
and [concrete owner integration](game-owner-integration.md). Complete006CA780's
loading presentation006C9640/006C95D0, animation00783350/00783BC0, engine Scene
006D4180, Scene2/root/object006CBF30/006CC760/006CD940, actors006D45C0,
sounds006D4880, check tables006D4280/00523C00/0051DE80, tree006EE570,
effects00445D90 and ordered failure/unload. Actual actor init and property reads
occur during loading and cannot be replaced with opaque stored blobs. Post-load
activation006989B0 and the frame/camera consumers must remain tied to real stage
completion. No menu camera, guessed scene preview or successful placeholder service.

004476C0 is a chunk's remaining-byte query, **not an actor-manager/service lookup**.
The apparently conditional AE20/B150 branches therefore parse nonempty chunks.

006CC580 is the stdcall(frame, name-map) registration callback (RET8), recording
frame+E8's name to the map's +10 value, including replacement of duplicates.
006D4C70 is stdcall(event, float progress, word, word), RET16; events10/20 are
ignored, others update loading progress stage0.006D4B40 and006D4B50 are RET.
Scene vtable1009B798: +F4→10047D20, +F0→1004BE10, +F8→10047DB0,
+44→10046880, +EC→10047C70, +E4→10048270, +128→10049690.
No engine execution was performed. Missing standalone Ghidra functions are defined
only transiently in a read-only research session; frozen inventory is unchanged.

## Animation format

00783BC0 asks two00783710 helpers for55F8 bytes each; seven00783490 for2220;
one00783960 for189C; one00783B00 forF0:111,708 bytes total. Each large table
has786 records (two groups498/288), two DWORDs and a20-byte name in a28-byte
record. Seven middle tables have273 records, DWORD/name20/DWORD/DWORD in32 bytes.
The next has22528-byte records; last has12 names of20 bytes. Fixups00783760,
007834E0 and007839B0 load names only when strlen>4 through the existing0041FC10
animation cache.00783B40 instead checks signed first byte>32. Failures leave null
references; missing animations are not fabricated.00783350 sets its attempt latch
before calling the loader, so a later true result alone cannot prove readiness.

The base others.DTA anim.sav is111,624 bytes (84 shorter than these fixed sizes),
SHA256996c50a37b74294777d439b89991b552c87ff96d482d374d1e74f4cd82d75cf6.
Do not silently pad it, execute the original short-read/uninitialized-memory path,
or infer that runtime uses this base copy: actual rw_data overrides must apply.
The native decoder requires at least the full serialized payload and
transactionally publishes owning references only after all records decode.
The user-run runtime capture resolved the actual patched table to111708 bytes;
that input passes all3720 records. No padding or base-file workaround is needed.

## Runtime-selected inputs

The user completed `--capture-loading-inputs` through the Python launcher.
`analysis/work/startup-runtime/run/loading-inputs-90f74f210e9f4150a123e200e31f523a/`
contains the complete manifest and SHA256/size inventory; all files were checked
against those fingerprints. Capture uses the actual initialized rw_data resolver,
then shuts down before constructing the controller/game. This was user execution,
not an agent desktop test. Original assets were only read.

Important resolved overrides:

| Input | Bytes | SHA256 |
| --- | ---: | --- |
| Tables/anim.sav |111708|442504017fdaf3e28bf788a751fc8aef17c0215c3219f68f1425db3791bb99e8|
| GameData/Gamedata00.gdt |22777|bbc686808c98dd704899e1c37d3fbb4a800d99c07bc0e9e3504d536b97606f93|
| ALPS1/scene2.bin |6727085|ba91c84be678571d5567594a1373a6197731f5e866c6bc0a95b1d1830ecae274|

Both check.bin files are normally absent with check2.bin present; ALPS1 scene.I3D,
tracks.dat and mpscripts.dta are absent. Loader fallback/optional handling must
follow original behavior rather than treating every captured absence as failure.

## Native implementations in progress

`animation_tables.h` and host `game_animations.h` supply the actual animation
cache owner/latch and fixed-table fixups. The owner lives in StartupSession;
references clear while the original engine/cache is still alive. It is not yet
invoked by the complete game loader.

`mission_table_reader.h` implements00523C00 framing and0051DE80's version9 reader,
including0051C7B0/870/910 record/reference fixups. It uses the existing native
MissionTableSet/TableStorage owners and preserves query vectors across reload.
Captured navigation counts are863/56(Tutorial) and5113/100(ALPS1). Reference
callbacks remain mandatory actual scene/actor consumers at integration.

`actor_properties.h` decodes the12 nonempty property families required by both
captured missions. StaticWeapon/NullActor property methods are real original
no-ops. This does not implement actor constructors, geometry, portal updates,
switch animation or vehicle physics; required side effects use mandatory callbacks.

`car_parameter_block.h` implements004D0110 COMP marker-RLE decoding and004D55C0
version3 framing. `car_parameters.h` reconstructs004D7D80's runtime conversion
(third argument zero). All nine captured vehicle blocks decode to4687 bytes.
DWORD column transposes, byte flags and inline names retain original offsets and
copy order. Tutorial Jeep's source8F7 name spans27 bytes into the next16-byte
field: bounded native copies preserve the resulting overlapping writes. An
initialized-byte mask rejects reads of original unwritten padding. Encoder/editor
remapping,004E2E30 model-part initialization and004ED270 physics application
remain separate required work; a converted block is not a loaded Car actor.

`collision_tree.h` reconstructs006EE570's file structure, reference/mesh fixups,
spatial-cell relocation and dynamic-grid initialization plus006EE380's triangle
classification. Original serialized process addresses are discarded. Native frame/
vertex indices replace raw32-bit pointer relocation and preserve live mesh lookup;
they do not freeze mesh vertex snapshots. Unsupported cell tags, bad dimensions,
counts and indices fail explicitly. Missing actual mesh references follow the
original triangle flag20 rejection. An unavailable backend must throw, not return
invented null references. The new host `GameCollisionResources` supplies real
engine endpoints: driver+88 resolution, kind1 filtering, visual+7C(1) static-world
geometry baking, MIRR exclusion and actual LOD0 read-only vertex storage. Native
trees preserve borrowed frame/index references; the adapter caches borrowed mesh
views with32-byte vertex stride and checks indices against LOD+54's actual count.
The loaded world must retain these views and scene until the tree is released,
and discard them before scene reset. This bridge compiles, but is not yet called
through a complete mission loader or desktop-verified.

| Captured collision input | Triangles | Cells | Cell entries | Dynamic grid |
| --- | ---: | ---: | ---: | --- |
| Tutorial |30082|32768|89426|11×14|
| ALPS1 |28300|32768|95127|76×30|

Static assembly confirms006EE380 visits the first X×Z cell slots only, even though
006EE570 builds X×Y×Z cells. Dynamic dimensions truncate the X/Z extents divided
by header20's cell size and clamp to at least1; the decompiler loses those FPU
arguments. Known constants:0080F520=float1e-8,0080FAC8=double(float1e-8),
008162F8/00810148=double−0.0001/+0.0001. Ignored assembly/exports preserve these
claims under `loader/006ee570-prefix.asm`, `loader/006ee570-suffix.asm` and
`collision-finalize/006ee380.c`. `loader/006ee570-mesh.asm` preserves the actual
006EEBB9..006EEBC6 LOD/vertex ABI. Engine10036350 independently establishes
world-coordinate baking, vertex count at LOD+54 and32-byte vertex stride;
100570A0 is the mirror's actual no-op. Original DLL bodies are delegated, not
reimplemented or credited.

`loading_progress.h` and `mission_loading.h` preserve the progress thresholds,
stage ordering, delayed Scene2 status check and scratch/module cleanup. They
require actual services for every stage and do not provide a successful backend.

The existing host `GameScene` now supplies real Scene+F4 open(path,7,callback,0),
refresh/reset, scene name lookup, driver1001A0F0 path resolution and006CC580's
borrowed case-sensitive name registration (including duplicate replacement).
The three scene sector names override enumeration entries. Engine enum status
80000006 is ignored as in the original; callback exceptions are retained and
rethrown after returning from the DLL, never unwound through it.

Camera setup follows006CBF30's actual fallback branch, then perspective, near.1,
far500 and FOV bits3FB2B8C3. **Correction to earlier working inference:** static
stack stores at006CC2CC..006CC2E4 establish fallback direction(−1,−.1,−1), not
(0,−1,−.1). Position is(10,5,10). The engine scene retains the camera after local
release. None of these new methods publish world readiness or bypass remaining
actor/collision/activation consumers.

## Required actor families

Baseline Tutorial Scene2:25 Doors(type6), one Pike(28), one Switch(13).
Tutorial actors.bin:23 Humans(3),7 SupplyBoxes(9),5 Animations(25),3 Explosive
places(22),5 Ladders(8),1 StaticWeapon(21),4 Cars(4),1 FowlGenerator(29),4 SpawnPoints(10).
ALPS1 additionally needs NullActor(33) and Dog(30). Actual overrides can differ.

The registry0089B420 is zero-filled initially. Static registration helpers install
factory descriptors through00839460; see ignored actor-factories.json for the
full recovered table. Native ownership must pair class-specific initialization,
property loader and teardown. Do not call raw original client constructors.
Required factory/constructor/loader exports are under the work directory.

## Evidence and remaining work

Ignored `analysis/work/mission-world-20261103/` holds fingerprinted client/engine
exports, callback/animation assembly, registration references, original Tutorial/
ALPS1 Scene2/actors/sounds/check/tree/effects samples and hashes. Samples are
read-only research copies from existing ISD0 extraction; original assets preserved.
Original fingerprints and scope rules remain those in game-owner-start.json.

Implementation is ongoing. Strict application/startup builds pass with the new
scene and collision bridges (`windows-scene-loading.log`,
`windows-collision-bridge.log`). The native reader audit also compiles for i386
Windows (`windows-native-readers-build.log`); that fixture was not executed.
Existing native regressions and158 Python tests pass from the earlier capture/
animation batch; they were not needlessly rerun for unintegrated reader changes.
Temporary native ASan/UBSan audits cover captured animation/navigation/actor
properties, all nine vehicles, both collision inputs, prefix failures and loading
ordering. Frame/mesh/portal/switch callbacks in those audits are fixtures, so they
do not establish actual geometry, physics or desktop delivery. Logs are under the
work directory (`native-audit-resolved.log`, `native-readers-audit.log`).
No new P2.2 ledger credit yet. Current temporary audits were removed after passing;
`verification-wip.json` preserves source/evidence/audit fingerprints and limits.
Continue the full dependency boundary, then one consolidated affected build/native
sanitizer/regression sweep. No permanent tests, package, staging, push or commit of
new interactive behavior before user desktop confirmation.

## Scene2 application continuation and vehicle input gate

The portable `mission_scene_reader.h` now implements the captured dependency
set of006CC760/006CD940: persistent version/environment fields, object creation
and lookup, parents/transforms, model and sound resource requests, camera,
light/sector references, dummy bounds, area geometry, lightmaps, actor dispatch
and root service dispatch. Required actors/tables/modules are mandatory backend
calls; they have no successful defaults. Known unsupported occluder, projector,
sector and legacy sound branches reject when reached; unknown chunks retain the
original skip. This is partial original-body reconstruction, not whole-function
completion or successful world integration.

Object finalization at006D1873–006D1987 is checked against
`scene2-object-complete.asm`: position/update, automatic parent for a newly
created unparented object, explicit parent/update, rotation, scale, visibility,
update, then actual animation registration. The old decompilation aliases the
flags word with a string pointer; its apparent flags cannot be copied literally.
`game_scene_objects.h` supplies real LS3DF/cache endpoints, including ownership
through local factory references and retained animated models. Model requests use
the existing recovered extension-stripping helper before cache1 lookup.

Engine evidence: visual LMAP vtable1009C6D8 +BC is10028A60, which consumes a raw
rw_data file handle. `lightmap_payload.h` bounds the variable reads against the
chunk and actual live LOD/group/vertex/face metadata before delegation; the engine
must consume precisely that payload. Mesh ownership remains with LS3DF. All3467
captured lightmap payloads structurally decode (Tutorial766, ALPS12701); native
fixture geometry is derived metadata, not actual engine geometry validation.
Area geometry includes the original stride16 argument. Lens B114 uses the
float-to-integer conversion at006D1332/006D1339, without an invented scale.
`effects_environment.h` implements004860B0's defaults and00489BD0's B700
parameter copy; this value owner does not implement the GEffects render module.
See `scene2-engine-methods-v2/`, `scene2-client-methods/`, environment exports,
`runtime-chunk-census.json` and `lightmap-payload-census.json` in the work directory.

Temporary ASan/UBSan audit results:

| Mission | Model requests | Lightmaps | Actor dispatches | Property records |
| --- | ---: | ---: | ---: | ---: |
| Tutorial |1023|766|80|79|
| ALPS1 |1222|2701|134|76|

The fixture also checks explicit finalization order, every short document prefix
and all mutation failure positions. Frames, actor construction, geometry and
resource calls are fixture endpoints. It establishes native parsing/dispatch,
not loaded actors, physics or desktop delivery. Strict application/startup builds,
existing native regressions and158 Python tests pass. Engine-window is current.
Logs: `scene-reader-native-audit.log`, `scene-bridge-windows-build.log`,
`scene-model-key-windows-build.log`, `scene-bridge-native-regressions.log`,
`scene-bridge-python-tests.log`. Temporary audit fingerprints are retained in
`verification-wip.json`; temporary audit programs are removed.

The expanded user-run capture resolved the vehicle input gate. All35 sizes and
SHA256 fingerprints in
`analysis/work/startup-runtime/run/loading-inputs-22dc45fef6dd48e8b628aa1412d03360`
verify, including the four required model candidates:

| Mission | Car frames | Model key |
| --- | --- | --- |
| Tutorial |Jeep, Jeep2|la_jeepuvj|
| Tutorial |la_Bedford_1, la_Bedford_2|la_Bedford|
| ALPS1 |hakl, hakl_na|la_hakl_Ev|
| ALPS1 |La_OpelE_2, La_OpelE_, cutopel|La_OpelE|

The capture derives dependencies from the actual resolved Scene2/actors/sounds
and reads registered cache1 directory candidates through rw_data. All candidates
are retained; presence does not imply engine acceptance or selection. The native
capture audit and35 output-failure/three read-failure checks remain valid.
`vehicle-model-inputs/verified-capture.json` and first-table/string censuses
preserve this evidence. Raw strings establish name/attribute fixtures, not actual
engine geometry. No further user capture is currently required.

Read-only math-import and callsite repairs now recover the required geometry
branches omitted or aliased by the old decompilation. Source-only runtime Car
parameter application004ED270, tyre registry004D6900/004D6BF0, geometry helpers
and ordered004E2E30 composition are implemented. See the
[vehicle geometry contract](car-geometry-loading.md) for exact boundaries,
addresses, audits and remaining actor/activation work. A composed geometry value
is not an initialized Car actor or a successfully loaded mission. Both P2.2
checkboxes remain open; continue their full dependency boundary.


Actor continuation2026-11-04: [common/Human ownership, reference cleanup and
Ladder/SupplyBox/Switch contracts](actor-loading-ownership.md) are source-only
and fixture-audited. Concrete shared frame/lifetime endpoints compile; full
Scene2 actor factory/family/module/loading activation remains open.

Loading presentation continuation:006C9640/006CA470/006CA2D0 now have source-only
single-player owners and real scene/graphics/font bridges. Runtime GEffects and
external-progress consumers remain mandatory/unintegrated. Native lifecycle and
actual-caption glyph audits pass; no world/desktop delivery follows.
[Presentation contract and verification](loading-presentation.md).

## Current module and activation continuation

[Module/activation contracts](loading-modules.md) preserve the newest original
receiver map, shared activation/collision ownership, effect/track/script-binding
readers, item instance owners, actual volumetric texture endpoints and mesh-movie
material/video loading lifetimes. The verification manifest
`work/mission-world-20261103/loading-module-verification-wip.json` fingerprints
357 source/log/evidence files and23 removed temporary audit files. No live
containing world, successful mission load or desktop claim follows from these
source owners; both P2.2 boxes remain open.

2026-11-05 targeted correction during Human ground-query recovery: serialized
collision cell+08/+0C are query limits, not disposable scratch.006EE570 retains
them and00715510 compares both. The reader now preserves their original bits in
native Y,Z,X order. Focused sanitizer/independent-byte comparison passes32768
pairs each for captured Tutorial/ALPS1. Mesh references in this metadata fixture
are deliberately unavailable; it is not engine/desktop collision verification.
[Current Human/collision consumer boundary](human-model-geometry.md).

2026-11-09: [offline containing activation and prepared spawn](session-activation-loading.md)
now have source ordering, world quantizer/light policy, cold model/equipment
population and local packet producer. The actual local decoder/camera/world/VM
composition remains required. Both P2.2 checkboxes remain open; no new world
rendering or desktop acceptance follows from these focused native/host checks.

2026-11-10: [Concrete activation and live-squad finish bindings](session-activation-integration.md) compile across the actual graph; same mutable records preserved, native marker read-only and navigation/dubbing finish checks pass. Shared transient receiver, meaningful guard/control/first frame and desktop acceptance remain.
