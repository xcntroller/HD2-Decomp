# Entire menu reconstruction plan

Updated: 2026-10-29. This is the menu delivery tracker. [HANDOFF.md](HANDOFF.md)
records how to resume; [RESEARCH.md](RESEARCH.md) records detailed evidence.

## Goal and completion criteria

Rebuild the base game's menu in our own readable, compiled client, initially
using the original LS3DF.dll and locally installed assets through Proton.
Complete means all reachable base-game menu screens and dialogs behave as the
original: rendering, navigation, controls, settings, profile/save data and
transitions to game/network services. Menu flows must reach a documented,
validated handoff to those services; rebuilding gameplay is a separate goal.
Sabre Squadron is outside the current scope.

Engine work follows menu dependencies: recover each necessary contract and
replace engine bodies in bounded slices. Replacing the whole LS3DF.dll is a
separate milestone; loading the original DLL does not count as reconstructing it.
Keep an explicit list of delegated calls so the eventual replacement is tractable.

## Current position

| Area | Status | Remaining work |
| --- | --- | --- |
| Independent i386 host, original engine window and teardown | Desktop-tested | Recover original application lifecycle beyond host scaffolding |
| Original main-menu artwork, cursor, bitmap text and hover pairs | Desktop-tested, bounded | Dynamic captions, localization and fidelity across settings |
| Exit and Yes/No confirmation | Desktop-tested | Generalize screen construction, stack and event routing |
| Shared pointer dispatch, picking, wheel and held-button transitions | Desktop-tested, bounded | Original object ABI, capture/device lifecycle and exact timing |
| Enter/Space on focused buttons | Desktop-tested | Keypad Enter audited, hardware test unavailable; device/character transport incomplete |
| Escape opens confirmation; Escape cancels it | Desktop-tested | Application interceptors and other screen-specific back rules |
| Previous/next focus traversal | Source reconstructed | Find original callers/key bindings and initial-focus policy before wiring |
| Original button sound and sound-driver lifecycle | Desktop-tested | Original preferences/resource cache and additional menu sounds |
| Shared control initialization and state messages | Base handler audited; tagged initialization/state desktop-tested for Exit/Yes/No | Derived/text initialization, full factory and reusable screens |
| Other main-menu actions/screens | Not functional yet | Inventory, reconstruct and enable one flow at a time |

Keyboard sound check: ordinary Exit/Yes/No keyboard activation is silent in the
recovered original path. Screen key handler 00668670 invokes activation +38;
button 006552F0 emits 04001000. Pointer dispatcher 0066B990 separately invokes
sound helper 0066B110 after an accepted click. Front-end keyboard pre-hook +44
is the no-op 0067EA80. Preserve this behavior. A separate options-volume callback
also invokes the sound helper, so this finding is not a claim about every widget.
This conclusion is from original binary analysis, not a new original-game run.

## 1. Finish the shared menu foundation — current priority

- [x] Bootstrap original graphics/3D engine, scene, models, font assets and cursor.
- [x] Recover bounded button events, focus/hover, picking and pointer sequencing.
- [x] Recover focused-button activation and queued Escape/back for the two live screens.
- [x] Restore original button.wav decoding, sound frame and accepted-click playback.
- [x] Check ordinary-button keyboard sound against original code.
- [ ] Reconstruct shared control state, initialization/event messages and eligibility.
- [ ] Replace hardcoded Exit/confirmation adapters with reusable controls and definitions.
- [ ] Recover screen creation, binding, lifetime, push/pop and general event dispatch.
- [ ] Recover keyboard preprocessing, character input, focus entry and navigation callers.
- [ ] Resolve exact queue/update ordering, timing, drag/capture and focus-loss behavior.
- [ ] Integrate original settings and resource-cache contracts instead of host defaults.

Base control handling (0065CD40) and eligibility (0064D9F0) have audited source.
The live Exit/Yes/No adapters now use original state defaults (0064DCC0) and
initialization-list sequencing (0065CF00); user desktop-confirmed.
The factory tail (0065D150) is reconstructed and audited, source-only.
Next: derived/text initialization and observer payloads, then full control
construction. Keep foundation delivery boxes open until integrated and tested.

## 2. Establish the complete screen and dependency inventory

- [ ] Enumerate every definition, derived/dynamic screen and shared dialog reachable
  from startup, the main menu, and the in-game menu; cross-check definitions,
  event routing, resource keys and callers. Identify which in-game flows require
  a running game and provide a menu-focused way to validate their UI contracts.
