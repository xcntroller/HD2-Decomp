# Phase 2 — dependency-led delivery and function completion

Planning milestone,2026-11-03. User direction supersedes the exhaustive pre-Phase2
gate: **postpone the standalone completion pass and finish functions alongside
Phase2 delivery.** There is no requirement to review all530 partials first.
This is a researched delivery plan, not a claim that world loading or gameplay
has been implemented. [Main checklist](../plan.md), [milestone index](phase2-delivery-plan.json).

## Working rules

- Select coherent delivery batches by the next observable outcome and its original
  call graph. Research that batch's entire implementation boundary before coding;
  reuse confirmed evidence and existing audited code. No function-count quota.
- Include required partial-function work in the implementation batch. Prefer
  integrating existing recovered bodies into their actual owners over creating
  another independent implementation. Finish bounded full bodies when their
  contracts are available; keep broad controller/loader functions partial while
  other original branches remain. A delivered feature is not whole-function credit.
- Prioritize real behavior, file compatibility, request/state transitions and
  ownership/failure correctness. Recover exact ABI where crossing DLL/virtual,
  serialized or shared raw-object boundaries. Internal layout-only cleanup and
  byte matching can wait when they do not affect that delivery. Any remaining
  difference stays explicit in the partial ledger; standards for `complete` do
  not change. Source-only completion remains separate from host integration.
- Use instruction matching selectively for ambiguous numerical, parser, save and
  lifetime behavior. Do not make compiler-byte equality or unused padding work a
  universal milestone gate. Genuine engine/OS delegation earns no body credit.
- Use quick affected-source checks during implementation and one consolidated
  build/sanitizer/regression/evidence sweep per substantial batch. Repeat only
  affected checks after a concrete failure. Update the ledger once at the end.
- Integrate and obtain user Python-launcher desktop acceptance for interactive
  behavior. The agent never runs original game/DLL/server/Proton/Windows fixtures.
  Use existing tests and temporary native audits; remove the latter. No packaging
  or push; commits follow the existing source-only/desktop-acceptance rule.

A milestone ends with its delivery result, original functions completed or still
partial with exact remaining branches, integration level and verification evidence.
A newly discovered dependency is added to that milestone if needed for the outcome;
otherwise it stays in the backlog. Do not restart a general530-function audit.

## Confirmed starting boundary

The accepted client already owns real preparation data and a copied
`PreMissionGameRequest`: mission resource/metadata, mode, squad and equipment.
`pre_mission_handoff.h` validates it; `application_launch.cpp` still installs the
explicit unavailable owner for kind7. `StartupSession::world_` is null. A menu
screen or a successful request validation cannot substitute for a game owner.

