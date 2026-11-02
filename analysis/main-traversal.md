# Main traversal — 2026-11-02

**User desktop-accepted2026-11-02, including the four Options corrections.**
The initial audit changed no production source. This is the next section 3
checkbox, `main.traversal`, after the accepted dialog-caller milestone `90bc87b`.

## Completion boundary

Audit all nine Main bindings, available front-end destinations, Back/return
semantics and explicit unavailable boundaries. Dependencies are already accepted:
[Main labels](main-menu-state.md#desktop-acceptance),
[Cinematics](cinematics-flow.md), [Options](options-flow.md),
[Profiles](profiles-flow.md#desktop-acceptance),
[reachable multiplayer](multiplayer-pipeline.md#desktop-acceptance--2026-11-01)
and [Credits](credits-flow.md). Existing launch acceptance covers Main/Exit.

Single-player completion, loadout screen verification, game entry and real
save/resume receivers stay in Phase 2. Server-ready UI and networking acceptance
stay in Phase 3. This checkbox does not require completing those receivers or
reopening their confirmed research. Section 4 retains the global fidelity audit;
section 5 consolidates final Phase 1 acceptance, reference audit and packaging.

## Evidence and route matrix

Original client SHA256:
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
Use the confirmed `client` section of [menu-ui-map.json](menu-ui-map.json),
[application routing](menu-application-routing-map.json) and
[screen lifecycle/stack](menu-screen-map.json). The nine activation bindings are
inside `00623DD0`; the table gives their exact instruction sites. Tagged
`060010xx` records emit `020000xx` through `0065CF50`.

| Main control/action | Binding site | Activation record / argument | Current destination and boundary |
| --- | --- | --- | --- |
| `04800000` Resume | `0062446B` | `06001075 / 0` | Excluded from input; game-save receiver deferred |
| `04900000` Create/Change Profile | `0062455F` | `06001001 / 2` | Main registry slot 2; Back cleans up profile edits and returns Main |
| `04A00000` Single Player | `00624653` | `06001001 / 3` | Main registry slot 3; existing bounded team checkpoint accepted; further completion/verification Phase 2 |
| `04B00000` Multiplayer LAN | `00624747` | `06001099 / 0` | LAN setup then browser slot 8; Back cancels discovery, saves settings and returns Main |
| `04C00000` Multiplayer Internet | `0062483B` | `0600109A / 0` | Internet setup then browser slot 8; explicit GameSpy-unavailable error, saved-server/Add Server access |
| `04D00000` Cinematics | `0062492F` | `06001001 / 7` | Slot 7; retained playback returns to its list, Back returns Main |
| `04E00000` Options | `00624A23` | `06001071 / 0` | Settings initialization then slot 6; Back validates bindings, saves/applies before returning Main |
| `04F00000` Credits | `00624B17` | `0600103A / 0` | Controller status 12; retained Main returns through status 13 |
| `05000000` Exit | `00624C0B` | `06001003 / 04000000` | Named slot 4; No/Escape cancels, Yes releases the deferred status-4 request |

`00626B20` tries profile `006211E0`, multiplayer `006333C0`, mission `00638D30`,
Options `0061BEA0` and connection `006322C0` in that order, stopping on handled.
Its generic Back cleans up profiles before queuing a pop. Original Resume is a
real save/controller operation, not a successful no-op; no receiver was added.
The existing native mission handler reports an unavailable-save dialog for a
direct Resume request, while ordinary Main input cannot activate that action.

`0066C950` handles regular pushes, modal pushes and root Back. Ordinary push hides
Main and releases focus; returning activates it with target zero, leaving no
focused control until input selects one. Modal Exit leaves Main visible and its
existing focus intact; activation `00668400` skips an already-visible model, so
No/Escape preserves that focus. Root Escape opens the same named confirmation.
Negative release/discard precedes pop; affirmative release queues quit, and pop
restores Main before that quit is consumed. These existing contracts require no
source fix.

`00627520` still disables Resume without a parsed stored save caption and
disables the six profile-dependent actions without a current profile. The
separate production availability policy in `MenuScreenFactory`/`DefinedControls`
keeps Resume unbound even when original caption-based eligibility would enable
it. No save compatibility or loadability is inferred from that caption.

## Native verification

A temporary ASan/UBSan audit composes production Main/confirmation definitions,
`DefinedControls`, `DefinedScreen`, initialization, shared control state,
button click/activation/event emission, `MenuEventQueue`, `ScreenStack`, screen
lifecycle helpers, application routing, Main refresh and `OptionsFlow`.
It verifies:

- All nine exact IDs and activation event/argument pairs; eight bound routes
  through click, Enter, Space and keypad Enter (32 primary activations).
- Ordinary versus modal focus/visibility returns; nested screen Back; Exit
  Yes/No/Escape, root Escape, canceled deferred quit and queue teardown.
- Actual Options initialization/re-entry and save → graphics → binding apply
  before returning; selected/empty profile refresh and action eligibility.
- Missing/malformed/valid stored captions cannot make unavailable Resume
  interactive; five-handler ordering precedes the generic Resume service.

Visual/text backends and destination-specific profile/single-player/multiplayer/
video receivers are trace fixtures. In particular, multiplayer entry fixtures
record LAN/Internet choice and slot 8 without running discovery or a network
service; Credits records its state request without playing media. Generic
Resume dispatch records a service call, not a load result. Prior family
acceptance establishes those integrated destinations; this audit establishes
shared hub routing only. It is not a new desktop or engine execution.

Log: `analysis/work/main-traversal-20261102/sanitizers.log`. Temporary audit source
and binary removed after verification; no permanent tests added.
Original evidence validation passes: Main client map 348 instruction anchors/
33 vtable slots; application map 17 anchors/6 calls; screen map 24 anchors/
5 vtable slots/178 calls/142 resource anchors. Existing native suite and all
158 Python tests pass. The three strict i386 targets are up to date. Frozen
ledger validation passes, unchanged at **564 partial, zero complete / 18,783**;
this audit earns no function-body credit.

## Initial desktop checklist and reviewed build

Use the existing staged application with isolated copied profiles:
`analysis/work/startup-runtime/run/application_launch.exe`. Build/staged SHA256:
`aa5fa4d1ae3cadaa3cc350d24e8603845e1ad68b723ebfe34d86eea7f3e7d8d5`.
This is the unchanged previously accepted dialog-caller build. From the repository
root, the existing launcher is `python3 -B tools/run_application_launch.py`.
The agent has not executed it, the original game/DLLs, a Windows fixture or Proton.

1. From Main, open Profiles, Cinematics, Options, LAN and Internet, then return
   to Main. Dismiss any genuine service/unavailable error first. For Cinematics,
   also check return from an available movie to the list; for Options, include
   its binding-validation return when applicable. Check labels, visibility and
   usable mouse/keyboard focus after each return and on reopening.
2. Enter and Back out of the existing Single Player selection screen only.
   This checks the hub edge, not completion or acceptance of the deferred flow.
   Loadouts remain completely untested until accessible.
3. Enter/leave Credits and check Main restoration. Verify Resume stays
   unavailable. If checking an empty-profile state, use copied profiles only:
   Create Profile and Exit remain usable; profile-dependent actions do not.
4. Check Exit with pointer and focused Enter/Space: No and Escape keep Main
   usable, root Escape opens the confirmation, and Yes exits normally. Ordinary
   returns may need a new focus selection; Exit cancellation retains Main focus.

The user subsequently reported the result below. No separate execution of every
case above is inferred from that feedback.

## User review and Options corrections — 2026-11-02

The user reported four Options hover/click issues and “Could not find other
issues.” Record this as the scoped traversal feedback for the reviewed build,
with the four corrections still awaiting recheck. Do not repeat the entire
traversal checklist to validate these fixes.

[Correction evidence, implementation and native verification](options-flow.md#traversal-follow-up--hover-and-binding-caption-clicks-2026-11-02):
checkbox and graphics quality text now follows focus highlighting; binding
captions retain their normal color; clicking a caption activates its associated
binding value editor. Production adapters changed after the initial hub audit.
All three Windows targets, native regressions,158 tooling tests, original
evidence and ledger checks pass; the focused temporary ASan/UBSan audit passes.

Corrected staged SHA256:
`e419f29892794b3cf1eaa57e38ceddfead4672d823584ea881e4da959c4023ec`.
Only the focused four-issue desktop recheck remains for `main.traversal`.
No new function credit, desktop acceptance of these fixes, commit or push is claimed.

## Desktop acceptance — 2026-11-02

The user confirmed: “All good. Record the checkbox complete and continue”.
This closes `main.traversal` and the section3 supported-Main-actions checkbox,
including the four Options corrections, against executable SHA256
`e419f29892794b3cf1eaa57e38ceddfead4672d823584ea881e4da959c4023ec`.
Earlier pending-review statements above describe the intermediate builds.

Acceptance covers the available front-end hub and explicit unavailable
boundaries; it does not establish individual execution of every suggested case,
single-player completion, loadout testing, real save/game receivers or any
ready-server UI. Those Phase2/3 deferrals remain unchanged. The next unchecked
item is section4 layout/stacking/clipping/fonts/localization/resolution audit.
No commit or push was requested. Function credit remains564 partial, zero
complete /18,783.