- [ ] For each screen record original identifier, model/assets, control kinds,
  captions, initialization, transitions, data dependencies and return behavior.
- [ ] Resolve duplicate/placeholder definitions before treating them as separate screens.
- [ ] Expand the table below into individual screen checklists as evidence is recovered.

Initial families below are work packages, not a claim of a complete screen list.
00623DD0 directly establishes main_menu, change_profile, singleplayer, confirm,
single_mission_2, options, multiplayer_menu, create_server, password,
server_filters, gameinfo, MP Connect console and Add Server definitions.
Additional screens/content are supplied elsewhere. Main-menu buttons also expose
resume, cinematics and credits; LAN and Internet have separate entry actions.

| Work package | Work required before completion | Current status |
| --- | --- | --- |
| Main menu | All nine original actions, live profile/last-save/date/version labels, eligibility | Exit only functional |
| Shared dialogs | Yes/No/OK, prompts, text/password entry, errors, cancellation and layering | Quit Yes/No only |
| Credits and cinematics | Recover entry/controller paths, scrolling/list/playback and return behavior | Not started |
| Options | Enumerate every page; graphics, audio, controls and other original settings; apply/cancel/persistence | Definition located |
| Profiles | List/select/create/rename/delete, validation and persistence | Definition located |
| Resume, saves and loading | Original metadata, previews, selection, errors and game-state handoff; save dialogs where reachable | Not started |
| Single player | Campaign/lone-wolf/carnage and mission-selection flows; recover setup, squad/equipment and briefing screens where reachable | Initial definition located |
| Multiplayer | LAN/Internet browser, filters, add server, info/password, create/connect, lobby/setup and disconnection/errors | Several definitions located |
| In-game menu and return paths | Inventory pause/options/save/load/quit variants and reconcile reused screens | Not inventoried |

## 3. Deliver complete flows in dependency order

- [ ] First additional screen: choose credits or another low-dependency screen after
  tracing its actual controller/assets; complete entry, interaction and return.
- [ ] Options: reconstruct shared sliders, toggles, lists, tabs and binding editors
  as their original use requires them. Apply original settings semantics.
- [ ] Profiles and saves: reconstruct editable text, scrolling lists and dialogs;
  use isolated writable copies for persistence validation.
- [ ] Single-player setup: reconstruct every reachable screen through the actual
  launch request, with a documented game-controller boundary.
- [ ] Multiplayer: reconstruct browser/create/connect/lobby UI and service contracts;
  distinguish working UI, local/network validation, and unavailable legacy services.
- [ ] Complete in-game menu variants and transitions back into the front end.

For each flow: recover evidence → write readable source → build and audit →
enable the action → desktop-test entry/action/back/error paths → update this
tracker → commit the working slice. Split large families into concrete screens
before marking them complete. Do not enable dead buttons or report a placeholder
service response as a completed original flow.

## 4. Finish fidelity and replace remaining scaffolding

- [ ] Verify layout, stacking, clipping, fonts/localization and supported resolutions.
- [ ] Verify pointer/keyboard rules, repeat, held transitions, disabled/hidden controls,
  focus changes, wheel, drag and modal cancellation against recovered behavior.
- [ ] Verify sound/music, preferences, failure paths and deterministic cleanup.
- [ ] Audit every host approximation and delegated engine/service call; record or
  reconstruct remaining menu dependencies, with source and binary evidence.
- [ ] Run a full menu traversal checklist after individual screens pass; record
  unavailable environments/services explicitly and leave dependent items open.

## Evidence and validation rules

- Update this file after each slice, with the screen/behavior and its validation
  state. A checked delivery item means integrated and desktop-tested unless it
  explicitly says source-only. Research and partial functions are not completion.
- Fingerprint and anchor original-client/engine evidence. Keep proprietary assets,
  binaries, generated pseudocode and logs local/ignored; preserve original game files.
- Build the strict i386 host, run relevant existing checks and temporary focused
  audits as needed. No new unit tests under the current project agreement.
- Commit interactive changes only after the user confirms desktop behavior works.
  No agent-side Proton launches; no pushes without explicit user instruction.
- Track function reconstruction separately in [README.md](README.md) and
  [analysis/function-status.json](analysis/function-status.json). Its frozen
  18,783-function denominator is not a percentage of menu delivery.
