# H8.4 active Car lights — source checkpoint, 2026-11-16

Branch `feature/p22-hum-h8-4`, baseeb8bce9. This completes the reached
00501CB0 light-policy boundary left guarded by the
[engine/skid/particle checkpoint](human-h8-car-finalizer-source.md). All nine
light types now execute on existing geometry/frame/projector/paired-state owners.
H8.4/D4/D5/H8/P2.2 remain OPEN: occupied containing-frame verification, selected
non7/entry dependencies and confirmed D11 missing producers remain required.
No original EXE/DLL/Proton, Windows artifact, application or desktop execution.
Agent1's worktree/branch is untouched; no post-fork source imported or push.

## Original evidence

Research-index lookup preceded exports of00501CB0 and00509A40/00509A50,
0041D4B0/0041D160/0041D200. The existing raw full00501CB0 export is18980 bytes;
its unassigned registers and stack inference are not reliable ABI evidence.
WORK=`analysis/work/mission-world-20261103`. The original image remains SHA256
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.

`ExportCarActiveLightsABI.java` applies bounded signatures and indirect-call
argument counts, exports `h8-4-active-lights-typed/*.c/json`, then rolls back its
transaction. Ghidra ran read-only/noanalysis and the research project was not
saved. Typed C and `h8-4-active-lights-original.asm` together establish all nine
switch paths, including cases where the decompiler still aliases scratch values.
`h8-4-light-direction.asm` establishes0041D200's arithmetic. Initialization
uses the indexed004E2900 constructor and existing typed004EA780 evidence.

Consequential original contracts:

-00501CB0 returns before any timer/state/frame work unless current OR previous
 F6C/F70 has80. Current80 also gates each active light.
-5B8=FLOAT0.4,5BC=FLOAT0.3,5C8=FLOAT0.3 in004E2900. Hazard alternation uses
 strict `interval < accumulated`, flips20000000, and selects10/20. Leaving80
 clears30 only when previous commands had80. Ordinary blink uses >=, flips
 10000000 and discards overshoot.
-0041D200 sums Y/Z/X in extended precision, compares the unrounded sum against
 the near-unit epsilon, uses the stored FLOAT square for sqrt and keeps its
 reciprocal extended through XYZ stores. Inlined early light paths sum X/Z/Y.
 W survives normalization; tiny-vector fallback replaces one axis without
 clearing other components.
-0050403E/005048C3 hide the **projector visual**, not its companion. These are
 reached in several active branches when companion feature1000 is absent.
 Type7 instead hides the companion in that case, under its native seen gates.
-00505CE2..00505CEE: type7's shared companion receives actual core240 forward;
 other directions come from each light/shared marker's normalized world axis.
-00506647..00506691: an inactive shared pair already marked80 skips both hides;
 otherwise visual hide precedes mandatory companion hide. Billboard frame24
 remains the final per-record operation, including unknown types.
-004EA780 publishes544 from the actual model parent AFTER scene placement.
 004D38F0 copies core240 and live160 to actor direction/position. It does not
 copy cached world translation in place of160.

## Source composition

`CarLoadingLightState` now retains real on/off/hazard intervals. Automatic
reverse/brake/steering commands continue to read SAME drivetrain/steering values.
Types1/2 indicate;3/4 give their side's indicator priority over brake;5/6 select
alternating light bits;7 selects10000;8 selects brake100;9 selects reverse1000.
Their projector-family masks are respectively100000,400000,200000,800000,
1000000 and2000000. Damaged records suppress activation. Family masks can leave
previous projector state alone even while billboards change.

The full reverse pass clears transient paired bit80 first, preserving health
bits0/1 and the high byte. Positive rendering links to SAME cached sector544
only when parent differs, marks shared usage before position work, places from
actual markers, publishes direction with FLOAT roll0, enables and updates each
selected projector/companion. Existing `paired_flags` belongs to health/damage/
reset; it is passed by reference rather than copied into new light state.
Unified nonpaired slots in that vector are unused. Missing actual companions
or source markers fail explicitly instead of reporting successful rendering.

`CarLighting::discover` now retains the actual shared PROJECTOR marker on both
paired records; it borrows that frame from the existing model lifetime. Single
lights still use their own marker. No factory, texture, light or model owner is
replaced. Native inline XYZ position writes and0041D110's four-word writes use
an explicit checked Car frame bridge; locked80 target frames skip position.
The old general three-component mission setter is not used as a four-word copy.

Actual initialization now publishes sector544 at the recovered point; finalizer
and retirement pass SAME paired flags/sector/physical forward into00501CB0.
Actor position publication now reads `physical_position()` as004D38F0 does.
The full nine-type guard is removed; the explicit occupied containing-frame
guard remains until actual H3 entry/frame verification. Application readiness
and desktop behavior are not enabled by this source checkpoint.

## Verification and remaining boundary

Temporary native C++17 warnings-as-errors ASan/UBSan/leak verification passes
7170816 checks over16000 comparisons with a separate bounded transcription of
the original switch, plus139 injected endpoint-failure prefixes. Exact event
order, paired flags, logical state/timers, parents/visibility and positions are
compared; directions use1e-6 relative tolerance. Real CarLighting discovery
produces nine standalone records and a shared two-record PROJECTOR pair. The
audit covers family masks, damaged records, feature withdrawal, hazard/blink
thresholds, target-position locks, shared preservation, type7's direction and
W-preserving normalization. Geometry/frame/math/resource endpoints are explicit
doubles; this is not a native whole Car/World execution or binary comparison.

Strict actual nested World/Car activation, both frame passes and retirement
fully instantiate/link under O0 i386 warnings-as-errors. Launcher/probe are
compile/link only. Tracker validation and27 existing tracker tests pass.
Temporary audits are fingerprinted and removed; no permanent tests are added.
The [receipt](checkpoints/p2.2-human-h8-car-lights-20261116.json) records evidence,
verification and cleanup. Existing partial entries extend; no new frozen entry,
complete promotion, native ABI or delegated engine-body credit is claimed.

Follow-up research indexed004D3110/004D38F0: offline presentation has no separate
occupant loop; its controller→mounted→finalizer→features→camera/navigation/model
order is already composed. This does not itself verify actual occupied entry.
A bounded2297/2327 displacement scan found initialization004E2C2A,
writer004F8D3F in004F8510 and existing readers; alias-based writers are not ruled out. Do
not invent an interior-audio flag from Human attachment. Retain the occupied
boundary until SAME H3 input/attachment/control/camera source is verified through
that frame. D11's confirmed fresh part5F9/wheel695/EXPL8/smoke14 omissions are
unchanged; full selected H8.4 delivery remains blocked on them under current
policy. No renewed D11 search, coefficient default or successful placeholder.

The later [occupied offline source checkpoint](human-h8-car-occupied-source.md)
verifies the shared Car admission/input/frame/camera/removal path and removes
that empty-seat guard. Its endpoint limits and remaining D5/D11 obligations are
explicit; it does not infer an interior-audio selector from attachment.
