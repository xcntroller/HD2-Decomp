# Shared application controller reconstruction

The original opening playback is user desktop-confirmed. The log records four
successful IShow opens, movie-to-menu entry, working Exit confirmation and clean
3D/sound/graphics teardown after 1,259 submitted frames. The new controller
source below is independently audited and **not connected to that live path**.

[`application_controller.h`](../recovered/frontend/application_controller.h)
recovers shared controller behavior from the fingerprinted base client:

| Original entry | Supported source behavior |
| --- | --- |
| 006C0820 | Startup/reinitialize gates, prerequisite order, mouse request and two-rand seed |
| 006C0CA0 | Alternate-start check, normal video-state installation and Logo1 start |
| 006C0EF0 | Null-descriptor shutdown/release, front-end construction, installation and initialization |
| 006C20F0 | Delay expiry, phase/status/suspend gates, input/update/post-update order, normal video completion |
| 006C3100 | Wrapping accumulated time, one 100 ms tick, status/suspend gating and finalization |
| 006C3190 | Render eligibility through fresh status calls; suspension adds no render gate |
| 006C31D0 | Both input buffer queries, forwarding only when both succeed |
| 006C72B0 | Non-consuming selected-buffer view; low mode bit, untouched output on empty |
| 006C20C0 | Flag-gated frame service call before phase reset; return supplied continuation byte |
| 006C30C0 | Distinct flag-gated fixed-frame service call before phase reset |

The state status word, type and video sequence index remain separate. Status
queries are repeated through the current state after callbacks. A state can
change during input or update; the following call must use the fresh current
state. Shutdown can also replace the active owner; null-descriptor replacement
releases the fresh owner after that callback. The front end is installed before
initialize(nullptr), and this branch ignores the initializer's result.

The input getter returns a pointer to the vector's beginning and its count,
not an individual key or character. It does not pop a record. The dispatcher
queries the second buffer even if the first query fails. This requires an
actual reconstructed input-buffer producer before live integration.

The fixed update subtracts 100 once, even if several ticks accumulated. Pause
consumes that tick without delivering it. The early below-threshold return
does not change phase. Addition/subtraction wraps in 32 bits, followed by signed
comparisons. This is a controller rule, not a game-wide simulation-rate claim.

Startup prerequisites 006B65D0/006B6620, alternate command-line handling,
mouse/platform services, state constructors, delay callback and frame service
bodies remain required supplied operations. They are not implemented as no-op
successes. Other descriptor/game/credits transitions and video indices 0/5/6
throw explicit unsupported-operation errors in this bounded source. Unknown
video indices outside the original switch retain its fallthrough behavior.

Native ownership, vectors, method calls and exceptions replace original class,
vtable, SEH and allocator ABI. Clearing a released native owner also removes
the original dangling slot during allocation; no use of that stale original
pointer is emulated. Empty-state/failed-allocation paths throw instead of
reproducing original invalid dereferences. The native owner does not claim
006C0920's global-table/input-device cleanup; concrete state destructors still
need their own resource lifetime. This is partial original control flow, with
ten PARTIAL entries and zero new complete-function credit.

Temporary ASan/UBSan and Windows i386 audits passed: startup rejection/failure,
alternate-start return handling, reinitialization, four-video transition trace,
failed initializer result, installation-before-initialization, callback-induced
state changes during shutdown/input/update, both buffer queries, render gates,
paused/large/wrapping tick accumulation, delay expiry and finalizer flags.
The [byte map](application-controller-map.json) checks original instructions,
state vtables and direct calls; it is static evidence, not runtime equivalence.
Temporary audits are removed after validation. No permanent test or new live
interaction is added. Do not require a repeat desktop test for this source slice.

Next integration dependency: concrete movie/menu application-state owners and
the original startup prerequisites/input producer. Recover those bodies before
replacing the confirmed host composition with this controller. Full WinMain,
setup orchestration, movie subtitles/music and original settings remain open.
