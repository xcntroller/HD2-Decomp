# Human H3 interaction source

Worktree `../HD2-Decomp-p22-hum-h3`, branch `feature/p22-hum-h3`, based on
`feature/p22-hum` b99f088. Source work only; no original or Windows execution.
**H3 source checkpoint complete.** Wider occupied Car simulation remains D5;
application/World delivery and the full Human frame remain OWN/FRAME/H8.

## Original boundary

Entry00751480 cases2/4/6/B/D and exit3/5/7/C are selected by actual E0→0077DE30,
with A3C typed target/A40 seat/A34 flags/A38 timer. Existing Door and model8–10
are reused. Car virtual74/78/80/84/8C/98/B4/C4 use SAME loading lifetime,
occupants, mass, physical state, authored markers, mounted controls and cameras.
StaticWeapon006041A0/00604590/00604630/006046D0 use its SAME seats/embedded160
control/stable collision descriptor. DD0 geometry, B0 setters, equipment and
shadow are the existing Human owners. No DWORD represents a host pointer.

- Entry initializes its previous seat to-1 at751495: the store is at ESP+30
  while the OutputDebugStringA argument is pushed. After stdcall pop it is
  ESP+2C, read at751A02. C-only v3 incorrectly presents unrelated float/pointer
  locals. Never infer a missing producer from that decompilation.
- Car controlled entry calls actual004F2530(true,!mode2), which can start
  00518710(true), or0051A620 on fuel/body rejection. The body-health feature
  gate reads F6C from actual solver.substeps.flags; light requests are5B0. Stop's gate is22A2,
  independently of22BF.004D16A0 produces24A8=0 at004D1773.004D1200 writes2321,
  not232C.004D9430 clears2329..232C AFTER assigning the occupant.
- StaticWeapon links the requested seat but samples the FIRST marker for base
  pose in006041A0. Occupant assignment precedes control/collision/script tail;
  existing occupant, even the same Human, rejects.006046D0 uses actual006055C0
  navigation position or backward model offset, then clears the seat only after
  pose publication. It does not use Car's swept exit search.
- Car004DA2F0 searches the current seat first, then remaining seats ascending.
  004DA420 excludes the SAME main/special descriptors, tests lateral .25 sphere,
  then floor(0,-3.5,0), and writes neither output on failure. Restores exclusions
  BEFORE output publication. Successful placement is floorY-.2 and normalized
  planar direction back toward the candidate marker. Blocked exits retain the
  actual attachment and publish A34=402/A38=-10000 in00755500.
-004C6DD0 publishes FC before player/view queries and SOUND release.004C6620
  saves occupant, tears down actual resources, rebuilds model/pivot/index, then
  restores the saved borrower. Car event6 invokes it after each ascending seat
  removal. No new seat/control/simulation owner exists.

## Mounted receivers and scope

004C55E0 authority and004C56A0 presentation retain their mode-bit delegation,
failed138/null-FC gates, health release, reloadE0 and154 expiry ordering. Native
004C1AD0 produces154=0; constructor/initializer do not produce118/11C,
so recovered angle slots remain disengaged until their actual aim writes.
004C3120 uses signed elapsed/ratesCC/D8, the lazy local quaternion/matrix branch,
metadata limits, pitch safety clamp, original damping and target-only B0+54.
004C2580 reads actual B0 target direction and VERT world direction, wraps yaw,
steps local pivots using160/164 speeds and publishes native14C direction bits.
Both retain the actual120/124 motion SOUND order and idle301ms release.
StaticWeapon presentation ticks model+20 only when the FIRST seat is occupied,
then refreshes its SAME stable collision descriptor. Car's full occupied frame
is still the separate D5 boundary; its mounted owner exposes these receivers.

S callback004C1990 now passes its actual barrel frame into004C5080. The receiver
samples barrel world position/forward/up, consumes three RandFloat calls for
normalized up velocity*3 and a fourth for the velocity-scaled position offset.
It copies GunShells/SHELL into an OBJ_ visual, links root and places it in Scene
BEFORE direct local position publication, then sets direction/visibility/update.
The SAME World Physics owner creates type0/material12/list1 with no callback,
then receives impulse(velocity,zero,.05); factory visual ownership is released.
This is casing ejection. Player/AI shooting-input and ammunition producers are
H6/H4, not replaced by a fake shot or an independent actor/Physics module.

