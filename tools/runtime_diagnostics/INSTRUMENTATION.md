# Diagnostic model and focused instrumentation proposal

Baseline: local master `ca321e2`, 2026-11-10. These are proposals only; no game
source is changed. Parser v1 recognizes existing strings, not proposed markers.
Source-only/native verification remains separate from runtime evidence.

## Established sequence and caveats

Preparation descriptor kind7 → construct type1 GameApplicationState/session →
load resources and actors/modules → activate same session → first camera/frame
callbacks → update/render loop → finish/unload/retire. This is a dependency model,
not a rigid log ordering assertion: construction/initialize can precede the
controller's post-frame type1 trace; first camera delivery can occur during
activation spawning; render precedes fixed/update in the outer frame.

- `recovered/host/pre_mission_menu.h::handoff` logs the validated request;
  `application_launch.cpp::transition_pre_mission` receives it. Request metadata
  is not the mutable live squad or a published loaded World.
- `game_owner.h::GameApplicationState::initialize`, original00698510/006987E0
  owner and006989B0 activation: existing owner log explicitly says loaded=0.
  Current host deliberately declines activation and returns false. Source bodies
  for real loading/activation exist independently; the application graph remains
  unbound. A type1 snapshot is not proof of successful activation.
- `recovered/game/mission_loading.h::load_mission`, original006CA780:
  scratch →006D3F20 preparation/prior unload → animation tables → scene open/
  refresh →006CBF30 Scene2 →006D45C0 actors →006D4880 sounds →006D4280 checks →
  **then** test Scene2 status0 →006EE570 collision tree → effects → loaded14 →
  loading-screen finish. Reverse scratch release precedes006CADE0 module tail,
  including failure paths. The native C++ boolean true maps to original return0.
- `recovered/game/session_activation.h::activate_offline_session`,006989B0:
  loading success/loaded14 → car/world/light preparation → fresh ascending
  authority module pass (+14) → HUD → presentation module pass (+1C) → first
  person → status5 → live squad spawn/packet-camera callbacks → dubbing/text/
  music/loading-clock tail. Activation original nonzero success differs from
  loader zero success. Status5 alone precedes required tail completion.
- `recovered/game/mission_module_dispatch.h`: fresh ascending registration reads
  matter; unload +18 is ascending, finish +20 reverse. Packet world link binds
  after modules and unbinds before destruction. Never infer successful rollback
  merely from an application teardown marker.
- `recovered/game/session_finish.h`,00698F80: preserve presentation/dubbing,
  authority/module and HUD finish order, shared transient cleanup and retirement.
- `recovered/host/game_owner.h` frame callbacks are guarded in this baseline.
  `analysis/frame-render-source.md`:0069A560 control,0069AF10 fixed,0069B230
  update; render0069B870 is absent from frozen inventory (saved assembly only).
  Compiled callbacks and aggregate frame counts do not establish presentation.
  Startup fade can mask the screen for approximately 3–4 seconds.

Contracts reviewed: [loading](../../analysis/mission-loading-integration.md),
[World](../../analysis/containing-world-loading.md),
[activation](../../analysis/session-activation-integration.md),
[packet/camera](../../analysis/process-packet-world-binding.md),
[frame](../../analysis/frame-render-source.md),
[accepted launch](../../analysis/launch-acceptance.md),
[launcher documentation](../../recovered/README.md).

## Current observability

Host stdout/stderr are unbuffered, merged into staged `host-runtime.log`, opened
with truncation each run. No timestamp/severity envelope exists; plain producer
strings are the reliable anchors. The Python launcher empties that log before
execution, saves relayed output and appends the direct log to `host-output.log`.

