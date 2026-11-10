# P2.8 verification and desktop boundaries

All scenarios are **pending** at the reviewed main SHA. Static research and
native fixtures cannot establish a connected session or desktop acceptance.
The user performs desktop tests through the established reconstructed workflow;
never run the original EXE/DLL/Proton. No artificial peer/ready state bypass.

| Scenario | Prerequisite | Expected observable result | Required acceptance evidence |
|---|---|---|---|
|Offline regression|Accepted P2.3 real world and implemented shared receiver|Same real offline mission, squad/equipment/control and return remain intact|User mission/resource/config, identity and repeat record; native ordering alone insufficient|
|Unavailable host/join|Existing front end, Phase3 transport absent|Explicit unavailable/error; no status9/current game, connected or successful spawn claim|User UI/error and controller transition evidence|
|Bounded receiver failure|Native source integration with actual owner interfaces|Original preprocess/current/cleanup order; exact missing receiver causes failure; no fake connected state|Source/native trace pinned revision; explicitly **not desktop acceptance**|
|Listen entry|Rebuilt real Phase3 host/client transport|flags3, one game/scene, authority plus local presentation, local slot mapped|User host/peer logs, resource and actual game pointers/identity observations|
|Client received start0|Real Phase3 server and selected map/metadata|flags2 loads same world receiver without campaign squad requirement; request9 after initialization;0x15 gate before control|User start/packet/owner traces and real rendered/control state|
|Remote Human|Two genuine participants|Correct peer/actor distinction, remote actor visible but not added as local controlled camera Human|User per-peer identity/pose/item/camera evidence; captured packet decode alone insufficient|
|Listen loopback|Real authority+presentation session|Same Human reused; no doubled actor/inventory or duplicate local roster|User registry/roster counts tied to identities plus actual world observation|
|Movement/items/actions|Reached actual input histories/Human receivers implemented|Authority accepts actual action; both views converge without decoding desynchronization|Real two-peer movement/item tests and packet/phase evidence; no mocked positive receiver|
|World initial sync mismatch|Real Phase3 endpoints with genuine incompatible state|Count mismatch0x15 requests original status11; no success based on count alone|User failure/current-state/cleanup traces; no malformed-content bypass|
|Disconnect while playing/retained|Real transport; P2.5 nonoffline role source|Borrowers detach, actual Human retires, current/retained owner valid; wrapper unregistered before deletion|User peer departure/menu/resume/shutdown repetition and diagnostic identities|
|Repeated world/return|Accepted real mode entry and finish|One world retirement; peer references reset; new mission does not process stale world pointers|User repeated entry/leave/re-entry with state/queue/owner diagnostics|
|Dedicated distinction|Reconstructed Phase3 dedicated owner|flags1/no local slot or camera/HUD; authority world continues|Phase3-only server/client record; excluded from bounded P2.8 desktop target|

Source/native checks for B28.metadata can run without transport or a running world.
Other native owner tests require real loaded interfaces or explicitly labeled
doubles; a double's return cannot establish missing engine or wire behavior.
Phase2's source completion receipt must list rejected conditional paths and the
transport dependency, preserving the plan's Phase3 live-network exclusion.
