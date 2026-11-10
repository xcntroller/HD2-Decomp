# P2.8 dependency-ordered delivery guide

Source observations pin c09c22c; refresh actual source at implementation time.
Research-ready means a bounded contract is available. It does not mean source is
implemented, native-verified, connected, world-integrated or desktop-accepted.
All source paths below are reusable owners, never permission to introduce a
second world. Details/evidence: [entry](shared-world.md), [modes](authority-modes.md),
[players](player-control.md), [packets](packet-synchronization.md),
[review](evidence/source-review.json), [graph](p2.8-graph.json).

P2.8's mandatory Phase2 boundary is status9/shared request/source cleanup with
honest unavailability. Positive live Connect/Host and full synchronized gameplay
are Phase3. The following small batches distinguish the two boundaries.

## B28.metadata — selected map and owned request data

**Smallest batch independently startable after accepted P2.3. Contract-ready.**
Original006AD580/590/5B0,006AD6E0/780/820/990,006ADB70/BD0,006ADC30/70,
006B8700/006B9190. Extend existing `frontend/multiplayer_maps.h`,
`game_data.h`, `pre_mission_handoff.h`, `game/session_owner.h` and
`game/session_selection.h` with actual nonoffline descriptor/rules/peer request
admission. Preserve campaign request validation. Need complete selected metadata,
kind12, actual COW copy/borrow semantics and application-owned mission lifetime;
the browser name/style catalogue alone is insufficient. Use actual catalogue and
configuration owners. No actual network transport required for source completion.

Native verification: paired bit serialization with asymmetric signed16 readers,
2-bit objectives, empty/nonempty collections, actual map→GameData field mapping,
kind12 identity, repeat replacement and invalid selection. Demonstrate source
request can distinguish valid nonoffline empty campaign squad from invalid offline
empty squad. Confirm packet storage can retire after metadata copy. Original reader
destructively replaces maps; safe truncation/bounds rejection must be documented
as a native deviation, not atomic original rollback. Desktop: no new positive
connection route exposed; existing UI must still report unavailable transport.

## B28.entry — controller9 and received command0

**Mandatory bounded receiver; contract-ready.** Predecessor B28.metadata and real
P2.3 game/session owner. Original006C20F0,00730A40,0072E890,00731610,
00728450(case0),00698510/006989B0,0069E620. Reuse actual application current state,
retained menu, `ApplicationController`, `multiplayer_connection.h`,
`session_activation.h` and shared packet/input process owners.

Implement distinct7/8 connection owner and9 game receiver; unwrap retained current
before temporary menu destruction. Preserve preprocess order, one actual game,
GameData importer, descriptor lifetime, localized player resource registration and
failure transition to Main. Received0 uses its own mutation order and enqueues9
only after initialize.00724110 replaces the process input manager before old-world
shutdown; main currently keeps Inputs as a world member. Explicitly adapt canonical
manager ownership using the SAME existing type/profiles/getters; do not add a
second input manager. See [initial sync](initial-sync.md). Do not require false network success to exercise a pure
receiver. Do not publish loaded/connected state when required world/mode services
reject. Declare native allocation/null protections as deviations. Original failure
has already destroyed old state/config and does not guarantee rollback.

Native: trace receiver identity/virtual slots, current publication-before-menu
deletion, exact queue chosen by flags, borrowed peer3C clearing, input reset,
failure before/after construction, repeat/shutdown, and unavailable transport with
no spontaneous status9. Engine/service doubles prove only ordering. Desktop:
ordinary front end remains accepted behavior; real live9 is deferred Phase3.

## B28.mode — shared lifecycle admission and explicit mode tails

**Mandatory mode/request gate, bounded contract available; full positive modes
conditional.** Predecessors B28.entry and actual P2.3 modules/updates/finish.
Original006989B0,0048DB00,004270D0,0069AF10/B230/B870,00698F80.
Reuse `session_activation.h`, `session_frame_loading.h`, `session_finish.h`,
`session_render.h`, `host/game_loading_modules.h`, existing Scene2/Items/Script/
Players/HUD/Camera/FPV and process mode owner.

Centralize no new mode manager: distinguish process flags, configuration mode,
rules style and game phase. Keep19-slot registry; bit1 andbit2 activate fresh passes
independently. Preserve one authority clock location; dedicated omits presentation.
Offline live-squad spawn and network rule/spawn are separate branches. Source may
remain explicitly unavailable at a required unrecovered positive tail; identify
which receiver is absent before mutation. That is not full nonoffline integration.

Native: modes7/3/2/1 produce original activation/clock/render/finish ordering; no
duplicate authority/presentation Human; actual registration/unbind identity. Test
both flags and configuration mismatches without inventing a ready state. Desktop:
offline regression under accepted P2.3; network mode acceptance needs Phase3.

## B28.queues — process/application and input interface

