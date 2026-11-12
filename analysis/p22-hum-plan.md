# P22-HUM implementation and dependency plan

Revised 2026-11-12; D0 confirmed against merged master **415e850**. Worktree:
`/home/control/Documents/Projects/HD2-Decomp-p22-hum-h6`, branch `feature/p22-hum-h6`
(created from completed H2 `85593e8` at the user's request; prior Human
worktrees/branches preserved). P22-HUM and both P2.2 delivery
checkboxes remain OPEN. Application loading still declines; no mission desktop
acceptance has occurred. This plan replaces the former chronological next-work
list. Detailed behavior/evidence stays in the linked contracts.

## Branch ownership — user clarification 2026-11-12

H1, H3, H5 and H7 are finished and merged into `feature/p22-hum`. Completed
aggregate201adbf is integrated; [integration evidence](human-completed-branch-integration.md). H2 is source-complete at85593e8. H6 is now assigned to
this worktree/agent; [full H6 plan and D ownership](p22-hum-h6-plan.md). H4 is active with another agent on its separate branch;
do not import or implement its work here. H8 remains unassigned. Finished status does not assert that a branch's
source is present in this checkout or add function-inventory credit.

H2 completes its reactions/damage/ragdoll source and verifies its own owner,
producer and retirement boundaries. Reached H4/H6 contracts remain explicit
external dependencies, not permission to take over those milestones. H8 owns
full Human-frame assembly and verification across the branches; H2 must not
require completion of H8 before delivering its own result. Existing required
source failures remain visible, and application/desktop acceptance stays separate.

## Completion boundaries

Labels describe the plan: H = Human milestones, D = required dependencies, and
S = earlier verified source pieces. Lowercase a/b/c split one dependency into
reviewable parts. A checkpoint is implemented source with focused verification;
its JSON receipt records hashes and results. It is not application/desktop acceptance.

A checked source checkpoint means its stated source boundary and focused checks
are complete. It does not mean every function is complete, that a containing
frame runs, or that application/desktop delivery is accepted. Function inventory
credit is separate: current source34 complete/3250 partial; [joint/clip kernel inventory
reconciliation](checkpoints/p2.2-human-ragdoll-kernel-source-20261112.json) is complete,
while full containing-World/application verification and external producer
limitations remain open.

The **H2 reactions/damage/ragdoll source checkpoint is complete**: the actual
Human/passive B0/inline98/E0/DD0/230/Effects graph now executes the owned reaction
segment, retained clips/joints, damage, failure/re-entry and ordered teardown.
Existing contact/fall/blast/death contracts retain their stated verification limits.
Authored data and explicit engine/OS/lookup endpoints are not a running World.
The earlier broad exit map is classified by the clarified H1–H8 branch ownership
below; external Car/network callers are not blanket H2 source gates. Full Human frame
verification belongs to H8 and depends on H3–H6 where reached; H2 records those
interfaces and dependencies without assuming the other agents’ work.
P2.2 additionally requires the other workstreams and actual user acceptance.

## Human milestones

- [x] H1 — Model presentation through actual007A7470 dispatch.
  [Source checkpoint](checkpoints/p2.2-human-model-presentation-20261111.json).
- [x] H2 — Reactions, damage and ragdoll on actual Human/control owners.
  [Source closure and limits](human-h2-source-closure.md), [receipt](checkpoints/p2.2-human-h2-source-20261112.json).
  Full World/frame/application verification remains H8/P2.2.
- [x] H3 — Offline Car/StaticWeapon entry/exit and mounted receivers on existing owners.
  [Verified source](checkpoints/p2.2-human-seat-interaction-20261112.json); [contract](human-seat-interaction-source.md).
  [H2 integration](checkpoints/p2.2-human-h3-integration-20261112.json) retains D4b/D6b1 owners and D5 guards.
- [ ] H4 — Common AI outputs: reached00584E30 completions, B0 action/navigation/
  formation, track events and DD0 transition/attachment consumers.
- [x] H5 — Guard/Area decisions:0057CC50 cover/door,00581E30 floor lists, kind9
  release and actual Scene2 actor dispatch. [Completed source](checkpoints/p2.2-human-guard-area-source-20261112.json),
  integrated from completed aggregate201adbf; H8/desktop verification remains separate.
- [ ] H6 — Player items:00542E20 held item,00545140/005454D0 shots and reached
  command/FOV/dry-fire/melee/repair/transport consumers. [Active implementation/dependencies](p22-hum-h6-plan.md); [held/math/notification controller checkpoint](human-player-item-controller-source.md). Actual item firing and full H6 verification remain open.
