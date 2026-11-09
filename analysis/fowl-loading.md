# Generated Fowl loading and first update — source, 2026-11-09

P2.2 remains open. GameApplicationState still declines loading; no original code,
Windows fixture or live mission was run. This supplies the Tutorial's actual
mode2 generated-bird loading/update/hearing/retirement dependency using the SAME
scene, Scene2 registry, collision tree/grid and SOUND registrations. Borrowed
mode0 birds still require their separate global pool and explicitly reject.

## Original boundary and captured input

Fowl vtable00811D80:3C→005BA670,40→005BB290,14→005BB3C0,
18→005BB4D0,1C→005BB280. Tutorial actors.bin has one `fowls` record:
modeA0=2, radiusA8=7.f, proximityAC=15.f, countB0=30, `ptacekletacek`.
Unlike ordinary actor activation,005BA670/005BB290 do NOT call common
00610240/00610510; source preserves that distinction.

005BA670 runs authority OR !mode1, clearsC0, calls005BA8B0 for mode2,
registers actual005BA860 with SOUND only on authority, setsB8=rand()%1000 and
clearsBC. Mode1 generates on take-off instead. Mode0 transfers borrowed birds
from global0089BA84 and needs that actual pool; it cannot pass with an empty list.
Failed activation requires retirement before retry. Listener/actor owners retire
before SOUND/frame producers. Frames retain generated-model references for
native source safety; the actual engine scene independently owns membership.

005BA8B0 uses FLOAT sqrt(signed B0), then floor(sqrt(B0)) descending rows and
columns: count30 makes **25 birds**, not30. Step is2*radius/sqrt(count), FLOAT
jitter=step*.4, FLOAT spread=2*extended(step*.4). Each cell consumes two actual
00475190 random draws for X/Z offsets, creates/clones a real model, adds to the
actual scene and links root1F8. Mode2 casts(0,-100,0) through actual007183A0→
006F5890 (existing CollisionRay vertical receiver), replacing position on hit.
It shows/positions the model, sets actual frame+1C direction from two further
random draws, names/updates and constructs an actual004797C0 bird. No missing
model, geometry/query, listener or consumer becomes a successful placeholder.

Important original name bug:005BB023 stores a DOUBLE row*sqrt(count), but actual
format0083E030 is `_bird_%d_%d`. The second `%d` reads the DOUBLE's low DWORD.
Source explicitly reproduces those bits using memcpy and a typed integer,
avoiding undefined varargs while preserving repeated/misleading native names.
This name is not a made-up unique bird/grid identifier.

## Bird lifetime, motion and actual callbacks

004797C0 creates actual timers40=rand()%5000+5000 and48=rand()%20000+3000,
state64=3, phase/velocity/flags0 and speed5. Link root with preserve-world1,
update, save local pose, publish this record at modelEC, find/hide `Object03`
and `Object01`. Generated models must not have an existing birdEC borrower.
Borrowed existing birds/return pool are explicitly outside this source branch.
Constructor publication rollback, scene add failures and allocation prefixes are
owned; no frameEC can retain a pointer to a destroyed source bird.

004799E0 clearsEC only if it still points to this bird; flag60=true removes the
model from the actual scene.005BB290 retires birds backward and on authority
removes the real listener. Source RAII retains failure evidence and retracts
published membership before frame release, including incomplete construction.
The original borrowed-bird restore/global-free-list branch remains excluded.

0047A2F0 preserves same CRT draws: speed=RandFloat(2)+5, direction random X/Z,
actual world forward normalization/addition/normalization and speed*.001,
phase=pi/(rand()%100+1), state0, flapping4D; both real wing borrowers are required
to set launched4C/show wings. No new RNG or replay-only seed is installed.

0047A2E0 converts signed elapsed DWORD to FLOAT before00479C70. Wing absence
returns false. State3 retains idle countdown, jump/landing toggle, native double
.0001 offset, gravity and model pose restore; flight states0/1/2 preserve climb,
recursive transition updates, descent normalization and actual camera/sessionC4
culling. Native FLOAT stores differ by branch: state1 stores(X²+Z²)+Y² before
both comparisons; state2 stores(Y²+Z²)+X²; flight forward X stays extended.
Wing phase retains extended arithmetic through sin, then angle=sin*.75-double
.4000000059604645 stores FLOAT. Actual engine S_quat::Make/Normalize/flags write
first wing and mirrored q3 on second. No engine delegate receives new credit.
005BB3C0 visits birds backward, retires false results with swap/pop, and updates
launchedC0 (plus10000 when the formerly nonempty list becomes empty).

005BB140 optionally creates mode1 birds, launches backward, emits actual group11
entry(count>3) with flags1 when mode2/event<1000, then setsBC/clearsC0. Notify
writes actual pending module13/Scene2 identity10bits with NO actor payload;
005BB280 invokes launch(false,0), consuming no imaginary command field.
005BA860 resolves actual listener/frameEC, accepts group0/2 and volume>.8,
then launches with notification. GameFowlSoundLoading owns real shared SOUND
registrations; callback failures are retained/rethrown outside the engine.

005BB4D0 uses wrappingB8 subtraction and equivalent +1000 normalization,
then actual Scene2 identities: first32 until the first null; backward beyond32
until null/boundary, restricting type3 to property19C==1; finally actual active
camera. Actual proximity radius/distance rules trigger005BB140, never a default
player/camera. GameWorldActors binds these services and same-tree downward query;
GameSceneActors now dispatches29. The application still does not construct this
loaded-world composition.

## Verification and continuation

[Receipt](checkpoints/p2.2-fowl-loading-20261109.json): focused native
ASan/UBSan/LSan with captured properties,25 real source bird owners, shared
bitstream, grid order, ground-call arguments, EC/refcount/scene cleanup, idle/
jump/take-off/climb/descent/cull, sound trigger, camera/reserved-player proximity,
repeat and554 frame failure prefixes plus ground/listener failure. Engine frame,
model/geometry/math/localization/audio/actor lookups are boundary doubles.
Strict actual i386 host instantiates source Fowl, real Scene2/WorldActors/tree/
CollisionRay/frames and StartupSession SOUND bridge; app compiles/links only.
27 tracker tests/check pass; temporary source/binary/object fixtures are removed.
No first world rendering or desktop acceptance is claimed.

Client SHA256:
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
Ignored WORK=analysis/work/mission-world-20261103: fowl-loading-required/,
fowl-first-update-required/, actor-activation/005ba670, fowl-bird-update.asm,
fowl-bird-flight.asm, fowl-creation-and-packet.asm, fowl-original-constants.json
and next-derived-actors-properties.json. Nine implemented frozen bodies get
partial only.005BA860/005BB290/005BB3C0 are absent inventory;007183A0 delegates;
no new credit for those or generic helpers, engine delegates and researched-only
bodies. Continue Animation/SupplyBox/StaticWeapon/Car and actual containing
session/Human/VM/FPV/HUD/SOUND owners, loader/first-frame/failure/repeat/user
acceptance. Full P2.2 checklist scope remains unchanged.
