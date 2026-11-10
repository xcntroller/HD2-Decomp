# Ordered implementation batches

These batches are proposals for Agent #1, not authorization to enable incomplete
services. `contract-ready` means the bounded contract is available; it does not
mean source/integration/desktop delivery. Any listed blocker keeps that batch
`research-blocked`. P2.2/P2.3 remain external prerequisites; no implementation
work is requested in this branch.

| Batch | Required predecessors | Research state | Purpose |
| --- | --- | --- | --- |
| B24.preparation | P2.3 | contract-ready for existing route integration | Carry actual preparation into spawned actors |
| B25.retention | P2.3 | partial; pause/audio leaves remain | Retain same world through menus/Options |
| B26.envelope | none for source recovery | contract-ready bounded I/O and metadata | Separate metadata, preparation and snapshot envelopes |
| B26.preparation | B26.envelope | partial; bounded child batches below | Existing application persistence |
| B26.identity | P2.3 | partial; registry/proximity dispatch closed | Loaded identities and actor replay |
| B26.script | P2.3, B26.envelope | partial; scalar/stack/Whenever closed | VM state on actual loaded frames |
| B26.snapshot | B26.preparation, B26.identity, B26.script, B26.modules, B25.retention | blocked by nested leaves | Whole live snapshot |
| B26.resume | B26.snapshot, B26.failure | blocked whole path | Parent for live/quick/restart/frontend receivers |
| B27.producer | B24.preparation, P2.3 | bounded extraction/order available | Capture actual Human results before world death |
| B27.awards | B27.producer | arithmetic available; lifecycle partial | Rank, medal and redistribution |
| B27.commit | B27.awards, B26.preparation | blocked lifecycle/persistence | Commit actual results/progress/save |
| B27.presentation | B27.commit, B26.resume | partial existing UI contracts | Debrief/diary/campaign/retry/next |

## B24.preparation

Required behavior/entry:006C0EF0 descriptor kind6/5/7,006B9190 actual mission;
application GameData owns descriptors, squad+14 owns soldiers, stores+18/+1C own
quantities. Reuse pre_mission_flow/data/route/squad/inventory/handoff headers and
ordinary BriefingApplicationState; preserve [single-player](../singleplayer-pipeline.md)
order and original capacity/required-items/firearm gates. Missing: verify real
world consumes same soldier IDs/equipment; do not bypass progress for equipment.
Failure/teardown: reverse/Back returns inventory exactly once and destroys only
current screen owners; request keeps descriptor valid through receiver lifetime.
Focused verification: actual request→actor identity/equipment trace, stock balance,
Back/cancel loops. Desktop: accessible loadout, briefing then actual mission with
changed equipment; prior team acceptance does not cover untested loadout.

## B25.retention

Entries0069E620/0067F350/0067FB00/00680680 and006C20F0 statuses14–17.
Menu+5B8 borrows game; application current pointer is restored before menu
shutdown/destruction. Reuse actual frontend stack/Options definitions and input/
audio services. Missing: in-game Options variants/player panels and pause clocks,
shared game+6E/audio state. Keep P2.6 actions unavailable until their real receiver.
Failure: allocation failure retains original game; modal cancel pops actual owner.
Focused verification: pointer identity and input/audio/pause order across repeated
resume15 versus leave16, allocation-failure and nested modal. Desktop: real
mission pause/options/player/ordinary resume with same scene/actor, focus changes.

## B26.envelope

Entries00405620..004059F0,006C47C0,006B03B0,006B0520,006C3340/006C39F0.
Reuse io/chunk_file.h and profile_load/save/payload/directories; no new save browser.
Implement exact six-byte total-size framing and independent metadata/full-reader
gates, filename-owner lifetime and checked native malformed-input policy.
Failure: missing/short/incompatible stays failure, do not equate file-exists with
restore. Original ignored writes/unsafe lengths stay documented limitations.
Focused verification: isolated headers/chunk seeks/backpatch/truncation, wrong
version/root/mission, negative metadata ordinal distinction and resource cleanup.
Desktop boundary: labels/eligibility only until full receiver closes.

## P2.6 child batches and execution order

Parent IDs above remain stable. Child IDs below are the bounded work units;
`contract-ready` means source work can start with the stated existing owners.
It does not authorize enabling an incomplete load command. Build order may
prepare services first; replay must keep the original stream order.

