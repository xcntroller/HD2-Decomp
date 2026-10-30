# Shared native screen foundation closure

2026-10-30: the plan's screen creation/binding/lifetime/stack/dispatch entry is
complete for the shared native contract. This is not full original-class ABI or
whole-menu completion. Unused service branches are explicitly source-only.

## Live construction and interaction

Main and named Exit confirmation use the recovered registered-screen factory:
preloaded model service, native allocation, copied definition binding, mutable
root initialization, owning registration, temporary-definition teardown and
direct initial selection. Product definitions, strings, children, focus, frame
and message endpoints remain stable for their required lifetimes.

Slot zero has its own original model and singleton owner. It uses the separate
recovered construction helper, installs before binding, registers after success
and skips root initialization. Requests, button visibility/caption changes,
queued stacking, observer replies, keyboard activation and Escape are connected.
Its F9 trigger remains diagnostic; no original application requester is enabled.

The user desktop-confirmed these paths and the subsequent factory migration.
The latest log recorded 732 frames, keyboard activation and clean teardown.
The working slice is committed as `6782cf0`.

Host bindings use original screen ID families 0/1/4. The named native owner has
construction-order indices 0/1; these are not original registry indices 1/4.
The general helper supports callback-driven definition/list changes and load/
binding failures. Concrete builders for unavailable screens remain dependencies
of their corresponding flows; unsupported builders are never successful stubs.

## General dispatch and service boundary

The real queue preserves deferred/reply priority, targeted delivery, regular/
modal/slot-zero stack operations and direct startup selection. Application
routing preserves all five callback-service priorities and current-screen back
rules. Screen-specific callback bodies, profile cleanup, credits and resume
implementation remain explicit application services.

The remaining common branches now run through `common_event_services.h`:

| Events | Recovered shared behavior |
| --- | --- |
| `01000068` | Find first resource identity; release frame through service; recheck size and erase saved index after callback |
| `010000a1` | Gate current screen/model and argument tag; save model and receiver before repick; deliver target to saved receiver, or leave with zero |
| `02000007/08` | Hide stacked screens in LIFO order, retain previous-screen pointer, then request state 3/2 |
| `02000009/68` | Direct resource reset before requesting state 10; standalone reset service |
| `0200007a/7b` | Nonzero/unique notification registration and first-match removal |
| `020000bf/c0` | Previous-owner release gate; install before acquire; reread owner after release; matched release clears capture/mode |
| `020000c2` | Normalize motion-only; application receives its inverse; input receives mode reread after application callback |

The instruction check corrected two decompiler omissions: `007dc460` receives
the found resource frame before vector erasure, and `006dcad0` receives an
inverted Boolean in CL. It is a byte setter, not a parameterless refresh call.
These details are guarded in the [common-service map](menu-common-services-map.json).

Resource tokens replace original pointer identity without dereference. Capture
events carry typed retained native adapters, never reinterpreted old 32-bit
addresses. Native null acquisition throws; original code would dereference it.
Unknown events retain the host diagnostic instead of original silent ignore.
Original segmented containers, object layouts and SEH remain separate ABI work.

The application state/resource release/reset/input-mode/model-picker bodies
remain explicit backend services. The bounded host throws if an unconnected
service is invoked. New common events are not emitted by the current live flows,
so no additional desktop interaction is enabled by this source-only slice.
Actual resource caching, device/capture integration and each screen's application
bodies retain their existing open plan entries. No backend implementation is
claimed merely because the native dispatch contract exists.

## Validation

Temporary ASan/UBSan audit exercised the real queue with fake application/engine
services: null/duplicate lists, saved-index erasure after resource mutation,
LIFO transitions and previous-pointer retention, direct reset before pending
work, capture refusal/replacement/reentry, null release, retained event-owner
lifetime, inverse mode and callback-induced mode changes, missing-model gates,
and focus delivered to the saved receiver after repick changes the stack.
The audit also compiled strictly for Windows i386. Temporary artifacts were
removed; no permanent tests or agent-side Proton run were added.

Strict i386 host build, existing native suite, 158 Python checks, exact binary
fingerprint/evidence validation and frozen function accounting pass. The new
map has 23 anchors and 11 direct calls. Existing partial 0066c950 coverage expands;
counts remain 104 partial, zero complete, frozen 18,783 functions.
