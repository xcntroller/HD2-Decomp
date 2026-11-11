# P22-UI implementation — 2026-11-11

**OPEN. This is an implemented, verified presentation checkpoint, not source
closure of P22-UI.** Work is isolated on `feature/p22-ui`, initially clean at
`799a79305c70bc8de1860ac7e438bba25a71a8ac`. Both local master and fetched
origin/master had that exact SHA on initial and subsequent inspection. No
reference branch or other worktree was changed; no merge, rebase or push.
Research requirements were read through `git show research/p22:...` at
`ca3b314`; implementation was inspected directly at the master baseline.

## Implemented source and actual owners

`host/game_ui_presentation.h` supplies concrete callable endpoints over the
existing `world_loading::World`, checked `Owners::dependencies()`, SessionOwner,
HUD scene, mission camera, subtitles, GEffects and Startup process registry.
Constructor checks parent factory, Startup, graphics device, live SessionOwner,
parent Scene and request identity. The small shared `SessionOwner::backend()`
and backend identity accessors are necessary to reject foreign factories/devices
without duplicating owners. It creates no second World,
Players roster, Human, camera, HUD panel, font, SOUND or Item owner. It rejects
calls after `Owners::close()`; like the existing frame facades it must itself be
retired before the World allocation dies. World exposes its existing HUD panels
through one checked accessor, rather than constructing replacement panels.

- `0049B9B0` is HUD command-route **render prepass**, with mission-camera
  suppression. `0049B9F0` is the HUD **render orchestration**, including the
  original repeated map/info draws. Existing actual panel draw receivers are
  reused. Positive `004B6720` command geometry still rejects explicitly.
- `004909D0` is FPV **activation**, not the frame tick. Existing activation,
  model/hands/table/playback and current-player bindings are retained.
  New `00491E30` reads `005AC360`→`00789C80` from the actual checked per-Human DD0
  opacity owner; it preserves mode2, activated, player and model gates, caches
  changed opacity and calls the same GEffects virtual54/model-opacity receiver.
  This is NOT the complete `00491660` FPV frame.
- `004B01E0` injury countdown handles signed wrap, zero, indefinite -1 and
  expiration. `004AFDB0` refreshes a visible carnage caption from the actual
  Scene2 selected-Human count. `004AF470` is a genuine RET; no credit claimed.
- `004AF980` reads actual ExplosivePlace state94/countdownC8, admits state1 with
  signed positive countdown, retains original reverse-registry insertion order
  for equal timers, sorts ascending and updates ten native slots/captions.
  The host scans current identity slots and compares base addresses BEFORE
  dereferencing a borrowed actor or applying RTTI. Full viewport hides all ten
  only when slot0 was visible, matching the original branch. No explosive
  detonation or countdown producer is invented.
- Mission text uses the existing `0048F2D0` tick and `0048F390` draw on the same
  World subtitle owner and SessionOwner text/font. Subtitles obey the live
  selected-profile preference; Graph0 restores after a draw exception.
- `GameUiSceneCallbacks` instantiates the real `006C9260` dispatch policy.
  Stage1: HUD prepass, required P22-RENDER pre-scene receiver, optional debug.
  Stage2: HUD or full viewport, mission text, genuine `00724CB0` RET, session
  fade, real GEffects `00486C10` after-scene fade, process overlay. Status14
  returns before any receiver. Near/far viewport words remain intact.
  P22-RENDER must supply `GameUiRenderStages`; no successful empty implementation
  exists. No scene render, volume visibility or procedural renderer is added.
- The original process bytes0086922D/2C and0098AF98 now have one mutable Startup
  owner shared with application/UI getters. Their cold zeros are original BSS
  state, not forced gates in the adapter.

## Process overlay lifetime

Startup owns **one** `GameFrameTextOverlayProcess`: backend plus existing
`FrameTextOverlay`. The normal launcher initializes it after device/resource
setup, at `006DBFC0`'s final `006E8590` phase, regardless of registry population.
`editfont.bmp` and `tables\\fnt_matrix.bin` remain independent of menu/HUD fonts.
The existing glyph batching, reverse page/row/character order, visibility and raw
state restoration are reused. Host state/texture failures now propagate, and a
texture returned alongside a failing load result is released.

