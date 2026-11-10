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
| B26.preparation | B26.envelope | research-blocked leaf payloads | Recover application persistence without world |
| B26.identity | P2.3,B26.preparation | research-blocked actor leaves | Restore actual identities, never a parallel world |
| B26.script | B26.identity | research-blocked exact context meanings | Restore VM, frame links, pending work |
| B26.snapshot | B26.envelope,B26.identity,B26.script,B25.retention | research-blocked remaining modules | Live round trip/reset/failure |
| B26.resume | B26.preparation,B26.snapshot | research-blocked006B7410 and failure path | Main/retained load, explicit/quick operations |
| B27.commit | B24.preparation,B26.preparation | research-blocked arithmetic/producer | Commit genuine squad results/progression |
| B27.presentation | B27.commit,B26.resume | partial existing UI contracts | Debrief/diary/campaign/retry/next |

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

## B26.preparation

Entries006B6A50/006B6DE0, wrappers006B71C0/006B72B0 and reset006B6FF0.
Reuse application owners, profile plumbing, current pre-mission records and
catalogue; see [child map](p2.6-save-load.md). Missing exact GameData/mode/HumanStore/
store/variables payloads006B4250/4400,006B46B0/47B0,006A1780/1B40,
006A57B0/5890. Script global00427010/27050 is now exact0x400 current bytes. Soldier reader006A4940/006A4E00 is field-based;
verify paired writer006A2020 before claiming format compatibility.
Failure: original resets/destroys existing state before some reads; no rollback
assumed. Retire old soldiers backward; dependent world references must be detached
or rebound by actual owning path. Focused verification: field-by-field original
stream pairs, live/no-game conditional skips, partial-read/allocate failures.
Desktop: no independent gameplay claim; later preparation retry/next profile route.

## B26.identity

Entries module12 00477B90/00477F50, module13 00476370/004764D0,
006E0890/006E0B70 and actor+48/+4C methods. Reuse actual WorldActors/Items/process
registry, never save-specific duplicates. Existing slots must be populated in
matching order before positional replay; dynamic allocation/relationship prelude
must match before script/objective borrowed lookups. Missing: all relevant Human,
Car, item and other reached actor payloads, fixed/runtime IDs, relationship triplet
semantics and00476A50 reconciliation. Failure: partially restored pointers cannot
outlive actual actor owners. Verification: null first32 slots, dynamic slots,
cycles/missing IDs, references surviving replay and ordered teardown under native
sanitizers. Desktop: actors/inventory/target identities after actual world load.

## B26.script

Entries00424970/004257B0,004147D0/00414E30,00427010/00427050.
Reuse mission_script_module_loading/contexts/variables and scene lookup/compiler.
Frame names and compiled binding/context layout precede restore. Raw fields and
pending records are mapped in [context note](p2.6-context-layout.md); exact meanings,
all special frame resolvers still block whole batch; global variable payload is exact0x400 current bytes.
Failure: old contexts/rotation owners retire first; unknown context chunks fail;
missing frames/compile errors retain diagnostic paths. Do not reconstruct a fresh
successful context or lose pending work. Verify scalar/variable/reference stream
order, kind-specific byte, stack cardinality, wait/timer/rotation/animation resume,
missing binding and partial mutation. Desktop: saved suspended script resumes once,
objectives/pending timers continue with real actors.

## B26.snapshot

Entries0069C8E0/0069CFC0,0048DC90/0048DE60,0069C390.
Reuse same nineteen module owners; route virtual30/34 through verified targets.
Source recovery of another lifecycle slot does not supply persistence methods.
Required top-level order: preparation → module state → HUD → objective/marker →
subowner8000 → presentation → FPV reset → optional missionA8. Missing remaining
module leaves and mission virtual+10/+14 format. Whole batch is NOT ready.
Failure: wrappers ignore nested failures, no atomic transaction; report native
rejection honestly and preserve explicit source deviations if cleanup is improved.
Verify mutation prefixes, short writes/backpatch failures, wrong mission/version,
repeated restore, post-load presentation and borrowed-pointer cleanup. Desktop:
actual state changes survive snapshot reload, including squad/items/VM/objectives.

## B26.resume

Entries006DCAE0 command dispatcher,006C3230/006C38F0/006C3ED0/006C4070,
0069C340,006B7A00/006B7D30/006B7410 and006B3C90/006B3D00.
Dispatch only after update with actual mode/status/6E gates; keep commands distinct
from state statuses. Command1 restart;2 quick save;3 quick load;4/5 explicit path;
6 frontend resume when no game. Reuse profile directory/caption writers and ordinary
controller descriptor transitions. Missing final006B7410/006B7D30 contracts,
all caption alternatives and safe failed-init/load unwind before enabling.
Failure: explicit filename clears and delay100 may occur even on failure; quick
resume destroys frontend before new game activation, no proven rollback.
Verify explicit/quick/nonquick split, open failures, frontend destruction order,
receiver failure cleanup, command clearing and repeat. Desktop: Main resume,
same-world load, restart, cancel/fail and return Main on isolated profile copies.

## B27.commit and B27.presentation

Entries0068C300/006B12D0/0068CE00 and0068D2F0/0068EAD0/0068F3D0/
0068E0F0/0068D5C0; campaign0068BD10/0068BDD0/0068BDE0/0068C0F0.
Reuse real objective/squad producer and B26 preparation persistence, accepted
menu identities, profile reader/writer. Missing producer→descriptor handoff,
rank/medal/stat formulas, exact randomized redistribution and repeated-entry gate.
Apply results before profile progress before preparation save before continuation
queue; original dispatcher ignores save result, so do not claim atomic advancement.
Retry from prior non-live save, standalone recreated route, conditional Diary,
null/empty campaign variants retain distinct behavior. Failure: model/load errors
are real, save failure must not be hidden by an invented success; explicit native
policy differences require review. Verify exact raw field mutations, randomized
draw count, dead-member reverse erase, zero survivors, repeated modal cancellation,
failed persistence and table sentinels. Desktop: genuine mission completion/failure,
stat changes, diary/Back, retry, next and Main/reopen persistence. Captured UI-only
results cannot close these acceptance boundaries.