The existing [single-player](singleplayer-pipeline.md#briefing-and-game-entry)
and [multiplayer](multiplayer-pipeline.md#completion-boundary-and-blockers)
contracts establish separate entry paths to00698510. Planning now also checks the
actual game-state vtable in [fingerprinted evidence](phase2-boundary-map.json):

| Boundary | Confirmed static evidence | Remaining work before implementation |
| --- | --- | --- |
| Single player | 006C0EF0 kind7 constructs00698510, gets mission006B9190, invokes state+04 with it | Reconcile complete caller ABI, guards and request ownership |
| Game activation | Constructor installs008159AC; slot+04 is006989B0, which calls006CA780 at00698A56/00698CD1 | Validate all virtual prototypes, initialization/failure states and reached wrappers |
| Owned dependencies | 00698510 constructs0049A420/00490740/00523AB0, creates engine kind13 scene and installs006C9260 callback | Recover the actual subowner, callback, scene/camera and teardown contracts |
| Scene stages | 006CA780 calls006D3F20→loading setup,006D4180 Scene,006CBF30 scene2 | Recover ordered side effects, stage results and unwind; opaque/protected calls need real resolution |
| Gameplay loop | Vtable+0C/+10/+14 target0069A560/0069AF10/0069B230 | Trace actor, input, simulation, camera, script/objective and sound dependencies |
| Multiplayer | 006C20F0 status9 calls00730A40/0072E890/00731610 before00698510 | Actual transport/ready-session/live acceptance stays Phase3 |

Vtable+08 targets00698F80. Targets+18/+1C/+20 are0069B870/006987E0/0067EA00;
these are reference addresses, some absent as standalone frozen inventory entries.
Slot role/prototype verification is part of P2.0. No new function credit comes
from this mapping. Loader zero-success versus activation one-success is visible
in the cached exports/result branches; verify typed interfaces before wiring.

Existing [mission-format research](mission-format.json) covers base ALPS1 Scene2
framing/selected fields and two model prefixes, not a complete mission loader.
The 4DS loading/rendering engine remains the original LS3DF initially. Its verified
[model interface](engine-model-map.json) and [candidate policy](model-load-policy.json)
are reusable evidence; implementing a complete replacement engine is outside Phase2.

## Delivery order

| Milestone | Outcome | Prerequisites |
| --- | --- | --- |
| P2.0 | Verified first-mission receiver/loading contract and dependency map | Existing accepted client/evidence |
| P2.1 | Real game/session owner consumes a valid preparation request and tears down | P2.0; an already accessible genuine preparation route |
| P2.2 | One original mission reaches its first real world frame | P2.1 and required scene/resource consumers |
| P2.3 | A controllable player and functioning mission loop | P2.2; actor/input/collision/script dependencies |
| P2.4 | Complete preparation/loadout/briefing flow feeds real gameplay | Existing preparation implementation; P2.3 for progress-gated acceptance |
| P2.5 | Retained pause/player/options flows resume the same game | P2.3; P2.4 where inventory data is required |
| P2.6 | Actual gameplay save/load/restart and front-end resume | P2.3/P2.5; real serialization consumers |
| P2.7 | Live mission/campaign results and continuation | P2.3/P2.4/P2.6; real result generation |
| P2.8 | Shared multiplayer game receiver contract and cleanup | P2.1/P2.3; live transport validation deferred to Phase3 |
| P2.9 | End-to-end Phase2 acceptance and remaining-function record | P2.2–P2.8 with explicit Phase3 live-network exclusion |

First visible target is P2.2, then P2.3. Full loadout acceptance and ready-server
work do not block that first level. Ordering after P2.3 can follow discovered
prerequisites; pause precedes in-game save acceptance, and real result production
precedes debrief acceptance. No calendar estimate follows from function counts.

## P2.0 — Establish the first receiving path

Research the complete boundary from the existing kind7 request through state
activation to mission loading. Confirm caller/callee conventions, state codes,
required globals, owner lifetimes, engine callbacks and actual failure returns.
Follow the vtable targets above and decode only protected/wrapper calls reached
by this path, using the existing startup resolution method. In particular,
006CA780's00783350 call and protected-looking activation code are dependencies
to resolve, not permission to return synthetic success.

Choose the earliest normally reachable mission with confirmed descriptor/resource
resolution; Tutorial is a candidate from existing GameData evidence, not a fixed
filename assumption. Record patch/loose/archive precedence and required assets.
ALPS1 remains a second structural sample until its ordinary route is selected.
Do not bypass real campaign progress merely to expose equipment screens.

P2.0 is targeted preparation inside the P2.1 implementation batch. Continue into
owner implementation once its boundary is ready; do not stop for another standalone
audit-only milestone. Deliver a call/ownership/dependency map, one mission acceptance
scenario and a bounded implementation list for P2.1/P2.2. Separate client bodies to reconstruct,
real engine delegates and unresolved interfaces. Original addresses and inventory
membership must be explicit. Research/discovery alone earns no function credit.

## P2.1 — Construct the actual owner and consume the request

Implement the game/session owner, subowner construction, global publication,
engine scene/camera/callback setup and deterministic shutdown/partial-startup
cleanup. Transfer owned mission/squad/equipment data at the proven boundary;
preserve active-soldier selection, identifiers and ownership across menu release.
Bind real world/audio views only while the actual owner exists. Resolve constructor
and matching cleanup functions together; constructor success alone is insufficient.

Function seeds:00698510,006989B0,0049A420,00490740,00523AB0,00698F80,
006C9260 and needed branches of006C0EF0/006C20F0/006B9190. Select matching
subowner destructors from original call evidence; names/prototypes are not assumed.

Integrate into the existing controller. Until required loading succeeds, report the
actual incomplete/failure outcome and leave readiness false. Validate repeat entry,
construction-stage failures, retained references and return to Main through real
recovered paths. Desktop world delivery is still P2.2. Complete bounded owner bodies
where possible; shared controller functions retain their other explicit exclusions.

## P2.2 — Load and render one real mission

Recover the ordered006CA780 pipeline: preparation/loading screen006D3F20/006C9640,
Scene resource006D4180, scene2 stage006CBF30, root dispatch006CC760 and object
reader006CD940. Reuse actual archive/directory/chunk/resource owners. Confirm
progress006C95D0 and all required post-scene/tree.klz/effects stages from the
original callees; do not assume the existing inspector implements them.

Apply required object creation/lookup, names, parents, transforms, models and
mission data to the real scene. Include existing engine scene frames in name
resolution; parent names absent from Scene2 are not automatically broken links.
Recover006CC580's registration callback even though it lacks standalone frozen
inventory membership. Unsupported required data fails explicitly. Optional ignored
fields must follow original skip behavior, not a guessed universal skip.

Original Scene2 has large unresolved branches. Implement the complete dependency
set for the chosen level, extend to a second mission before final acceptance and
keep006CD940 partial until its full original body is reconstructed. No whole-body
credit for the existing resource-key and candidate-classification fragments.
Original LS3DF remains the model/scene engine; do not rebuild4DS as a side project.

Success: actual selected mission assets, original loading presentation, real scene
and camera reach a world frame through the controller, with readiness tied to
completed stages. Native audits cover real parser/resource/callback bodies and
truncation/missing/bad-resource failures. User verifies the frame, load errors,
return and repeat loading through Python. A loading screen, log message or raw
model preview does not satisfy this milestone.

## P2.3 — Activate player and mission simulation

Trace and implement the game owner's input/fixed/update/render dependencies:
0069A560/0069AF10/0069B230, render target0069B870 and controller006C3100/006C3190.
Recover actual actor spawning and consumption of selected soldiers/equipment,
player/camera control, collision/physics, required weapon/damage interactions,
mission scripting/objectives and mission sound. Inventory these new bodies from
that actual call graph; do not invent actor/AI addresses or fake a static roster.
Broader AI/script variants remain explicit until their required consumers exist.

Integrate the existing source-complete input/configuration bodies when the live
owner needs their contracts. Resolve producer006C7870, buffer query006C72B0 and
manager lifetime gaps as required by this input path. Do not run a separate full
cursor/SEH/localization pass before player delivery; recover those parts when they
change reached input, ownership or failures. Reuse existing working host bridges
where compatible and record any remaining original-body differences.

Success: the chosen soldier is controllable, carries selected equipment, interacts
with real world/collision and required mission logic, and can reach a genuine
objective or failure condition. User verifies focus loss, resolution/fullscreen,
input bindings, sound and return/re-entry. A controllable camera alone is not a
playable mission. Batch actual input/numerical/actor checks with this integration.

## P2.4 — Finish preparation, equipment and briefing delivery

Use the running receiver to finish the existing descriptor-driven routes,
Campaign/Lone Wolf/Carnage and standalone handoff. Verify equipment constraints,
stock/preset persistence, required items, active squad, movies/subtitles and Back.
Do not replace descriptor ordering with a fixed screen sequence. Functions start
with006C0EF0/006B9190/006B9680 and the existing pre-mission ledger/contracts;
select exact equipment/actor consumers from that evidence rather than re-auditing
all94 preparation entries automatically.

Loadout screens remain COMPLETELY UNTESTED until accessible. Test legitimate
progress-gated routes when P2.3 makes them available; keep that acceptance pending
without blocking P2.2. Prove changed squad/equipment reaches actual spawned actors.
Finish bounded required body gaps, integrate them and obtain user acceptance for
the complete preparation→briefing→game route, including repeated Back/cancel.

## P2.5 — Retain, pause and resume the real game

Recover0069E620/0067F350/0067FB00/00680680 and existing in-game control contracts.
Implement owner retention, input/audio/pause order, mode eligibility and normal
resume; integrate in-game Options/dialog variants with actual world/session data.
Player/equipment panels start from00682980/006835E0/00684390/00684890/0067A770.
Addresses0067F550/006827A0 are action-dispatch evidence, not inferred operations.

Implement save-dependent actions after P2.6 and network-only actions after Phase3.
Research each operation before wiring; unsupported services remain explicit.
Verify repeated pause/resume and focus/modal cancellation preserve the same owner,
scene, player and input/audio state. Trace fixtures prove ordering only; user
verification from a running mission is required for delivery.

## P2.6 — Actual saves, restoration, restart and resume

Recover format/version/path/metadata and actual game serialization/restore owners
before enabling successful load/save. Seeds:006C3340,006391E0,00680680 and
controller006C20F0, plus reference-only save entry addresses00626B20/0067F550/
0068CE00. Follow original call sites for the serializer bodies; profile.sav and
front-end stored captions are not gameplay-save implementations.

Use isolated copies of compatible original saves where available; never modify
original profiles. Integrate world/actor/equipment/script state, reference fixups,
mission compatibility, progress/error/cancellation and owner replacement order.
Save formats/numerical encoding are high-priority matching candidates when static
comparisons help resolve ambiguity. No invented save browser or file-exists success.

Success: real state changes survive a save/load round trip, restart recreates the
intended initial state, Main resume restores the actual saved game, and corrupt/
missing/incompatible or failed writes preserve coherent ownership and honest UI.
Native parser/serializer fault audits plus user gameplay verification are required.
Keep metadata/load/save source closures separate from desktop acceptance.

## P2.7 — Produce results and continue the actual campaign

Wire real objective/failure/soldier-result production into mission debrief,
conditional diary and next/retry/Main-return. Function seeds0068D2F0/0068EAD0/
0068F3D0/0068E0F0/0068D5C0, with0068CE00 as reference evidence; campaign variants
0068BD10/0068BDD0/0068BDE0/0068C0F0 and reference0068BD70. Preserve the confirmed
20-row campaign table's empty/null/sentinel distinctions.

Integrate descriptor advancement, progression and persistence with P2.6 rather
than acknowledging a fixture result as mission success. Verify a genuine result,
retry/next mission, diary/Back and return Main, including failed persistence and
absent results. Captured-result UI/native tests remain separate from live results.
Close full bounded result helpers and retain wider mode/network exclusions.

## P2.8 — Shared multiplayer receiver, Phase3 live dependency

Recover status9 mode/request preprocessing00730A40/0072E890/00731610 and shared
00698510/006989B0 receiving behavior; preserve distinction from status7/8's
connection owner0068A710. Integrate owned settings/map/player request contracts,
correct failure teardown and source-level readiness requirements with the shared
game owner. No extra lobby or invented connected callback.

Phase2 accepts this source/receiver boundary only. Real Connect/Host, ready-server
advertising and host/client/browser/password/console desktop acceptance require
the rebuilt controlled server in Phase3. Thus Phase2 does not wait for Phase3 to
supply its own prerequisites. Actual live status9 acceptance stays an unchecked
Phase3 item. Fixtures never establish network success or game synchronization.

## P2.9 — Consolidated delivery acceptance

Verify two genuine mission/resource paths, preparation through live gameplay,
required objectives/failure/results, pause/options, actual save/load/restart/resume
and return/re-entry. Record platform/render/language/config and isolated-data
versions actually exercised. Reuse the accepted Phase1 matrix and run affected
regressions; do not reopen every accepted menu or repeat full archive hashes.

Report each owning subsystem's delivered behavior, completed/partial function
entries and exact remaining branches/dependencies. The530-function backlog is not
an exit criterion. Required unsupported behaviors keep their corresponding delivery
items open; source-only status9 work and Phase3 live gates stay separately labeled.

Stop after one verified delivery milestone per turn under AGENTS.md. Future tasks
start with the next dependency-ready Phase2 checkbox, not the postponed audit queue.