Registered source page owners are available through `registry.make_page()`.
Their destructor calls original `006E83B0` unregister **before** page rows die;
that operation deliberately clears all registered pages. If process shutdown
comes first, registry close clears/removes the still-live pages, and a weak
lifetime guard prevents later page-owner destruction from calling a dead
registry. Existing raw register/unregister remains a borrowed-page API: raw page
owners must explicitly unregister before destruction.

Retirement order: application/UI callback borrower → session FPV/HUD/Human/VM
ordered finish → World graph → startup data/modules → process overlay pages/
wrappers and independent texture (`006E85A0`) → device/driver close. Startup's
failure cleanup also retires the overlay before graphics. It must not outlive
the device. Normal launcher initialization is bound; standalone diagnostic
callers must explicitly call `initialize_frame_text_overlay(driver,graphics)`
if they use the process overlay. No loading/activation is enabled.

## Verification and limits

[Machine receipt](checkpoints/p22-ui-presentation-20261111.json). Temporary native
fixtures exercise actual recovered text, subtitle, timer, FPV-opacity and overlay
bodies with explicit engine/font/graphics endpoint doubles. One hundred repeated
lifetimes include indefinite/expired timers, positive captions, invalid borrower
rejection, full-viewport hiding, positive glyph drawing, draw failure restoration,
optional font failure, registered page retirement in both lifetime orders,
subtitle advancement and preference suppression, FPV mode/player/model gates,
changed/unchanged opacity, and invalid Human lifetime rejection. Callback policy
checks cover stages1/2, status14, full viewport preserving near/far, fade order
and early exception propagation. ASan/UBSan/LSan pass outside the ptrace sandbox.

Strict i386 compilation instantiates the **actual concrete host adapter**,
callback dispatcher, real World types and all newly exposed methods. The only
external callback arguments are the explicitly required P22-RENDER receiver.
No missing HUD/FPV method is substituted with a compile-only declaration.
Application launcher links. No original binary, DLL, Windows fixture or Proton
was run; no rendered game frame or desktop acceptance is claimed.

## Remaining work required before source closure

These are implementation gaps, **not a claimed external blocker**:

1. Full ordinary `0049BD50` still throws in `HudSceneLoading::update_loading`.
   Its positive workers include0049D6E0 selection/camera publication,
   004AA840 crosshair spread,004A9530 hands,004AAD30 status,004B3EC0 soldiers,
   00494190 compass,004B6420 command updates,004A4150 inventory and reached
   movie/map/progress/objective branches. New timer leaves are callable but are
   not falsely presented as that complete update receiver.
2. Full FPV `00491660`, animation completion00491C60 and their reached
   hide/switch/reload/item callbacks remain. Opacity00491E30 alone cannot be
   installed as `advance_first_person`. Preserve camera/update/draw ordering.
3. `004B6720` positive command geometry and reached positive panel/input/editing
   workers must be implemented against the same owners. Do not move these
   UI-owned gaps to Agent #1.
4. The concrete callbacks are callable but are not installed into the parent
   GameApplicationState, whose loading and frame callbacks remain disabled.
   P22-FRAME must schedule the completed UI update/FPV leaves in0069B230 order;
   P22-OWN must bind application activation, callback forwarding and rollback.
5. P22-HUM supplies real combat/weapon/action producers and existing Human
   lifetimes; UI must consume their actual interfaces, without implementing
   B0/DD0 simulation. P22-RENDER supplies pre-scene00486BF0/00482260/00482E50
   and debug geometry. Actual mission populations and desktop observations
   remain unknown.

Original evidence: client SHA256
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
Read-only original exports and JSON references retained under ignored
`analysis/work/p22-ui-20261111/`. Earlier baseline exports were read from the
main project's `analysis/work/mission-world-20261103/`, never modified.
Two exports rejected absent006E7B80/006E8160; their logs are retained and those
addresses receive no body credit. Timer radix helper006E4790 confirms stable
forward emission, used by006E4910. Four new frozen partial entries only; no
complete promotions or denominator changes. The user subsequently authorized committing this verified partial checkpoint
and continuing; P22-UI is still not source-closed.