**Required source interface; runtime history positive path remains conditional.**
Predecessors B28.entry, actual P2.3 process packet bindings. Original0072BEF0/BF90,
0072C170/230/360/3B0/5D0,0072A0B0/B1C0,0072D0E0/00745EA0.
Reuse StartupSession's `SessionBroadcastPackets`, `SessionLocalPackets`,
`SessionPacketWorldLink`, `HumanInputShared`; don't copy buffers into a world owner.

Add actual request2BC/target2F8 and application envelope queues, preserve drain vs
enqueue, channel/queue split, phase routing, deep payload ownership and unlock
before decoder. Full input histories require manager virtual14/18, not a callback
returning true. Real send interface must distinguish unavailable submission from
successful delivery; ignored original returns are not proof of success. Connection
queue/socket/ACK worker implementation remains Phase3, not a P2.8 prerequisite.

Native: bit cursors, terminator placement, metadata routing20/22/42, local versus
target ID conversion, queue survival across unbound worlds, free current payload
on decoder failure while preserving later packets, shutdown without dangling
callbacks. No sockets required for source-order verification. No desktop fixture
can claim connection or synchronization.

## B28.spawn — actual peer/Human identity binding

**Conditional positive game path; architecture ready, actual rule/spawn and input
leaves required.** Predecessors B28.metadata/mode/queues and actual P2.3 factory,
equipment, world quantizers and registry. Original0069E340,00611C90,00612A60,
0059EDC0,006125A0/006123A0,0072E940,00730140,00730460.
Reuse `prepared_human_spawn.h`, `offline_spawn_presentation.h`, WorldActors,
`HumanRoster`, `human_input_loading_manager.h` and actual B0/Items/clothing/profile.

Implement separate peer slot/transport/Scene2/HumanStore mappings, nonoffline
identity writeback under418, existing-authority reuse, local-only camera binding
and ordered stat fields. No new actor ownership graph. Source currently rejects
these nonoffline branches. Complete actual reached spawn validation/rule consumers
before calling positive path ready; missing spawn emits original0F event.
Native paired writer/reader and two-distinct-identities cases must detect swapped
ECX/EDX, generation and local/remote roster bugs. Retire input/peer borrowers before
Human destruction. Live spawn/respawn acceptance deferred Phase3.

## B28.consumers — typed module and Human action receivers

**Conditional runtime batch; partial contract, not wholesale implementation-ready.**
Predecessors B28.queues/spawn and P2.3 actual module/actor services.
Original0048DC50/70,00424590,004788A0/E0,004755D0/00475630,
005A5F60/70/80/B0,005406C0/00540DB0,00592FE0/00583220,
005418E0/00541CB0. Wire paths are explicitly mapped in the packet document.

Extend actual module virtual28/24 bridge, preserve false defaults, reserved-slot
state gates and exact discard grammar, no unbounded success/skip. Implement only
actual reached opcodes/command payload readers plus real Human/Items receivers.
Manager histories, full command helper bit layouts and actor50/54 nested snapshots
remain named research/implementation gaps; outer0x15 and module12 framing plus
module38/3C dispatch are now recovered in [initial sync](initial-sync.md). Script5F source can be reused but does
not stand in for every Script opcode. Versions/HeavyFireSupport/effects only become
required when reached by target mission/packets. Native grammar/identity fixtures
must use genuine formats and expose missing branches; no transport required for
bounded decoding tests. Live motion/actions and full state sync remain Phase3.

## B28.retirement — borrowed references, failure and repeat

**Mandatory bounded source cleanup; contract-ready with actual P2.3 retire leaves.**
Predecessors B28.entry/mode; spawn/queues if those paths are admitted.
Original0069E720,00722180 terminate,00746B50/00746D20,00698F80,
006C20F0(status11), existing retained P2.5 shutdown. Reuse actual process vector,
registry retirement, Players overrides/HUD/spectator, TeamMgr auxiliary and world
packet link. Disconnect clears original slots without shifting, detaches borrowers,
retires actual Human then clears3C/stats. Worker unregister precedes deleting
connection; process queues persist across world retirement. Network finish retires
application live squad rather than writing campaign snapshots.

Native failure matrix: failed packet metadata after destructive map reset, failed
initialize, load-count mismatch0x15/status11, connection termination while retained,
late queued packet after world unbind, repeated enter/exit and application shutdown.
Validate one current/retained game and one retirement of each real owner. Do not
claim rollback after old state was destroyed. Exact engine failures and live
disconnect behavior need Phase3 evidence; offline shutdown remains P2.3/P2.5.

## B28.boundary — source completion and deferred acceptance

**Mandatory final verification batch.** Predecessors metadata/entry/mode/queues/
retirement bounded interfaces. Conditional spawn/consumers are not universal
blocking edges for the Phase2 source checkpoint. Document every positive path
still rejected, source/native/integration/desktop levels, real transport interface
and deferred Phase3 tests. Run affected builds/signature/ownership/native sanitizer
checks when implementing; this research checkpoint changes no game source.
No successful mock receiver, fabricated peer, fake ready server or hidden mode7
substitution can close the boundary. Use [acceptance matrix](acceptance.md).
