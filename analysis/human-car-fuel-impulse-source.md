# Car fuel and collision impulse source —2026-11-12

P22-HUM reactions/damage/ragdoll remains OPEN. This is a source checkpoint,
with no application mission loading or desktop acceptance.

## Original evidence and owners

Original client SHA256 remains
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
Evidence is in ignored WORK `analysis/work/mission-world-20261103`:
`human-car-authority-extra-branches-index.txt`,
`human-car-impulse-owner-fields-index.txt`,
`human-car-fuel-impulse-export-index.txt` and
`human-car-restore-fuel-checkpoint-index.txt` record indexed lookup before further
exports. Candidates are not equivalence proof. Rollback-only
`ExportHumanCarFuelImpulseABI.java` produced `human-car-fuel-impulse-abi`;
`human-car-fuel-impulse-originals` retains default exports. The writer's decompiler
still loses the FTOL ST0 input: use raw `human-car-human-damage-packets.asm`,
`human-car-impulse-packet.asm`, `human-car-fuel-impulse-contact.asm` and bounded
`human-car-fuel-packet-constants.json`. No original code was executed.

Actual004DB360 presentation and004DB760 authority requests now consume kinds10
fuel and11 impulse through the SAME GameBlastWaves, CarLoadingActivation,
Car physical state and shared module13 streams. The callback damage borrower
bundle prepares/publishes its collision endpoint together and unbinds together.
No second actor graph, packet stream, physical body or collision clock is added.

## Fuel10 and impulse11 grammar

004E0040 reads16 bits: the native DOUBLE49999.99925494195 yields that width.
Decode unsigned16/65535 times5000, with native zero/5000 clamp and FLOAT store.
004E00F0 writes the low16 of FTOL(value * FLOAT.00019999999494757503 *65535),
without an intervening FLOAT spill. Therefore5000 encodes65534. Do not change
this to an idealized normalized quantizer. The source checks an unrepresentable
FTOL input instead of making an undefined C++ conversion.

Presentation10 invokes actual004F2670 through the existing cold initializer:
upper-only capacity2220 clamp and fuelF5C store. Authority10 stores through the
same setter BEFORE rebroadcasting the ORIGINAL decoded event, including when
capacity clamps its local value. Setter failure prevents publication; publication
failure preserves the fuel store. No unrelated geometry/DM preflight precedes
the payload read or a genuine native default.

004E0800/004E0830 read/write the collision point using SAME primary(-1)/mask7,
then source identity10. Both actual receiver paths call004D3D10. Event headers,
request/broadcast priority3, identity10, partial writer state and finish remain
shared with earlier Car packets. Writers004E00F0/004E0830 are absent from the
frozen inventory and receive ZERO credit.

Expanded regression exposed an actual defect in the preceding source checkpoint:
004E0180's local `CarHumanDamageEvent event;` left its two unproduced fourth
vector lanes indeterminate. It now uses value initialization. Six serialized raw
FLOAT components and signed16 fields are unchanged; no original padding meaning
is asserted. The earlier contract's initialized-lane claim is now implemented.

## Collision ownership and lifecycle

004D3D10 reads current driver0; a nonnull first read is followed by a fresh read
before identity and global local-ID comparison. A vanished second driver remains
a required failure. A local driver applies directly even in client mode.
Otherwise mode&1 false returns. In server mode, a fresh nonnull driver follows
another mode read and driver read; a missing fourth driver returns. Signed
wrapping(now-2584)>1000 triggers a SECOND clock read/store BEFORE event11
broadcast. No physical lookup or force runs on that relay branch. An empty third
driver falls through to direct application.

Direct application uses00475DD0 physical Scene2 identity, distinct from request9's
00475D50 command lookup. Missing source is a genuine noop. Actual Car9A8 mass is
physical scalar918; source4A0 velocity is SAME existing body410 motion. Compare
fresh masses, use1 if other>own, otherwise reread other/own and FLOAT-spill ratio.
Reload own mass after genuine empty00611080/0069BFE0. Raw x87 has distinct spills:
X ratio*velocity*mass FLOAT before*.3; Y remains extended until final store;
Z ratio*velocity FLOAT, then timesmass FLOAT, then*.3. Forward actual004F2790
mask0/scale1 through the SAME integration state, without replacing velocity.

004D3EF0 hit58 calls actual004DE800 through existing presentation authority.
Owned contacts continue previously verified Human/Car force bodies. Unowned
client contacts read SAME native2584 and request11 after1000ms, storing the
SECOND clock BEFORE reading source identity or serializing. Earlier contacts
and server unowned contacts call004D3D10. Offline mode7 remains unconditionally
owned and keeps its cold-loading callback path.

2584 already belongs to CarLoadingActivation:004D16A0 produces it each activation
BEFORE camera264/reset. Its source representation is now optional, so an absent
producer fails while a genuinely produced zero is valid. No duplicate clock,
new readiness gate or delayed store is introduced. Non-Car actors do not expose
verified9A8/4A0 mappings here; reached such reads fail honestly rather than using
fabricated Human mass/velocity.

## Verification and limits

`human-car-fuel-impulse-native.cpp` passes ASan/UBSan/LSan:100 retained request/
timer regression families and100 fuel/impulse families. Actual source packet
buffers, quantizers, streams, Scene2 routing and consumers are exercised with
explicit actor/clock/physics/damage/detachment/destruction endpoint doubles.
Cases include fuel golden bits, both receiver paths, upper capacity clamp with
original rebroadcast, absent unrelated owners, profiles0/1, mode/driver fresh
reads,1000/1001ms, signed wrap, missing source, and retained failure prefixes.
Independent Decimal100 oracle `human-car-impulse-spill-oracle.json` supplies exact
FLOAT inputs yielding XYZ bits457E4C41/42/43; all three source outputs match.

Strict actual nested World/Car/Human/module13 i386 compilation passes with the
explicit fuel/impulse/hit58 fixture. Final launcher/probe compile/link passes.
Heavy compilation and Ghidra were sequential. Windows outputs were NOT run.
[Receipt](checkpoints/p2.2-human-car-fuel-impulse-20261112.json) adds3 client
partials and extends6, with no complete promotions:34 complete/2757 partial
(2752 client+5 engine), denominator18783.

Typed/template/STL/RAII adaptation does not reconstruct native allocator/object/
SEH/virtual ABI or establish full network/trajectory equivalence. Non7 activation,
Car events6/7, deformation/global backup/occupied restore and full Human entry/
frame/failure/re-entry/ordered retirement remain required. Confirmed unproduced
part5F9/wheel695/EXPL8/smoke14 remain separate honest failures. The selected full
Human checkbox and P2.2 application/desktop work remain open.