| Batch | Mandatory predecessors | State / exact work boundary |
| --- | --- | --- |
| B26.preparation.mode-stock | B26.envelope | contract-ready: mode225C, stock sparse framing, current globals0x400 |
| B26.preparation.squad | B26.envelope, B24.preparation | partial: paired soldier/equipment fields and aggregate current stream; full GameData/HumanStore and legacy aggregate remain |
| B26.identity.registry | P2.3 | contract-ready: preserve mission slot/generation/retired and runtime ID domains |
| B26.actors.items | B26.envelope, real catalogue/holder owners | contract-ready: four item+44/+48 subtype formats and original common-tag asymmetry; actual catalogue/holders required |
| B26.actors.base | B26.actors.items, initialized model/holder owners | contract-ready base format; actual inventory/model owners required |
| B26.actors.car | B26.actors.base, initialized Car solver/parts | blocked variable packed version10 leaves |
| B26.actors.human | B26.actors.base, B26.identity.registry, reached vehicle/target owners | outer and state selection closed; nested B0/input/animation/AI leaves open |
| B26.actors.conditional | B26.actors.base where actually called | small actor pairs available; other reached families partial, catalog is conditional |
| B26.identity.dynamic | B26.identity.registry, B26.actors.items, reached type2/11 payloads | dispatcher/item formats closed; type2/11 physical leaves and type35 actual motion receiver remain |
| B26.identity.proximity | B26.identity.registry | contract-ready: entries/speeds/buckets/three-byte pairs, no notifications |
| B26.identity.replay | B26.identity.proximity, all reached actor payloads | positional module13 dispatcher closed; payloads block full replay |
| B26.script.layout | P2.3 compiler/scene/bindings, B26.envelope | contract-ready for scalars, integer slots, references, Gosub, Whenever and globals |
| B26.script.async | B26.script.layout, actual animation/rotation owners | blocked pending animation receiver, derived contexts and module leaves |
| B26.modules | P2.3, matching real module owners | use classifications; close required nested consumers, preserve empty wrappers |
| B26.live | B26.snapshot, B26.failure | same-world explicit save/load receiver; blocked transitive snapshot |
| B26.quick | B26.live, profile directory/caption publication | quick filename and command gates closed; blocked snapshot |
| B26.restart | B26.preparation, B26.failure, real current mission initialization | receiver known; verify full reset/reinit path before ready |
| B26.frontend | B26.preparation, B26.snapshot, B26.failure | preparation reader/path closed; destructive new-world activation failure open |
| B26.failure | relevant reset/shutdown owners and each reader mutation contract | partial: no rollback is supplied; close failed initialization/repeat teardown |

`B26.snapshot` aggregates these child contracts; `B26.resume` aggregates receiver
batches. Neither parent is ready while any reached required leaf remains open.
Conditional actor/module requirements must come from the supported mission's
actual registry/resources, not a guessed “standard mission” exclusion list.

## Preparation: mode, stock, squad and aggregate

Exact entries006B6A50/006B6DE0, wrapper006B71C0/006B72B0, reset006B6FF0,
frontend reader006B7410. Use existing application owners and profile plumbing;
reuse recovered frontend/pre_mission_data/squad/inventory/route rather than new
save-only records. [Child map](p2.6-save-load.md) defines ownership and live-reader
skips. Mode006B46B0/47B0 is16 raw bytes at+4 inside225C. Stock006A57B0/5890 puts
quantity AFTER leaving1234(index), not inside it. Global00427010/50 is exactly
0x400 current bytes; do not write previous snapshot or declaration metadata.
Soldier006A2020→006A46D0 and006A2150→006A4940 are field streams; equipment
006A4D00/006A4E00 is exactly six bytes. Aggregate006B4250/4400 wraps006A7A30/7C80;
current0x6F/70 vectors and legacy0/2 handling remain separate compatibility cases.

Failure: stock/aggregate readers clear first; preparation reset retires old
soldiers before some reads. No transaction or silent successful empty fallback.
Verify exact emitted/consumed byte positions, sparse stock zero entries, repeated
reads, live skips versus frontend no-skips, partial allocation/reads. Desktop:
no gameplay credit alone; real retry/next/profile persistence later exercises it.

## Identities and actor payloads

Reuse SceneActorRegistry, WorldActors/Items and actor_proximity_loading services.
`00475D50` can find retained retired actors for null reserved slots; do not turn
that into a factory. Runtime004772F0 reverse-searches separate IDs. Module12
`00477B90/00477F50` uses type-specific recreation004793F0/00478E50 and existing
lookup for35; module13 `00476370/004764D0` uses existing slots, no saved type/ID
per record. `00476A50` recomputes Human counters, not references.
[Actor contract](p2.6-actor-persistence.md) and [catalog](evidence/actor-persistence.json)
supply exact virtual48/4C targets, chunks and nested readiness. Reuse existing
Human/Car loading owners and their real teardown. Human1001 creates selected B0
before1003; mounted actor/target identity must already resolve. Model/solver/
holders/compiled bindings must exist before decoding their state.

Failure: type2 recreation can return null that outer reader dereferences; base
inventory clears before rebuilding; Human state changes and Car packed restore
can partially mutate. Native guarded failure may improve this, but must remain
an explicit deviation and clean resources through actual registries, not a new
parallel factory/transaction. Release borrowers before actual owners. Verify
reserved null slots, compacted mission indices, retired lookup, runtime IDs
unrelated to position, missing mounted target, item subtree failure and repeat
cleanup. Desktop requires actual changed actors/equipment/vehicle state after
reload; a decoded object list cannot satisfy it.