H3 closes the selected offline interaction and mounted receiver **source**
boundary. H2/D5, H4–H6, H8 and actual application/render/desktop acceptance remain
open. Non7 packet/input owners remain explicit required guards. No complete
Human/Car simulation or function-equivalence promotion is claimed.

## Verification

[Checkpoint receipt](checkpoints/p2.2-human-seat-interaction-20261112.json) records
source/evidence/temporary-audit fingerprints and the function inventory delta.
Four focused native ASan/UBSan/LSan audit families pass100 repetitions each:

- Human action request/timer/entry/exit, all attachment modes, overlay rows,
  live/dead displacement, race rejection, failed-exit402, signed wrapping
  and interrupted assignment. Adjacent Human animation/physics/equipment and
  engine endpoints are explicit doubles; actual DD0 template instantiates i386.
- Actual StaticWeapon actor/storage/seat/collision/control initialize, requested
  versus FIRST seat pose, navigation fallback/re-entry, forced release, occupied update, mounted
  reset, player/AI aim, reload/authority expiry, casing and ordered retirement.
- SAME Car seat source kernels with real mass/physics/control/mounted reset/
  SOUND/exclusion owners: positive assignment/transfer without seeded occupants,
  engine start/stop/rejection, input masks, blocked/missing-floor exits, alternate
  seat search,200/left-side placement, query exceptions and release. Car geometry,
  containing actor container, query/frame/camera/B0/registry endpoints are doubled;
  actual CarLoadingActivation and World bindings instantiate separately i386.
- Human seat-action kernel→actual StaticWeapon assignment→occupied mounted frame
  →exit→resource teardown on the SAME owner. Adjacent Human services and engine
  endpoints remain explicit doubles; no populated application World execution.

A targeted additional native ASan/UBSan/LSan audit passes100 engine admission
families with actual feature storage, independent lighting flags, health/fuel/
force/audibility and explicit audio endpoints. It verifies the corrected F6C
owner after a final source check found an accidental5B0 lighting-state read.

Strict actual nested World/Human/Car/Items/Physics i386 instantiation and the
application launcher/startup probe build pass. Windows outputs were compiled and
linked only. Temporary audit sources/binaries are fingerprinted and removed;
research exports and verification logs remain ignored WORK. No desktop test.

## Evidence under ignored WORK

`analysis/work/mission-world-20261103` is a local copy-on-write snapshot of the
base HUM evidence; shared tool installations are symlinks. Original client
SHA256 `8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
Index lookups precede every new export. Existing `human-actions-explicit-abi-v3`,
`human-action-entry.asm`, `human-action-update.asm`, `human-car-frame-field-uses.asm`
and `human-reaction-static-seat.asm` retain their provenance. New rollback-only
`ExportH3SeatsABI.java`, `h3-seat-abi` and`h3-seat-abi-v2` correct seat signatures
and selected engine call sites; use raw assembly for uncorrected calls. The first
`h3-seat-original` directory is EMPTY: ExportFunctions rejected absent004D9BE0
before exporting anything, although headless returned0. No missing-inventory
function receives credit. `h3-state-direction-vtables.json` records B0+50/54
across all eight vtables; raw00583100/120 copy four words,0056FC10/20 delegate
actual existing AI direction124 setters.

Mounted math evidence: rollback-only `ExportH3MountedABI.java`,
`h3-mounted-original`, `h3-mounted-aim-abi-v2`, `h3-mounted-ai-aim.asm` and
`h3-mounted-shell.asm`. Import Normalize/SetRot3 signatures and frame24/1C,
SOUND30/8C call overrides matter; default decompilations lose stack provenance.
004C5080 raw5439..546B uses velocity30..38 for the fourth random offset, not the
saved forward50..58 used by SetDir at549B. Native44-byte Physics parameters
start stack64: callback08=0, context0C=this (unused without callback), material10=12,
first-list16=true/sight-pass17=true, angle20=pi/2, radius28=-1.
Native dead displacement0075E560 is preserved in
`human-dd0-surface-producer-range.asm`75E560..75E600; it SetPos/copies base30,
restores shot scales and calls actual007B0910(flags8,zero,NULL,reason0,NULL).