- [x] H7 — Reached breath/blood particle and rest-footprint source through SAME
  process/module17 lifetime. [Checkpoint](checkpoints/p2.2-human-particle-surface-20261111.json).
- [ ] H8 — Full Human frame: H2–H6, actual World/control dispatch, positive paths,
  partial failure/re-entry/ordered teardown, consolidated verification and audit cleanup.

Earlier mounted/remote equipment, motion/terrain/Look, bone/IK/weapon posing,
aimed-posture workers, mount007BA0C0 and head007ADA10 source remain verified in
[Human frame source](human-frame-source.md). Do not restart those audits.

## H2 source checkpoints already achieved

Each item is deliberately limited to the boundary named here. Later dependencies
below still apply; these are not whole-subsystem or desktop claims.

- [x] S01 — Joint impulses, integration, constraints, point/body contacts, Car
  forwarding, sleep/substeps, pose publication and contact/audio retirement source.
  [Contract and focused verification](human-ragdoll-source.md#source-progress);
  [kernel source/inventory receipt](checkpoints/p2.2-human-ragdoll-kernel-source-20261112.json).
- [x] S02 — Retained clip007B7410/007B3C80 collision/transitions and actual ragdoll
  007A7470 branch. [Contract](human-ragdoll-source.md#retained-clips-actual-frame-dispatch-and-rest-surfaces).
- [x] S03 — Actual rest007B37C0→00787110, shared ShotMgr surface pool and retained
  footprint queue. [Checkpoint](checkpoints/p2.2-human-particle-surface-20261111.json).
- [x] S04 — Reached reaction selection0074D510/007B0910, obstruction/contact/fall
  damage, inventory/death/presentation and selected retirement source.
  [Damage](checkpoints/p2.2-human-damage-presentation-20261111.json),
  [authority/retirement](checkpoints/p2.2-human-reaction-authority-retirement-20261111.json).
- [x] S05 — Reached objective/session scoring, application scalar FIFO and peer/
  console publication. [Objective](checkpoints/p2.2-human-death-objective-session-20261111.json),
  [peer/console](checkpoints/p2.2-human-peer-console-20261111.json).
- [x] S06 — Selected-map/rules metadata, borrowed request admission, cold peers/
  Human creation and reached disconnect retirement.
  [Metadata](checkpoints/p2.2-human-network-mission-metadata-20261111.json),
  [peer lifecycle](checkpoints/p2.2-human-peer-lifecycle-20261111.json).
- [x] S07 — Application presentation2/10/17, canonical input ownership/reset,
  pending-input/FIFO reset and optional voice owner; received0 prefix only.
  [Application](checkpoints/p2.2-human-application-packets-20261111.json),
  [input](checkpoints/p2.2-human-process-input-20261112.json),
  [entry prefix](checkpoints/p2.2-human-entry-input-voice-20261112.json).
- [x] S08 — Reached Car/Human contact and protection HUD, presentation/authority
  spawn, match/HUD/camera/transmit targets and zone/rejection/map source.
  [Contact](checkpoints/p2.2-human-car-contact-20261112.json),
  [presentation](checkpoints/p2.2-human-presentation-spawn-20261112.json),
  [authority](checkpoints/p2.2-human-authority-spawn-map-20261112.json).
- [x] S09 — Containing blast→Human, runtime Mine9C, normal Mine producers and
  module12 packets, armed Mine/explosion and positive Versions transitions.
  [Blast](checkpoints/p2.2-human-containing-blast-20261112.json),
  [Mine owner](checkpoints/p2.2-human-runtime-mine-20261112.json),
  [Mine producers](checkpoints/p2.2-human-mine-producers-20261112.json),
  [armed contact](checkpoints/p2.2-human-mine-contact-20261112.json),
  [Versions](checkpoints/p2.2-human-versions-transitions-20261112.json).
- [x] S10 — CarD4 blast/zone/seat damage, DM effects, authority destruction and
  nearby Human damage. [Blast/DM](checkpoints/p2.2-human-car-blast-damage-20261112.json),
  [presentation](checkpoints/p2.2-human-car-dm-presentation-20261112.json),
  [destruction](checkpoints/p2.2-human-car-destruction-20261112.json).
- [x] S11 — Queued physical/mesh/glass reset, actual glass fragments and detached
  Item12 construction/launch/lifetime source.
  [Reset](checkpoints/p2.2-human-car-physical-reset-20261112.json),
  [fragments](checkpoints/p2.2-human-car-fragments-20261112.json),
  [Item12](checkpoints/p2.2-human-car-item12-20261112.json).
- [x] S12 — Detached Car rigid-body motion/contact through SAME World/body/cache.
  [Checkpoint](checkpoints/p2.2-human-car-rigid-motion-20261112.json).
  Fresh part5F9 and wheel695 remain explicit producer failures; warm fixtures
  seeded known prior state and do not establish a fresh-path success.
- [x] S13 — Car wheel2/Human9 requests, actual module13 authority routing and timed
  DM dispatch. [Checkpoint](checkpoints/p2.2-human-car-contact-request-20261112.json).
- [x] S14 — Car fuel10 and collision impulse11 through SAME streams, physical
  state and activation2584 clock; both packet receivers and unowned hit58 route.
  [Checkpoint](checkpoints/p2.2-human-car-fuel-impulse-20261112.json), commit b132e6d.
  Native regression also corrected004E0180's uninitialized source fourth lanes.

## Recovered H2 source and containing-world dependencies

These are implementation checkpoints, not an instruction to reconstruct every
adjacent Car or network feature. Before expanding a dependency, record the actual
selected trigger/caller and required owner. Shared entry support is source scope;
live synchronized multiplayer acceptance belongs to Phase3. Use existing owners,
and leave a nonreached wider feature with its explicit required guard.

### D0 — Confirm the remaining containing-path boundary

- [x] D0 — Record the trigger/caller/guard map for the remaining H2 path,
  linking the source contracts and native evidence below. Distinguish mandatory
  dependencies, mode-specific branches and neighboring H3–H6/Phase3 work. Include
  the original producer and retirement boundary for every state used by a new body.

[Completed boundary map](human-reaction-containing-boundary.md) names T1–T10,
actual producers/retirement, mandatory versus mode/occupancy dependencies and
merged UI limits. Direct ragdoll→Car004F2790 does not imply mesh deformation;
Car→Car/passenger004F2B60 does. Status9/received0 are entry-specific, not callees
of every offline reaction. D1–D3 and the H2-owned D10 segment are source-complete. D4/D5 and shared
entry work remain conditional containing-world integration; no transport
readiness or continuing occupied/non7 Car frame is implied.

The completed H2 source boundary is [recorded separately](human-h2-source-closure.md).
Unchecked D4–D9/D11 entries below are retained containing-world/Car/entry
integration obligations. Their exact guards remain; none is silently marked
resolved or imported from active H4. Full World integration remains H8's work.

### Car state producers, damage and retirement

- [x] D1 — Mesh backup ownership and reverse restore004FE910. Recover producer
  004FF8B0's resource/part matching, native658 vector owner/sharing, retained mesh
  and usage-count lifetime. Restore LOD0/1 positions with native lock/unlock,
  recompute/invalidation, flag4 clearing, decrement/release/erase order.
  [Verified source checkpoint](checkpoints/p2.2-human-car-mesh-backup-20261112.json); D2 source completion follows below.
- [x] D2 — Reached positive impact004FF8B0: light/projector damage, part thresholds,
  backup capture, variant/glass transitions, deformation and wheel puncture/
  detachment through SAME loading receivers. [Verified source](checkpoints/p2.2-human-car-positive-deformation-20261112.json).
- [x] D3 — Seat-associated Door visual restore: repair actual FA8+28 Door20 before
  variant visibility through existing geometry discovery/binding, including unoccupied Cars.
  [Source/corrected ownership](human-car-positive-deformation-source.md). Actual Human occupancy remains H3/D5.
- [ ] D4 — Containing Car frame: periodic event6/reset, incremental event7 inventory,
  pending targeted damage and reached navigation observations. Bind actual producers through
  authority004D2790, presentation004D3110 and004DB360; preserve timer/water/earlier-failure ordering.
  Required internal dependencies:004DB2A0→virtual44/004DD0D0 reset→004DCE40,
  004E95B0(2), SAME physical/collision/skid/particle owners and saved pose;
  004DE1F0→007D7810(NULL destination)→007D8B40→actual Car storage;
  004DEAD0 pending producer→004DEC10→existing004DBF00. Non7 presentation/occupied
  reset consumers compose with D5. Raw004D3336..335D proves259C's reverse
  observations call navigation0051D5A0 via game54, with unused results, not SOUND.
- [x] D4a — Verify event6/reset, incremental event7/Items storage and pending
  damage source receivers over SAME physical/inventory owners, including failure prefixes.
  [Verified source checkpoint](checkpoints/p2.2-human-car-frame-events-20261112.json); [contract](human-car-frame-events-source.md).
- [x] D4b — Compose common B0/00586200 subtype14 item/target/transform admission
  through actual Player authority and Remote presentation command3 receivers.
  Original Player/Remote/AI vtables share DC; keep Player presentation consume-only.
  Occupied/non7 Car reset/frame consumers remain D5; neither checkpoint closes D4 alone.
  [Verified source](checkpoints/p2.2-human-car-target-command-20261112.json); [contract](human-car-target-command-source.md).
- [ ] D5 — Reached occupied/non7 Car activation/control/frame through existing
  004D16A0/loading lifetime. Resolve actual driver/input/mounted borrowers;
  retain offline7 gates. Verify any reached non-Car9A8/4A0 physical mapping in
  004D3D10 or prove that the selected branch cannot produce it.
  H3 committed10bebe9 is integrated: mounted reset/seat receivers are source.
  Its containing occupied/non7 simulation remains D5; retain that guard.

**Exit per item:** actual producer and consumer compose on SAME owners; focused
native positive/failure/repeat/lifetime checks pass, actual host i386 instantiates,
and launcher/probe rebuild passes. Test doubles must be named. Compile-only or
manually seeded backup/occupant state does not close a missing producer task.
D1–D5 dependencies are conditional on D0's actual path; unsupported neighboring
features stay listed under their owning Human/Phase3 milestone.

Evidence: [reset/restore](human-car-physical-reset-source.md),
[fuel/impulse](human-car-fuel-impulse-source.md),
[activation](car-activation-loading.md). WORK has
`human-car-restore-fuel-checkpoint-index.txt`, `human-car-global-mesh-backup-index.txt`,
`human-car-deformation-producer-index.txt`, `human-car-backup-container-index.txt`,
`human-car-part-restore.asm` and `human-car-deformation.asm`. These are research,
D1 implementation/evidence is in the [mesh checkpoint](human-car-mesh-backup-source.md).
Native654/658 is the physical object's owned vector (constructor004E2900 and
append004FC320); no process global cache is introduced. [D2/D3 source](human-car-positive-deformation-source.md)
closes positive damage and Door visual restoration; D4/D5 remain open.

### Shared entry and process lifetime

- [ ] D6 — Actual peer6C/client334 and containing worker/caller lifecycle.
  D6a closes cold connection/registry/entry receivers only; D6b remains required.
- [x] D6a — Cold00745D20 queue/clock/lock ownership and retirement,00746B50/
  00746D20 registry and typed00730A40 entry mutation. No live worker/protocol claim.
  [Verified source](checkpoints/p2.2-human-entry-connections-20261112.json);
  [contract/remaining boundary](human-entry-connection-source.md).
- [ ] D6b — Actual007466E0/007474E0 worker/socket storage,007467B0/00747690
  start callbacks and00746840/00747750 stop/retirement; client334/peer6C
  publication, port/register and disconnect deletion through SAME owner.
  Client00723D20 publishes334 before port/register; host007260A0 registers
  after418 unlock. D6b1 is verified source; D6b2 remains. Generic coldD4 is
  unproduced; actual singleton loader-zero and native start are proven producers.
- [x] D6b1 — Worker/socket/control lifetime source: actual callbacks, ACK/ping,
  typed caller prefixes and reverse-delete/stop/storage ordering.
  [Verified boundary](human-worker-control-source.md); no live session claim.
- [ ] D6b2 — Actual admission/callback composition and reached retained payload/
  queue/fragment receivers; preserve same client/server, process414 and app11.
  Required before containing D6b/D6 closes; not a Phase3 live-session test.
- [x] D6b2a — Received fragments through SAME socket30/buffer14: retained sorting,
  expiry, contiguous assembly and retirement. [Source](human-fragment-reassembly-source.md).
- [x] D6b2b — Retained payload serialization, queues/retry/ACK order and SAME414
  delivery into actual process430 FIFOs, with ordered retirement/reset.
  [Verified source](human-retained-message-source.md); application entry stays open.
- [ ] D6b2c — DirectPlay admission/name/address/record and complete/terminate
  callback composition with SAME client04/server00 and current application11.
  Reconcile the existing menu-lived discovery client with process lifetime;
  termination requests status11 on the actual current application state.
- [ ] D7 — Status9 composition:00730A40→0072E890→00731610(true), retained-game
  restoration BEFORE menu shutdown, SAME00698510/006989B0 owner and actual request.
  Depends on D6 and existing S06/S07; preserve its shutdown/delete failure path.
- [ ] D8 — Received application0 composition: actual00724110/00723000 manager
  replacement BEFORE old-game retirement, metadata/map/import/peer/request9 and
  distinct failure deletion. Depends on D6/S06/S07; do not alias it to status9.
- [ ] D9 — Reached non7 load/zone and common frame/reset/retirement consumers.
  Preserve mode gates, actual input/voice/process/world ownership and borrower
  unbinding before destruction. Depends on D7/D8; no fake successful transport.

**Exit per item:** original publication/failure prefixes, repeated entry and
ordered retirement exercised over actual source owners, with explicit OS/engine
endpoints only; strict host i386 and launcher pass. Do not require a Phase3 live
network session merely to close these source receivers. Current guards are open
work, not proof that the necessary live service is already available.

Evidence: [shared entry contract](multiplayer-world/shared-world.md),
[entry/input/voice checkpoint](checkpoints/p2.2-human-entry-input-voice-20261112.json).
Reuse WORK `human-entry-connection-originals`; voice final ABI-v3 has EIGHT args,
interface18 and publish0C before initialization. Do not repeat its completed audit.

### Human composition and H2 closure

- [x] D10 — Execute the H2-owned0077DE30 reaction segment→0074D510→007B0910→
  007A7470 branch on actual Human/B0/inline/E0/DD0/230 owners, including retained
  clips, joints, two-Human source borrowing, actual damage, failure/re-entry and
  retirement. [Contract and test limits](human-h2-source-closure.md). Full World
  dispatch and H4/H6 consumers remain H8/external interfaces.
- [ ] D11 — External Car producer limitations: fresh part5F9, wheel695,
  EXPL record8 and smoke14. Preserve confirmed failed prefixes and guards;
  these are reached by the separate Car/debris simulation callers, not by the
  Human-owned reaction segment. No producer/default or full-World unreachability
  is invented. [Exact failures](human-reaction-containing-boundary.md#source-closure-blockers).
- [x] D12 — Consolidated H2 source verification, inventory and audit cleanup.
  Native100×8 actual-owner families, strict actual-host i386 and launcher/probe
  links pass; [closure receipt](checkpoints/p2.2-human-h2-source-20261112.json).
  No complete promotions, permanent tests or desktop claim. Original solver/
  engine/ABI equivalence and full cross-branch execution remain outside this checkpoint.

**H2 source exit is satisfied.** The new owner graph closes the earlier
selection/control/geometry-double gap. D11's original omissions and D4–D9
integration remain visible on their actual callers; H2 completion does not turn
their required failures into successful services. H8 verifies the entire Human
behavior set in the actual containing World/application, and P2.2 requires
application/desktop delivery. H4 stays separate; H6 is active on its own branch and H8 remains unassigned.

## External integration and acceptance

These remain visible dependencies, owned by their existing workstreams. They
are not silently included in a Car leaf checkpoint or claimed by a source fixture.

- [ ] P22-OWN/FRAME — Actual application loading and scheduling over SAME World,
  scene, modules, request and actor graph; required positive integration borrowers.
- [ ] P22-UI/RENDER integration — UI-owned source consumers are now merged and
  [source-closed](p22-ui-source-closure.md). Bind real Human/Actor producers and
  containing frame/render callers; preserve the existing UI ownership boundaries.
- [ ] P22-LIFE integration — Compose the already verified offline lifecycle source
  with newly enabled positive owners and the application failure/re-entry path.
  [LIFE source plan](p22-life-plan.md) remains source-closed, not desktop-accepted.
- [ ] P2.2 user acceptance — Actual mission scene/camera, failed-load Main return,
  repeat loading and ordered exit on the user's desktop. No original EXE/DLL/
  Proton execution and no Windows fixture execution; never claim an unperformed test.

## Checkpoint discipline

Research each selected dependency's complete boundary before implementation;
use the research index before new exports. Resolve dependencies until H2 completes
or a concrete blocker prevents it. Check off and commit each completed, verified
checkpoint above, then continue; do not wait for the whole H2 milestone to make
progress visible. Add a newly discovered required dependency here with its caller,
owner and exit test before following it, rather than appending another “continue”
paragraph. Do not reopen unchanged verified kernels without new contradictory evidence.

Consolidate contracts/receipts/ledger/handoff at those checkpoints. Use focused
checks while implementing and a consolidated audit at D12. Heavy compilation and
Ghidra run sequentially. No permanent tests, no push, no unrelated worktree edits.
Interactive behavior changes still require user desktop confirmation before
committing; source-only checkpoint commits need no invented desktop test.