Existing anchors: launch header; Startup gate/prepare; controller type/sequence/
status/phase snapshots; Pre-mission handoff validated; Game owner constructed;
Game activation declined; Game owner initialization failed; Game entry failure;
loop-completion frame count/reason; owner cleanup; teardown result; fatal catch.
Resource-input capture is explicitly distinct from runtime loading. There are
no reliable positive stage markers for actual World/resource/module completion,
activation success, camera, world update/render or engine presentation. Cleanup
markers describe startup/application cleanup, not full mission cleanup. An error
outside recognized strings is an unclassified warning, never silent success.

## Proposed markers for Agent #1

Use one attempt ID, stable event name, original address, begin/end or result,
and explicit result convention. Avoid personal resource paths, pointers and
per-frame logs. Emit on actual calls; do not make a service succeed to emit them.

| Existing source / original stage | Record | Diagnostic value | Lifetime |
| --- | --- | --- | --- |
| `host/application_launch.cpp::transition_pre_mission`, kind7 handoff | Attempt ID and descriptor kind, begin/outcome | Correlate failure return and retry without squad/name dumps | Permanent boundary |
| `host/game_owner.h::initialize`,00698510/006987E0; `host/game_world_loading.h` containing owner | Session and World constructed separately, attempt ID | Distinguish scene owner from actual World/module graph | Temporary until integration stabilizes |
| `game/mission_loading.h::load_mission`,006CA780 | Loader begin/end, native boolean and original return convention, loaded14 | Locate failure and distinguish conventions | Permanent boundary |
| Same function,006CBF30/006D45C0/006D4880/006D4280/006EE570 | Stage begin/end, recorded Scene2 status, exception stage | Diagnose last completed resource/actor/tree stage; preserve delayed Scene2 test | Temporary, bounded per load |
| Same function, scratch release/006CADE0; `game/mission_module_dispatch.h` | Tail begin/end and registration count; failure stage/index on exception | Verify required failure tail and fresh ascending pass | Permanent summary; temporary failure detail |
| `game/session_activation.h`,006989B0 | Begin/end, success, loaded14/status, exception stage | Distinguish status5 write from full activation completion | Permanent boundary |
| `host/game_loading_modules.h` / `game/session_packet_world.h`, packet camera receiver0048DC50/70 | First successful camera binding per attempt, source route, valid frame flag | Establish actual first camera without raw pointer output | Temporary, once per attempt |
| `host/game_owner.h::render/fixed_update/update`,0069B870/0069AF10/0069B230 integration | First entry/completion of each per attempt, elapsed/fade state | Separate callback reach from completion; permit render before update | Temporary, once per callback kind |
| `host/application_launch.cpp` actual graphics submission boundary | First successful world submission/present return where API exposes it, owner type | Separate renderer from presentation; no fabricated present result | Temporary, once per attempt |
| `game/session_finish.h`,00698F80; `host/game_owner.h::shutdown` | Finish begin/end, failed component, packet unbound, owner retired | Detect incomplete cleanup and compare retry ownership | Permanent summary |

Exact host file ownership may move as Agent #1 integrates the graph. Check the
actual call boundary before adoption. Module activation/finish summaries must
preserve native order, not introduce extra passes. No marker alone replaces user
desktop evidence of a visible scene/camera or acceptance of failure/repeat.

## Source marker adopted — 2026-11-10

`host/game_world_loading.h::World` now emits once after its actual containing
owners, vertex preference and packet-world link are constructed:

```text
P2.2 event=world_constructed result=success loaded14=<0|1>
```

`loaded14` reads the SAME actual session flag. This is construction only, with
no loader/activation/render/presentation or desktop-success implication. Failed
construction before that boundary emits no positive marker. Application loading
still declines in this source checkpoint; no runtime observation is claimed.
An attempt ID awaits the actual application binding and is not fabricated here.

Parser v1 does **not** recognize this new format. Add a coordinated parser rule
and focused classification verification before treating it as an automatically
classified construction stage. Current parser behavior and acceptance policy are
unchanged. Contract: [shared projection/source boundary](../../analysis/human-projection-frame-process.md).
