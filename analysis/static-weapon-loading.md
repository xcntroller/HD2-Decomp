# StaticWeapon loading and cold update — source, 2026-11-09

P2.2 remains open. This supplies Tutorial's required mode7 StaticWeapon loading
and initial unoccupied update through the SAME Scene2, Items/entry registry,
frame-data, navigation table and collision buckets. GameApplicationState still
declines loading. No original executable/DLL or Windows fixture was run, and
there is no rendered-world or desktop acceptance claim.

## Original boundary and input

Vtable00813130:3C→006036D0,40→00604120,14→006034D0,18→00603070,
B0→00604D90→004C57D0. Actor+160 owns004C1AD0 control; +150/+158/+159
are separate flags. Captured Tutorial actors.bin has one type21 `Browning`, no
property bytes (00604CE0 is a genuine no-op). Its object record creates kind9,
loads `w_1919Crouch`, position(33.0264816,0,45.012085), scale1.

Read-only base Models.dta extraction confirms `SEAT00`, `HORT00`, `VERT00`,
`BARREL00_00` and `SHDUM00`; barrel user text is
`matid12,itemid34,up30,down35,left45,right45,hs180,vs180`.
Base model and firing model/animation are preserved with hashes in ignored
WORK/static-weapon-base-model-inputs.json. This is base archive metadata, not
an engine/runtime override-resolution test. Required live frame/resource calls
remain mandatory; different reached metadata cannot use successful substitutes.

006036D0 calls common00610240 before its +159 once guard. Offline mode7 skips
the separate non7 input/network producers. Seats `SEAT%02d` are found ascending
mask20, each initially unoccupied. Partial source activation must retire before
retry; repeat completed derived activation still executes the common pass.

## Actual control, item and resources

004C1AD0 binds actual actor/model/index0, selects HORT00 parent prefix if present,
finds/hides `VIEW00`, and parses `BARREL00_00` itemid.004C6570 lowercases BOTH
text/key, finds the first substring and runs atoi immediately after the key;
there is no separator skip or token-boundary rule. Missing frame/key returns−1,
nonnumeric suffix0. Bounds reject native buffer/atoi undefined inputs.

GameMissionItems now supplies actual007D7AD0: validate0..499/catalogue, use the
existing real family factory/entry registry, set identity−1 and append to SAME
Items loaded-instance registration. There is no new identity counter or fake
weapon item. Control retirement uses existing007DC8A0 matching remove/destruct.
Captured Tables/items.sav has item34 with actual payload kind3, so control168 is
false (native comparison is kind==2). Payloads deserialize at native+04;
control's native payload+1C ammunition word therefore comes from serialized
byte24. No native padding is read as an item field.

Barrels iterate `BARREL00_%02d`, mask10001; first reads hs/vs/up/down/left/right.
Actual float radians0080F6E8=3C8EFA35 and negative008106A8 preserve signs/defaults.
`SHDUM*` and `CARDUM*` use10020. Animation key takes original model basename,
removes extension and appends `_f00_%02d`, through existing cache3. Native append/
hook guard is SHDUM, not animation availability. Matching source owners retain
clips and retract callbacks/backlinks before releasing them, including failures.

GameStaticWeaponEvents installs actual frame+28 callback004C1990 semantics,
with the SAME frameEC word7 borrowing the typed control. S markers now pass the actual barrel into
[H3 casing receiver004C5080](human-seat-interaction-source.md); callback exceptions
latch for the containing world to rethrow. Source retirement clears every matching hook/backlink before
control destruction. No independent opaque-data block or guessed frame pointer
is installed. The cold checkpoint is extended by H3 occupied seat/control/aim/casing source.
Shooting-input/ammunition producers remain H4/H6.

