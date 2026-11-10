# Source coverage at c09c22c

This is an inspection of actual C++ and revisioned main notes, not a new audit of
accepted Phase1 or a new desktop test. Exact file digests/line observations are in
[source review](evidence/source-review.json); function-ledger source claims are a
separate graph layer. Main was reviewed at
c09c22c01d1049d7f087142064a3c2c53488f582. No function receives reconstruction credit
from this research.

| Boundary | Actual source and owner | Coverage / remaining gap |
|---|---|---|
|Browser/create/connect/error front end|`frontend/multiplayer_maps.h`, `multiplayer_settings.h`, `multiplayer_connection.h`, host menu services|Reachable Phase1 UI historically user-accepted2026-11-01. Ready-server operations remain untested; no new acceptance inferred|
|Host/Join submission|`host/multiplayer_connection_menu.h`|Actual callbacks explicitly return unavailable. Game-entry callback logs unavailable. No live transport/session|
|Application controller|`frontend/application_controller.h`|7/8 source connection route,12/13 retained video;9 and14..17 unrecovered transition throw|
|Mission request/selection|`frontend/pre_mission_handoff.h`, `game/session_owner.h`, `game/session_selection.h`|Actual offline owned request and mutable squad borrower; nonempty squad/campaign root guards. Full MP map metadata/importer/kind12 missing|
|Scene/world/module lifecycle|`session_activation.h`, `session_finish.h`, `host/game_session_activation.h`, `host/game_loading_modules.h`|Shared real source/native owners exist; nonoffline activation/finish tails explicitly unavailable. App still declines loading|
|Fixed/variable/render coordinator|`session_frame_loading.h`, `session_render.h`|Recovered source, actual world service bindings; no live mission acceptance, nonoffline frame tails rejected|
|Process queue/world link|`session_local_packets.h`, `session_broadcast_packets.h`, `session_packet_world.h`|Same process streams/FIFOs/HumanInput owner survives world; source/native offline binding verified in main notes; request2BC/transport/nonoffline pump not supplied|
|Human/Items/profile spawn|`prepared_human_spawn.h`, `prepared_spawn_packets.h`, `offline_spawn_presentation.h`|Actual factory/equipment/packet/decoder source and same-Human offline loopback; rejects nonoffline identity/positioning/presentation branches|
|Module packet dispatch|`host/game_loading_modules.h`|Actual registered module selection, kind13 presentation bridge; reserved Human and other positive packet receiver paths throw|
|Input runtime|`human_input_loading_manager.h`, `human_input_records.h`|Cold/default/profile/borrowed callback source; not complete runtime history transport/playback|
|Script presentation|`mission_script_visibility.h`|Actual5F receiver, positive nonoffline publication rejected; not all Script opcodes|
|Phase3 server/network|Paused `feature/networking` at e777570|Existing research/server startup only. Read through Git; no worktree changes, no ready endpoint assumption|

The current absence of a P2.8 source branch is separate from original architectural
reuse: one existing World/Items/Human/Players/Camera is the required integration
target. P2.3 implementation/desktop acceptance is external to this research.