## Script and registered modules

Use existing mission_script_program/context(s)/variables/module_loading and real
scene/compiler. `00424970/004257B0`, `004147D0/00414E30` restore after binding frame
resolution/recompile. [Context mapping](p2.6-context-layout.md) closes scalar/PC,
integer `reusable==false` slots, retained named frame references, Gosub stack and
nine bytes per already compiled Whenever record. Pending animation2600 is not a
generic event queue; the current tick rejects it and needs its actual receiver.
Use the three original frame resolvers in order; no reserved magic-name factory.

Actual module read order4 Script→11 Team→12 runtime actors→13 mission actors.
The world identity prerequisite means loaded owners, not actor payload replay
before Script. [Classification](p2.6-module-persistence.md) distinguishes verified
no-ops, reset-only readers and nonempty/conditional modules. Keep0..18 wrappers,
slot IDs and order. Close Tracks/Items/Script/Sound/Versions/LMaps/Team/Particles
leaves when reached; projectile reader skips four recognized tags even though
writer can emit nonempty payload. Optional missionA8+10/+14 remains unresolved.

Failure: context reader releases references and mutates before possible0;
module dispatcher ignores nested returns. No success follows from ignored AL.
Verify tag/order/counts against same compiled program, reference release/retain,
missing frame diagnostics, strict Whenever timing, pending animation/rotation
and frame-borrower retirement before world death. Desktop: real suspended script,
objectives, delays and animation continue once after load, with same real actors.

## Live, quick, restart and frontend receivers

Entries006DCAE0 dispatcher,006C3230/006C38F0/006C3ED0/006C4070,0069C340,
006B7A00/006B7D30/006B7410 and006B3C90/006B3D00. Preserve mode/status/6E gates and
dispatch after update. Commands1 restart,2 quick save,3 quick load,4/5 explicit
save/load,6 frontend resume are distinct from state statuses.

Live006C3340 uses exact same current mission/version, reset0069C390 and ordered
read; writer006C39F0 uses actual owners. Quick paths use profile directory plus
mode-specific quick name and publish caption/LASTSAVE through actual profile.
Restart006C3ED0 follows real teardown/reset/reinitialization and preparation
semantics, not generic snapshot reload. Frontend006C4070 obtains last path via
006B7D30, checks case-sensitive `quick`, reads006B7410 without live-game skips;
quick resume destroys frontend, constructs game00698510 then0069C340 initializes
real world BEFORE replay. Do not impose same-world mission guard on this branch.

Failure/repeat/shutdown: filenames may clear and delay100 be set even on failure;
frontend is already destroyed on failed new-game init. Close0069C390/full shutdown
cleanup tails before enabling these paths. Record mutated prefixes and dispose
through real owners in reverse dependency order. No original transactional rollback
is established. Verify explicit/quick/nonquick path ownership, short writes,
failed activation, command clearing, repeated reset and final application shutdown.
Desktop boundaries are separate: same-world explicit load; quick round trip;
restart current mission; Main resume into a new world; cancellation/failure and
return Main. Use isolated profile copies. No desktop result is asserted here.

## P2.7 producer, awards and continuation

`B27.producer`:0069BC40→0069B230→status6/2→006C0EF0, with offline6F/pause and
objective gates. `00698F80` must extract005A9040 into same squad via006B7390,
006A27C0/2A90/2E50 BEFORE actor/module cleanup. Existing preparation/Human owners
supply state; no saved actor stream is required for a fresh result. Verify actual
Human identity, side/equipment/deltas and teardown order; desktop actual mission
completion/failure feeds debrief, not captured UI results.

`B27.awards`: activation0068CCF0→0068CB70 produces score/counters/rank/medals;
accept/leave0068CE00→0068C300 consumes pending fields. [Results contract](p2.7-results.md)
records thresholds, overwrite/return medal semantics, nine PRNG draws per eligible
candidate and exact cleared-delta mutation. Bounded arithmetic is ready for source
work; protection producers, PRNG seed lifetime and repeated owner activation still
block whole lifecycle readiness. Focused checks: rank equality boundary, medal8
category uniqueness and overwrites, zero survivors, record70==1 versus other
nonzero values, reverse removal, signed mean, x87 stores and repeated activation.

`B27.commit` then006B12D0 progress→006B76E0→006C39F0 nonlive→continuation queue.
Reuse B26 preparation/profile format. Original ignores write success; do not
invent successful persistence or roll back mutation without documenting deviation.
`B27.presentation` reuses0068D2F0/0068EAD0/0068F3D0/0068E0F0/0068D5C0 and campaign
0068BD10/BDD0/BDE0/C0F0. Keep Diary mode gate, retry prior preparation save,
standalone recreated route and null/empty table variants. Verify modal cancel/
re-entry/failed write, real profile reopen and descriptor lifetime. Desktop:
actual result stats, retry, next, Diary/Back, Main/reopen; UI-only panels do not
close arithmetic, save or campaign acceptance.