Actual HORT00*/VERT00*/AIM00* and `projectile` borrowers use native masks.
`GunShells` is category1 MODEL004C76C0, not category4 shadow cache; a returned
actual model is hidden. Real S_quat::Inverse(axis,angle) preserves original
horizontal/vertical poses; native Make(axis,angle)/normalize/update restores them
on retirement. These engine math calls are delegated, without new credit.
Positive soundm/sounds metadata now uses actual004C6E50 group3/first-variant SOUND
owners and the same existing cache. Reaction release004C6DD0 and ordered004C7630
retirement are bound. [Mounted sound checkpoint](checkpoints/p2.2-human-mounted-sounds-20261111.json).
The base Tutorial HORT metadata does not select these positive paths; native
endpoint doubles and compilation do not establish mission/desktop behavior.

## Collision, navigation, initial update and cleanup

00602B20's existing StaticWeapon bounds aggregation excludes descendant visual
flag1000, supports actual OBJ_ geometry and retains native local transforms.
Descriptor F0 has flags40/mask800000, the actual frame/world-matrix borrower,
actor pointer, unscaled local bounds and stable owned inverse. Radius takes
FLOAT stored extents, halves, extended Z²+Y²+X²/sqrt then maximum actual world
scale. Sphere centre is bounds midpoint transformed through the actual world
matrix, rather than frame origin. Undefined invalid bounds reject. Assembly
006038DB..00603B60 preserves this contract.

006055C0 uses the first backward matching actor reference from the SAME session
navigation dataset, then real point position. No match uses original world
position minus half forward; each product stores FLOAT before addition
(00605644..006056B0). Source fourth lanes are defined without claiming native
unwritten padding equivalence. Optional camera model is real driver9 with
`C<full original resource>`, flags10; original load failure leaves it absent.
00605AE0(1) shows and inserts the actual descriptor once.

00603070/006034D0 call the true unoccupied004C55E0/004C56A0 early-return paths.
Initial controlFC=null and first seat occupant=null mean no manufactured tick,
target, firing, packet or occupant receiver. Non7 input/network, occupied seats,
firing, positive movement sounds and destroyed pose restoration explicitly need
actual later receivers. Unreached timestamp/targeting/simulation control fields
are not represented as a full control ABI or whole-body implementation.

00604120 common-first cleanup clears seats, retires004C2380 control and selects
state3. Restore actual poses/update model, remove matching real control item,
retire frame callbacks and clip/shell owners. State3 removes descriptor; native
00605C80 restore is gated by actual158 (false cold). Source failure cleanup also
removes any partly published descriptor and helper resources before frame/item
producers disappear. Actor loading owner retires before its control/base storage.

## Verification and continuation

[Receipt](checkpoints/p2.2-static-weapon-20261109.json): temporary native
ASan/UBSan/LSan with captured runtime catalogue/base authored metadata, source
control/buckets/actor lifetimes, descriptor centre/radius, both activation passes,
cold update, repeat and84 frame failure prefixes plus item/event/navigation/
factory/mode failure. Engine geometry/model/math/localization and item construction
are boundary doubles; item34 definition/payload is the actual decoded capture.
Strict actual-host i386 compiles actual Items factory/WorldActors/Scene2/frames/
frame-data/events/buckets integration. App compile/link and27 tracker checks
pass. Windows fixtures are compile-only. Temporary sources/binary/object are
hashed/removed; no permanent tests added.

Client SHA2568c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.
WORK=analysis/work/mission-world-20261103: static-weapon-loading-required/,
static-weapon-control-required/,actor-activation/006036d0,
static-weapon-collider-setup.asm,static-weapon-tutorial-input.json,
static-weapon-base-model-inputs.json. Base w_1919Crouch.4ds129689bytes SHA256
bdb262f97676437dbc597f2409529b5564b2dabed8f309c8e914b00d33b00a46.
Captured Tables/items.sav SHA2569dbad22a089489eb4bf2e7600f7b88d6be5ffc6104146b1872080c41c60a92f0.
Nine implemented frozen bodies receive partial credit, zero complete promotions.
00604120/006034D0/004C1990 are absent frozen inventory and earn no credit.
No new credit for delegated engine/cache/navigation/previous item bodies or the
control update callees' null gates. Continue Car and actual containing Human/
session/VM/FPV/HUD/SOUND owners, ordered loader, first update/render/failure/
repeat and user acceptance. The full P2.2 checkbox remains unchanged.
