# P2.2 reverse critical-path research

Reviewed `origin/master` at `841d89a35c02de2c12c73bb3df8792e758450aed`, from the prior audit at `39b16571dcd4a111e2e077898e55625cc19b47b2`. P22-HUM is partially resolved. The checkpoint adds AI timer, steering and task-selection bodies with real World owner dependencies. Its receipt reports three native sanitizer fixtures, strict actual-host i386 compilation and launcher link. Application loading remains disabled and both P2.2 checkboxes remain open. [Blocker matrix](blocker-matrix.md) · [Validation](evidence/validation.json)

## Immediate findings for Agent #1

- **AI timer receivers.** `recovered/game/human_state_frame_timers.h` implements `00588010` but its process membership `0089B4E0`, target field94/life queries and current derived B0+0C completion are still supplied through template parameters. The World binds the new pools and Scene2 identity. The production Scene2 frame path still has no callers for Guard/Area prefixes or decisions. [Pinned Human observations](evidence/blockers.json)
- **Integrity startup.** The prior static audit located writers in `009905B0`, `009F02A0` and `009F0A60`. The latter two have caller `0099AEB0`. That gives a concrete starting point for producer order and validated observation publication. `recovered/host/startup_session.h` still owns an uninitialized table. [Integrity evidence](integrity-and-rendering.md)
- **Separate frame dependencies.** New B0 steering `00575940` leaves the Scene2 movement receiver `005647C0` open in `recovered/host/game_scene_actor_frames.h`. In rendering, `00482260` calls procedural `00482E50` before visiting the light list. The process texture state determines whether that draw path can return early. [Original findings](evidence/original-findings.json)

**Boundary A.** Finish reached Human and movement receivers, wire the frame and render services into the existing World, resolve integrity startup and publication, and connect application activation with partial-failure cleanup. A first desktop attempt may report a real stage failure. Reserved packets and asset-specific branches depend on the active mission. [Implementation order](implementation-order.md)

**Boundary B.** Verify the original scene and camera are presented after the fade, the reached updates run, and failure return and repeat loading work through the Python launcher. Extend coverage to a second original mission per the detailed delivery plan. [Mission reachability](mission-reachability.md)

The refresh keeps the original evidence and the 13-node graph's identifiers and prerequisite edges. [Dependency graph](graph.json) · [Evidence manifest](evidence/manifest.json)
