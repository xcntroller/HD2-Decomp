# P22-UI implementation — 2026-11-11

**SOURCE-CLOSED, 2026-11-12.** [Final closure contract](p22-ui-source-closure.md),
[delivery checklist](p22-ui-plan.md),
[receipt](checkpoints/p22-ui-source-closure-20261112.json).
This document preserves checkpoint history; earlier OPEN/next-work statements
are historical and superseded by the final contract. UI source closure includes
all UI-owned receivers and supported positive behavior. Human/Actor/packet data
producers and FRAME/RENDER/OWN execution remain explicit external requirements,
consistent with the user's final completion boundary. No populated Windows
World, rendered game frame or desktop acceptance is claimed.

Isolated `feature/p22-ui`. Initial master
`799a79305c70bc8de1860ac7e438bba25a71a8ac`; final read-only master
`05dbdee51cc366476317011b91b1a1863041dcee`. Research requirements read through
read-only `research/p22`; actual committed implementation takes precedence.
No reference/worktree mutation, active merge/rebase, push or loading activation.

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
  reused. Positive `004B6720` command geometry is now bound in checkpoint
  `d01face`, detailed below.
- `004909D0` is FPV **activation**, not the frame tick. Existing activation,
  model/hands/table/playback and current-player bindings are retained.
  New `00491E30` reads `005AC360`→`00789C80` from the actual checked per-Human DD0
  opacity owner; it preserves mode2, activated, player and model gates, caches
  changed opacity and calls the same GEffects virtual54/model-opacity receiver.
  Ordinary `00491660` frame and completion are implemented in the later FPV
  checkpoint below; occupied actor producers remain explicit dependencies.
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
   Callable real-owner receivers now include crosshair, hands, status, soldiers,
   compass, progress, pointer, messages, injury/movies and explosive timers.
   Selection/camera publication0049D6E0, command004B6420,
   inventory004A4150 and reached modal/map/objective branches still need their actual UI receivers and common dispatch.
2. Ordinary FPV00491660/00491C60/00491E90 is implemented and connected in the
   later checkpoints below. Occupied actor seat/visibility/hands/progress
   receivers remain explicit producer dependencies; no successful substitute
   is installed. Populated AI orders and visibility-range surface metrics must
   come from their actual Human owners.
3. `004B6720` positive command geometry is implemented in the command
   presentation checkpoint below. Full command-frame/roll and reached modal/map/inventory
   receivers remain UI-owned work. Input editing00494D20, start/stop and
   scrolling are implemented below, but actual console/chat dispatch00720130/
   00720340 remains required through a typed receiver. Do not attribute UI-owned
   gaps to Agent #1 or count compile success as closure.
4. The concrete callbacks are callable but are not installed into the parent
   GameApplicationState, whose loading and frame callbacks remain disabled.
   P22-FRAME must schedule completed UI services in0069B230 order; P22-OWN
   must bind application activation, callback forwarding and rollback.
5. P22-HUM supplies real combat/weapon/action producers and Human lifetimes.
   P22-RENDER supplies pre-scene00486BF0/00482260/00482E50 and debug geometry.
   Actual mission populations and desktop observations remain unknown.

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

## HUD animation continuation — 2026-11-11

The prior presentation checkpoint was committed as `31b01f9` at the user's
request. The subsequent read-only fetch reports origin/master at
`31b01f9c0e60e9f20125d5d5f6c554df12c7d47f`; this agent did not push or merge.
[New receipt](checkpoints/p22-ui-hud-animation-20261111.json).

Actual `004ABE60` now advances the SAME pointer70 toward74. IMUL(elapsed,5)
wraps before signed FILD; multiplication/addition and comparisons stay extended
across the float store. NaN follows the native decreasing branch without a
fabricated clamp. The backend preserves DC padding, makes the existing quaternion
when needed and marks the native scale/matrix dirty flags. There is no model24.
The producer of pointer targets/selected artwork remains the existing HUD command
path; the updater does not invent a selected pointer or display it.

`004AD350/004AD380/004AD440` now use the seven existing injury IShow owners.
Selection stops the current movie, draws exactly once from the actual process
CRT, and borrows the chosen owner. A failed optional movie remains null after
the original initial visibility write. Positive show binds the first material's
texture, seeks0, plays, updates the visual and resets fade/timer. Hide stops and
clears the borrower with no random draw. Update stores wrapping elapsed, fades
in at4 per second to0.5, then out at0.5 per second; strict-negative opacity invokes
hide before decoding. Otherwise actual visualA8 precedes IShow4C(false). FILD and
extended comparisons are confirmed by retained original instruction listings.
Cleanup withdraws the current borrower before retiring stream owners, including
reentrant cleanup callbacks. No extra movie/font/model/texture owner is created.

The concrete UI adapter exposes these receivers over the actual existing HUD
panels and CRT. It still does not substitute them for the incomplete full HUD
update, nor install a new application loading path. Native sanitizer checks cover
positive selection/play/fade/decode ordering, optional movie failure, decoder
failure, repeated stop/rebind/destruction, reentrant retirement, pointer clamps,
NaN and signed multiplication wrap. Engine/video calls are explicit doubles.
Strict i386 compilation instantiates all new methods against actual World owners;
application linking passes. Four additional frozen client partials, zero complete
promotions. Full P22-UI remains OPEN.

## FPV animation completion — 2026-11-11

[Receipt](checkpoints/p22-ui-fpv-completion-20261111.json). Continued from
`18d1809`; latest read-only origin/master remains `31b01f9`. No reference-branch
changes, merge, rebase or push by this agent.

`00491C60` now has a concrete complete source receiver, implemented in
`first_person_animation_completion.h` and bound by the existing FPV view and
composition into `GameUiPresentation::complete_first_person_animation`.
It uses REFERENCES to actual FPV60/64/98 and the actual model/playback70/74/94;
there is no duplicate player, model, clip, animation state or inventory owner.
Mode2, activated3C and live clip gate all work. Capture the clip before querying
model time; subtract duration-time with DWORD wrap and compare SIGNED, STRICTLY
less than elapsed. Equality defers completion.

Animation9 resets zoom, then argument8 rebinds actual current Human, resets tracks,
zeros argument and plays8. Other arguments unbind weapon/magazine and publish -1
animation/argument. State4/animation2 resets tracks, refreshes the actual weapon,
stores bob=-0.8, then either sets state5/argument0 or uses actual Human B0 state
and unbinds a suppressed weapon. Zoomed animation11 queries actual action twice
before cursor3 publication. Zoomed arguments0/1 store animation0 and return.
Ordinary argument0 performs the two original holder queries; zero amount sets
argument1, then the actual existing playback receiver runs. Missing live model,
Human or vanished second holder rejects explicitly rather than succeeding.
Human lifetime is checked through the existing per-Human map before inline/item
queries. All existing binding, weapon caches, SOUND and camera services are reused.

Native ASan/UBSan/LSan checks cover every completion branch, exact callback order,
strict end boundary, signed subtraction wrap, zero/no weapon, invalid Human and
model borrowers, mode/activation/clip gates and100 repeats. The native fixture
uses explicit endpoint doubles; strict actual-host i386 compilation instantiates
all new methods with real World/FPV/Human/Items/cursor types and no missing-method
declarations. Application link passes. No original/Windows fixture was executed.
One frozen client partial is added, with no complete or desktop promotion.

Original `00491E90` was also traced but is NOT newly implemented or credited:
mode2/3C, secondary59 false, model and Human gate; dead Human hides a visible
model and updates it. Live order is motion00491660 → completion00491C60 →
blend00491440 → opacity00491E30 → flash retirement → model20/24 when a current
clip remains. It runs even when58 first-person visibility is false. Flash time0
hides/updates flash, writes -1, then still subtracts elapsed and clamps negative
to0. Preserve this surprising original sequence rather than changing it into a
new timer policy. Original assembly and decompilation are retained under ignored
`analysis/work/p22-ui-20261111/`.

Next is the full00491660 motion body and actual00491E90 wrapper, then the remaining
HUD/command/input receivers. Completion must NOT be passed off as the complete
`advance_first_person` callback. P22-UI remains OPEN; FRAME/RENDER/OWN invocation
and desktop observations remain separate.

## FPV frame and HUD status sweep — 2026-11-11

[Receipt](checkpoints/p22-ui-fpv-frame-20261111.json). Documentation was deferred
until source implementation and focused verification, as requested. **P22-UI
is still OPEN**: the sweep did not finish the large remaining HUD/command/input
receivers, and no external blocker is claimed.

`00491660` now consumes actual Human.activation.vector1b0, model world rotation,
active camera position/direction and SAME FPV9C phase/64 bob/68 rate. The phase is
a FLOAT, correcting its former DWORD guard. All original state1/2/3/5 branches
are present, including state5's nonintuitive <= comparison, wrapped elapsed*3/4,
kind9 holster selection with SAME session28, real model binding and animation2,
direction normalization and mission-camera interpolation. Camera00465E80/00465E30
read actual240 camera120/12C; these short absent-inventory getters earn no credit.
No camera simulation, weapon firing or Human B0/DD0 producer is introduced.

`00491E90` now exposes actual `GameUiPresentation::advance_first_person` through
the existing FPV composition: mode2/3C/secondary59/model/Human gates, signed-health
dead hide, then motion→completion→blend→opacity→flash→model20/24. Invisible58 still
updates; live58 only restores visibility when needed. Zero flash time writes -1,
hides/updates, then still subtracts elapsed and clamps to0, exactly as the original
instructions show. No duplicate scene/model/clip/camera/clock owner exists.

`004AAD30` now refreshes the SAME HUD status text and frames: countdown clearing,
positive mm:ss caption, current-Human B0 flags50&7 and5C, original next-show THEN
previous-hide, cumulative movement icons and fresh state query after callbacks.
Nonoffline style/team publication remains explicit required work. This is a
callable leaf over real panels, not a replacement for full0049BD50.

Native ASan/UBSan/LSan checks use explicit engine/Human/item/camera endpoint doubles
with100 repetitions: every bob state, positive kind9 switch, no holster, active
camera error, retired Human error, position/direction ordering, interpolation,
full frame ordering while invisible, dead hide, secondary/mode suppression,
repeated zero-flash retirement, status icon transitions and countdown expiry.
Strict actual-host i386 instantiation and application link pass. New temporary
audits are fingerprinted and removed; no permanent tests. Standard-library
long-double sine follows FSIN mathematics without claiming bit-identical hardware
argument reduction. Invalid FTOL phase or unwritten camera output is rejected.
No original code/Windows fixture ran and no desktop frame is claimed.

Remaining UI-owned work is full0049BD50 and its soldier/hands/selection/crosshair/
compass/inventory/map/objective workers,004B6720 command geometry and reached
editing/control receivers. Prior FPV attached-actor branches still require their
real seat callbacks. The actual adapter is callable, but its containing session
scheduling/application callback installation remains FRAME/OWN; pre-scene volume/
procedural rendering remains RENDER. Do not mark the blocker closed from these
successful native checks or from the completed FPV frame path alone.

## Crosshair/Human publication continuation — 2026-11-11

[Receipt](checkpoints/p22-ui-crosshair-20261111.json). Continued from25b1ca7;
latest read-only origin/master remains31b01f9. No merge/rebase/push.

004AA840 now consumes actual005A8100 and updates the existing cross_L/R/T/B
artwork. Root visibility precedes current-Human query. No current Human returns
without moving parts. Target is extended spread*FLOAT.025+FLOAT.01; signed elapsed
steps by FLOAT.001*FLOAT.05, stores the FLOAT then compares the extended result
before clamping. Each existing part receives its native signed X/Y lane; flag80
protects position but does NOT skip model24. Same offset14 is now correctly typed
as FLOAT; its legacy raw getter preserves DWORD bits. No new artwork is allocated.

005A8100 reads the same hand holder, Items runtime and catalogue. No weapon or
kind6 yields native zero. Payload virtual10 returns zero for types1/2, native44
(serialized40) for type3 and native40 (serialized3C) for type4. Unknown families
remain explicit required receivers. The actual Player virtualEC00542B10 reads
B0 flags50 and HumanFC (stats_ec[4]): posture priority4/2/default, then movement
priority1000/2000/4000 when lowF8 is set, then(100-signed FC)*FLOAT.01. All seven
other recovered B0 tables dispatch EC to genuine zero00524DA0. This is a read-only
UI query in human_hud_loading.h; Agent #1's B0/DD0 implementations are untouched.
Short absent-inventory getters00542B10/00524DA0/007E0270/0310/0370 earn no credit.

The concrete adapter exposes advance_crosshair over actual World panels,
first_players and Items. Per-Human lifetime validation precedes holder access.
Native ASan/UBSan/LSan checks run100 repeated lifetimes with positive Player and
nonPlayer values, no weapon/kind6, malformed payload and retired Human failure,
visibility/no-Human gating, smoothing/clamping, signed artwork coordinates and
protected-frame update ordering. Graphics/font/material endpoints are doubles.
Strict actual-host i386 instantiation, application link and tracker checks pass.
Temporary fixtures are hashed and removed. No original/Windows code was run.

P22-UI remains OPEN. Full0049BD50 selection/hands/soldiers/compass/map/inventory/
objective processing,004B6720 command geometry and reached editing/control
receivers remain required. This receiver is callable and uses the real owners;
it is not falsely substituted for the full HUD update. FRAME/RENDER/OWN and
mission desktop observations remain separate.

## Hands, shared item icons and selector update — 2026-11-11

[Receipt](checkpoints/p22-ui-hands-selector-20261111.json). Source work and
verification preceded documentation. Continued from353f244; read-only fetched
origin/master remains31b01f9. No other worktree/reference branch edits, merge,
rebase or push. **P22-UI remains OPEN**, with no external blocker claimed.

004A9320/004A93E0 now cache actual catalogue DEFINITION pointers, not transient
inventory entries. Publish pointer before caption/icon work, preserve identity
short-circuit, use original single-space empty hand caption, and restore the
retained original texture on null. Optional failed icons leave the material
unchanged.004A9420 stores native current/reserve fields in original side-specific
order then publishes signed captions in the existing Text owner/style37/38.

004A9530 ordinary update consumes actual current Human storage: hand3/shoulder2,
real Item amount and existing005A6C50 ammunition counts, command6 shortcut1/1,
shoulder native+8 family==0 gate, then005A6D10 kind9 count. Original absent
005A2AC0/005A2AF0 wrappers prove their returns are ITEM ENTRIES, not holders:
shoulder+8 is RuntimeItem::family, not a holder offset. The holster counter scans
reverse, uses only each occupied stack's FRONT entry, admits unsigned cooldown
field14<=same session28 and kind9, adds stack size with DWORD wrap. No pack/hand/
shoulder or amount filtering is added. Mounted-seat98/B4/A0/9C remains explicit
required work; no successful substitute was supplied for an attached Human.

007DF650 now owns icons in the SAME existing RuntimeItemServices allocation
beside its model caches. It validates the selected catalogue identity, loads
resource1/native2C plus literal.bmp with null secondary/flags0, accepts EXACT
status0, and caches actual returned texture. Driver15/19 are saved BOOLEANIZED,
disabled and restored separately, including exception prefixes. Optional failure
retries on later cache access. A nonzero load returning a texture releases that
failed output safely. Names exceeding native128-byte storage reject. Icon28
releases after first-person20 and before primary30, preserving derived magazine
retirement first. There is no competing catalogue/font/graphics owner. Existing
selector population now receives this real shared icon service.

004ACE20 show/hide and004AD180 update bind to actual Players::selected_player,
with the existing per-Human lifetime check. Show publishes selected Human and
resets38 before focus/action tests. Positive population uses actual holders/
stacks/icons and existing model/text owners. Hide retracts Human before reverse
record destruction. Update preserves signed wrapping elapsed and2000/2001ms
boundary, dirty178 close/repopulate/layout, death/mismatched-selection hide,
and positive30 timer priority over34. Current Human remains a DISTINCT query
for amount captions. Input transfer/timer producers are still required, and no
new successful empty callback is introduced. UI adapter exposes these actual
hands/selector methods without enabling application loading.

Native ASan/UBSan/LSan checks cover100 repeated actual recovered panel/cache/
selector lifetimes with explicit engine/font/material endpoints: positive hand,
shoulder, ammo/reserve/counter, command6, cooldown, no item, family gate, mounted
and retired Human rejection, optional icon failure/retry, foreign catalogue
identity, graphics-state failure restoration, positive selector population,
layout, dirty rebuild, expiry equality/next tick, death/selection-change cleanup.
A fixture initially used the hands texture name for selector artwork; correcting
it to original i_frehnd.bmp resolved the expected missing-artwork rejection.
Strict i386 instantiates actual World methods with no missing-method declarations;
launcher links. No original binary/DLL/Proton/Windows fixture was run. Six new
frozen client partials, two existing rows extended; no complete promotions.

Remaining UI-owned boundary: full0049BD50 soldier/selection/compass/map/inventory/
objective workers,004B6720 command geometry and active00494D20 editing/control.
Attached hands and prior attached FPV seat services remain explicit. This is
implemented source progress, not closure or desktop game rendering.

## Soldier, compass and progress frame checkpoint

Continuation from0789bd3, still on `feature/p22-ui`. Latest read-only fetch and
inspection: origin/master31b01f9c0e60e9f20125d5d5f6c554df12c7d47f. Research
reference remainsca3b314. No reference branch/other worktree edit, merge,
rebase or push. **P22-UI remains OPEN**, because UI-owned receivers listed above
are unfinished; no external blocker is used as a closure excuse.

`HudSoldierLoading::update` implements offline004B3EC0 using its existing panel,
Human, Items, text and artwork. Rank/dirty-weapon updates precede the dead latch;
health/death transitions preserve the original selected/player/AI/ear hides,
bar/photo updates and range text. Live updates retain the50-percent health icon
threshold, .95 bar cap, raw stamina/disguise metrics, RGB cache, command/mind
icons, current/AI indicators, pending-order priority and strict squared
40-metre ear range. It does not manufacture combat, weapon or order data.
Human005A5FF0/005A62A0 queries preserve signed fields and FTOL low-DWORD behavior.
World adapter `advance_hud_soldiers` walks the SAME six panels in reverse and
checks visible panel Human borrowers against the actual Scene2 identity registry
before accessing their existing per-Human composition. Visibility range uses
existing005A59E0/Sight/DD0 services. Populated Active orders still reject through
the existing typed250 owner; original008109F8+134=0052F130 returns the250/254
list count. This checkpoint does not replace that producer with a fake list.

`HudCompassLoading::update` implements00494190,00494590,00494360,00494750,
00493C60 and00493860. Heading precedes sway, then original signed-time spring/
damping and angle clamps, cover/arrow virtual1C rotations, reverse16 sound
sectors and reverse roster markers. Sound alpha uses the OLD countdown; zero
hides on the next update. Markers use actual Human health/suppression/status,
fade and relative direction/position, preserve radius/Z and destroy their
retained frame before swap-pop. Frame updates have no new root-visibility gate.
Sway reuses process shared0089BB54 cadence, SAME CRT random source and engine
RandFloat; HUD binding resets48/4C but deliberately retains54.

Read-only005A8160/005AC410/0075E610 uses the existing DD0 physical bones0/1,
native810/81C. Original0075B280 initializes this array at810 with strideC;
these are distinct from geometry feet. It predicts model position with twice
actual Human velocity, reads model then bones0/1 and preserves the second-Z
FLOAT store before squared-distance comparison. No B0/DD0 frame implementation
was edited. Native instructions also confirm original compass QWORD constants
008104A8=.10000000149011612,008104B0=1.5,008104B8=2.5 and native00494750 uses
elapsed atstack18, rather than the decompiler's apparent sector-loop counter.

`advance_hud_progress` connects004AB110 to actual current Human and existing
17C/180/184 timer/caption producers005A9830, which already include Player action
progress. Bar ratio uses original signed FILD/FIDIV followed by one FLOAT store;
there is no intermediate float conversion or invented zero-denominator clamp.
It preserves root-show/bar-update/caption cache ordering and style42. Missing
current Human/action hides the panel. Registered attachment kinds4/20/21/34
require actual virtual98/C8 seat producers before falling back to Human fields;
other kinds follow the original fallback. Nonoffline remote progress remains
explicit. `advance_hud_messages` also exposes the existing ordinary message
advance service; it continues to reject active editing00494D20.

All new query facades live only for the call and borrow the SAME World/Players,
Human/Items, camera-independent frame services and Startup random/clock state.
No second persistent owner, font, texture, World or roster was allocated. Existing
HUD bind initializes artwork/text/ring; add_human supplies actual panel/marker
borrowers. Expired marker records release frames before removal. Existing HUD
close releases marker/death/text owners and discards borrowers before HUD scene
release. Retire the UI adapter before World owner close; actor membership checks
reject invalid borrowers before dereference. The existing module/VM/Human/Items
finish order and application activation remain unchanged.

[Verification receipt](checkpoints/p22-ui-soldier-compass-progress-20261111.json):
three separate native ASan/UBSan/LSan audits, each100 lifetime cycles; strict
actual-host i386 instantiation of soldier/compass/progress/message endpoints;
application link; frozen ledger check and27 existing tracker tests. Genuine
source positives exercised include health/death/dirty/rank/bar/photo/ear paths,
compass hidden-root update, sound fade/expiry, live/dead marker positions and
removal, shared sway cadence, DD0 proximity and actual-field progress/caption
publication. Tests include null/invalid borrowers, required producer rejection,
resource failure, repeat cleanup and the integer-ratio rounding discrepancy.
Native engine/query endpoints are explicit doubles. Populated Active orders,
real occupied seat progress, nonoffline progress and visibility-surface metrics
were not verified against runtime game populations. No original program/DLL,
Windows fixture or Proton was run, and no rendered desktop game frame is claimed.

Evidence exports and instruction/constant dumps are retained under ignored
`analysis/work/p22-ui-20261111/`; the receipt fingerprints important artifacts.
Only11 frozen client entries gain partial credit; existing005A62A0 is extended.
No complete promotion or frozen inventory change:34 complete /2398 partial
(2393 client,5 engine) of18783 identified functions.

## Required Human order-display boundary

Continuation afterfc75168 implements004AB2E0 in the SAME `HudAuxiliaryLoading::Info`
owner. Native field00 is now a typed Human address cache, rather than an unused
DWORD zero; no duplicated owner/list is created. `bind_info` initializes it and
containing `discard_borrowers` clears it. Identity change short-circuits the dirty
query, stores the Human before hiding infosel and querying orders, and unchanged
identity/false dirty returns before any frame mutation.

The body implements all original positive artwork branches: kinds1/2 use first
states10/11, kind3 uses the actual actions table parameter, kind4 state19, kinds6/7
states13/14. Unknown kinds retain the prior iteration's texture: native stack14
is zeroed only once before the loop. Zero skips material binding. Selected4A
sets1.1 scale, reads the icon world matrix, offsets Z by-1, then scales/positions/
shows/updates the SAME selection visual before material/show/update of the icon.
Count is queried again after EACH icon update; remaining slots hide forward.
Malformed counts/indices reject instead of reproducing native out-of-bounds use.
No order, actor, texture or engine pointer is retained beyond the existing Info
cache and containing HUD lifetime.

`GameUiPresentation::advance_hud_orders(GameUiHumanOrderSource&)` chooses the
actual current Human; offline command mode uses actual0045D1B0 PREVIOUS30 when
present, then falls back to current. This is distinct from0045D160 selected28.
Each changed/count/order read checks the SAME actual per-Human composition/B0
again. All frame/material methods are real existing GameSceneActors endpoints.

`GameUiHumanOrderSource` is explicitly REQUIRED from P22-HUM's real producer:
005A5ED0 changed dispatch (kind3 virtual130, Player BYTE525),005A5F00 count
(kind3 virtual134, otherwise0), and005A5F50 entry virtual138. `HudOrderDisplay`
contains only native0C kind/10 parameter/4A selected publication. UI consumes the
record by value during that call and owns no task/list. There is no default or
successful empty implementation. Populated Active250 orders remain unavailable
in the actual Human owner. Consequently this is a source-implemented positive
UI receiver with an explicit external producer contract, not a claim that real
populated order production or its scheduling is bound. No declaration-only UI
methods or Human simulation changes were added.

[Receipt](checkpoints/p22-ui-order-display-20261111.json): native ASan/UBSan/LSan
100 cycles exercises all artwork/selected/count/cache paths, callback-visible
count changes, ten-slot boundary, invalid producer/null/indices and teardown.
Strict i386 instantiates the concrete World adapter with its required Human
interface; application links. Engine and Human-order fixtures are doubles.
Only004AB2E0 receives partial ledger credit;005A5ED0/005A5F00/005A5F50 contracts
receive no new body credit. Frozen total18783 and34 complete remain unchanged;
partial2399=2394 client+5 engine. Latest read-only fetched origin/master remains
31b01f9; no merge/rebase/push, loading enablement or original/desktop execution.

**P22-UI stays OPEN.** Next UI-owned work is0049D6E0 selection/camera,
004B6420/004B6720 command frame/geometry,00494D20 active editing and reached
camera/map/inventory/objective workers, then full0049BD50 dispatch. The required
Human order producer is separate from those unfinished UI receivers.

## Input editing, snapshot overlay and target presentation

Continuation from477c5d1, isolated feature/p22-ui. Latest read-only fetch and
inspection still finds origin/master31b01f9c0e60e9f20125d5d5f6c554df12c7d47f.
No reference-branch/other-worktree source change, merge/rebase/push or application
loading enablement. **P22-UI stays OPEN.** The new research-index tool was read
at tooling/research-index02f804a through/tmp/hd2-research-index. Address lookups
identified existing source/evidence before new exports, including the already
recovered0064D5F0 encoder; its source was checked against verified origin/master
and reused. No uncommitted Human source is assumed available. The tool remains
in its separate checkout and its generated cache is not included in this branch.

`hud_message_edit.h` implements00494D20,00495190 and004961E0. It consumes the
existing HudMessageState, independently owned HUD message fonts and borrowed
IGraph input. Scan reads precede character reads; repeat compares signed wrapped
elapsed strictly greater than150/50 thresholds, checks the original low-byte
scan, and resets counters in source order. History uses the same message vector
count, including native unsigned count-minus-one behavior. Byte editing admits
255 bytes, whereas wide-unit append requires total<255, encodes each UTF16 unit
with the existing0064D5F0 and rejects surrogates without combining them. Tab and
backspace stop processing the already-read wide queue. Wide deletion preserves
original lead/count checks, with a defined guard against the native malformed
all-continuation underread. Unterminated buffers reject instead of overreading.

Scrolling00495190 uses the SAME font3 underscore metric, actual first/second
bounds selected by any nonzero field10, original height conversions and reverse
BYTE glyph traversal. Instruction and QWORD confirmation found008104C8 is
**negative DOUBLE-.4**, which is ADDED for a zero-aspect glyph; it differs from
the drawing worker's positive fallback. The implementation and focused test
expectations were corrected before the final passing audit. The returned byte
index is not incremented. No second layout, caret owner or shared menu font was
created. Wide/GDI glyph rendering remains outside the existing byte-font host;
wide input-unit editing is implemented and tested as source behavior.

Submission echoes yellow255/255/0 before command dispatch. Empty input echoes
one space008399C4 but submits the actual empty edit buffer. Say takes priority
over SayTeam; otherwise dispatch is console00720130. Buffers/scroll index and
conditional mode clear follow successful return, preserving state after a
required executor failure. `GameUiMessageCommands` is an explicit mandatory
console/chat executor interface, with no default success implementation. Console
parsing00720130/00720340, gameplay effects and network submission were not added
or declared completed. The message service is callable with a real executor;
its absence cannot close P22-UI's reached positive command boundary.

Full00496230 now increments caret, edits, rereads the repeat key for elapsed
addition, then ages records. The no-input overload retains an explicit active
input rejection. Input start resets caret, calls actual IGraphAC/1006F300 to
clear its32-DWORD queue, then stores the requested enabled BYTE. Stop clears
input and say/team/input flags without resetting unrelated counters. The small
`GameOwnerBackend::presentation_input` accessor is necessary to reuse its actual
module and Startup graphics without exposing Startup private state. It returns
the existing BorrowedGraphicsInput facade by value: no device initialization,
second poll, queued input owner or persistent keyboard was added.

Camera snapshot004AF4F0 is bound through the existing small panel. It subtracts
elapsed as DWORD and compares signed time. Below1000, its one-shot flag emits
SAME SOUND group1/entry2E/flags41 at the actual scene active camera's world
position; the flag clears only after the endpoint returns. Strict800<time<950
calls GEffects008101F0+88=004867F0 directly with black RGB and alpha1 through the
existing GameEffectsFadeRender/process quad, not a new fade owner or simulation.
At<=0 it zeros time and hides. The World adapter preserves the original optional
camera-root and visibility gate. Existing004AF600 show/hide semantics, including
retained timer/flag on hide and2000 on show, are unchanged.

`HudCrosshairLoading::publish_target` implements004A9F50's identity/request cache,
clear/store-before-query order, all actor-kind presentation branches, white/blue
material policy, offline hostile/dead captions, roster numbering (full name before fresh index query), Car reverse
first-live occupant and StaticWeapon occupant captions. It draws through the
existing TextState; root visibility gates material colour changes, while target
presence separately gates caption drawing. Actual World adapters check Scene2
membership before dereference and reuse Players roster/Sight relation, actual
Car occupants/geometry and the existing StaticWeapon loading seat owner. Normal
non-Human captions use their existing Actor name. Nonoffline name/rank/team and
Human174 typed-focus publications remain explicit required producers.

Original005A8320 is FULL profile0C (user14 first), distinct from existing LAST04
and SHORT10 helpers. Human virtual04 is005A9980, NOT a passive Actor name read:
roles1/4/5 refresh the existing Actor80 string from005A8320; other roles use1A8
localization or defaults1BEF/1BFD for roles2/3. The source helper publishes into
that SAME name owner. This newly inspected body is absent from the frozen
inventory and receives no credit. Format strings were confirmed as`%d. %s`,
`%s %s` and`%s %s - %d%%`. Target retirement uses the existing ActorBase identity
and actual HUD actor-removal callback to clear its cached borrower/text before
actor destruction; this is a defined source lifetime guard with no new native
function claim. The containing HUD also discards the cache before its scene.

`hud_crosshair_visibility.h` supplies0049D530, separate from spread and target
publication. It preserves180 clear/hide, command/camera suppression, actual
current Human/attachment, occupied-seat checks, original held-item cursor-payload
and kind6 gates, and signed Player mode1..3 suppression. Its concrete World
receiver uses existing MissionCamera, FPV, Players, Items and Startup state.
Occupied actor virtual8C/CC producers remain explicitly required. No fake weapon
mechanics, seat flags or always-visible predicate is supplied.

[Receipt](checkpoints/p22-ui-controls-targets-20261111.json): four separate native
ASan/UBSan/LSan100-cycle audits cover the editor, snapshot, target captions and
visibility policy. Positive tests exercise all UI actor-kind/colour/text branches,
roster/seat captions, threshold/query order, real profile/name helper, editor/font
state and failure/retirement paths. Native engine/input/font/actor/submit endpoints
are doubles. Strict i386 compiles all new concrete World methods without absent
UI method declarations; application link and frozen ledger/tracker checks pass.
No original executable/DLL/Windows fixture/Proton or desktop game frame was run.
Only7 present frozen entries gain partial credit;00496230 is extended. The
existing0064D5F0 encoder and delegated engine calls gain no credit; absent005A9980
is excluded. Frozen18783 and34 complete remain; partial2406=2401client+5engine.

Next is the still UI-owned0049D6E0 ray/roll/signals selection,004B6420/004B6720
command frame/geometry and reached map/inventory/objective workers, followed by
complete0049BD50 scheduling within the UI boundary. Console/chat execution and
nonoffline/seat/focus producers remain required as stated above. P22-FRAME
schedules these UI services; P22-RENDER supplies full scene/volume/procedural work;
P22-OWN installs activation/forwarding/rollback. None is enabled by this checkpoint.


## Command presentation, pointer controls and target producer — 2026-11-11

Continuation from `f586e28`; origin/master remains
`31b01f9c0e60e9f20125d5d5f6c554df12c7d47f` after a read-only fetch.
[Receipt](checkpoints/p22-ui-command-presentation-20261111.json).
**P22-UI remains OPEN.** This completes the listed presentation workers, not
full004B6420 command-frame scheduling, roll-menu population or0049BD50 dispatch.
Research index at tooling02f804a was used before fresh exports; existing original
command geometry and route-write evidence was reused, and ambiguous decompilation
was checked against instructions. No main/research source or other worktree edit.

`GameUiPresentation::command_before_scene` now supplies actual004B6720 through
SAME HudPanels/command/auxiliary owners. It obtains the actual0045EB30 temporary
selected-Human list and iterates it in reverse. Health<1 short-circuits the
005A6380 unavailable query; the selected box is black or red. An attached Human
checks actual Car/StaticWeapon seats, ascending with a fresh seat count after each
empty seat. Only the first nonnull occupant matters: another Human suppresses
this highlight; the selected Human redirects it to its attachment. No occupied
seat keeps the Human. Current18 and previous14 redirect to attachments; target1C
uses its actual actor directly. Green current004BB260, blue previous004BC300,
white target004BD3D0 and selected004BE670 geometry draw in that order after the
selected list. Pointer projection runs last when native5A7BB is set.

Each highlight checks actual model11C against WORLD scene11C: these are render
epochs, not identity/membership values. It then reads camera position, rereads
active camera for its forward matrix, normalizes, checks actor-facing direction,
obtains the existing actor virtualDC world corners and projects through actual
World Scene98. Screen min/max retain the original width/height/zero sentinels.
Minimum span is2.5% of width/height. Quarter lengths are computed before previous
inset or target outset of.5% of width/height. These command rectangles have no
0049E260 ten-pixel clamp. Selected boxes submit8 vertices/four lines; other styles
submit16/eight. FLOAT colour multiplication and current FISTP rounding include
alpha.5 (127 under downward rounding,128 nearest/upward). Graph44 bind-null
return is ignored as originally; a failed Graph88 draw propagates. Transformed
vertices preserve.5 Z/2 RHW; original unwritten padding has defined zero guards.

`004ABF50` uses the SAME pointer stored at command44 (HUD6B320). It is not another
inline pointer or new owner. World Scene98 projects its command20 position;
X divides by FLOAT-stored signed width and Y by signed height then exact
DOUBLE00810518=-.75. Optional rectangle78 is a typed borrowed eight-FLOAT record;
limits use offsets0/1/4/5 with.002 margins. No clipping occurs when null. XYZ is
published via the existing protected-position guard; frame24 still runs. Z/W
remain the existing pointer20 lanes. The rectangle borrower is discarded before
its owner can be destroyed, even when no overlay/target was populated.

`004ABAF0` now supports the full artwork selection table: unchanged-kind returns,
old frame hides, kind0C stores, -1 retains frame00, and positive kinds position,
show and update the actual artwork. `004ABB70/004AB990` read those same fields.
`004ABDD0` casts a +Z ray from pointer20 with Z+.5 through the HUD Scene04 FC,
flags04040000 and six trailing zero arguments; it returns the actual hit frameEC
tag. Original LS3DF vtable1009B798 confirms94→10047250/RET20,98→100476A0/RET16
andFC→10010C20/RET40; concrete endpoints check those targets. World projection
and HUD picking remain separate scenes. `004AB9A0` uses
FLOAT-stored signed width, exact DOUBLE00810508=-1.3333333333333333 and two
007E5E00 truncations. Extended precision is observable: default Y=-.375 with
height600 truncates to299. Invalid QWORD conversion retains the original zero
low-DWORD guard. `004BF230` strictly checks X(.01,.99), Y(-.61,-.01); boundaries
and NaNs reject.

`004B6280/004B61A0` bind actual hover tags to the eight existing command
TextStates/localizations. Reset order is regroup/signals/pose/movement/mind/
delete/options/camera, then the requested row unless sentinel8 or nested roll30.
`004B5C10` preserves movement10000/4000/2000/1000, mind400/200/100 and pose4/2/1
priorities, all-ones disable and zero-mask retention. Selected button updates
start SAME process0089BB7C click frame30 before normal/inverse-alternate
visibility. It is the existing HUD button.wav owner, not the menu sound or a
new SOUND emission. `004B5B90` updates regroup/wait01/wait02 without clicking;
order kinds4/6/7 disable them.

`004B5A20` first finds selected order4A via actual HUD-info Human and fresh
005A5F00 count queries (`004AB7E0`). A selected Human-owned order44 receives
missing defaults4000/200/4 and supplies masks1F000/700/7. With no selected order,
previous14 supplies SAME B0+5C and B0+50&700/7; no previous means all-ones. Button
updates precede the final process click. `GameUiHumanCommandSource` extends the
required Human-order interface with an actual mutable word44 reference consumed
in-call. No order vector, B0 owner, fake policy or successful empty producer is
created. Source getters005A5C80/005A5CB0/005A5D10 were inspected but receive no new
Human ledger credit and their implementation files were not edited.

`004BA070` is also connected as `advance_command_point_target`. Active nested
roll returns; otherwise native10 clears. An out-of-command-area pointer keeps
its old target/position. Actual World Scene94 supplies origin/direction at the
truncated pointer pixels, then the existing loaded World collision service casts
100*direction with layers0/flags610000/no normal. No hit publishes the100-unit
endpoint and direction's fourth word, then clears target. A hit publishes XYZ
minus.2*direction, preserving position2C; original Z has a FLOAT intermediate.
Static/nonactor results clear target. Actual Humans preserve150 suppression,
attachment redirection and kind3/live/nonroster rejection. StaticWeapon requires
2F0==1; Animation excludes targets when A4 is set. Other actor kinds retain the
actual actor. The receiver does not simulate camera/Human/weapon/world state.

Initialization continues through existing World/HudPanels bind: actual scene,
eight inline texts, pointer artwork, nested roll and shared click sound. The new
geometry workers allocate no persistent owner. Cached actor identities use
ActorBase addresses, resolved through actual Scene2 before dereference; point
queries resolve actual runtime actors and collision descriptors. Existing actor
retirement retracts command target and, as lifetime guards, current/previous
borrowers before destruction. Command discard clears cached identities; pointer
discard clears frame/scene/clip borrowers. UI callback borrower retires before
World/HUD/font/scene owners; the existing process click closes with its HUD.

Verification: one expanded native ASan/UBSan/LSan audit runs100 construction/
retirement cycles of actual command/auxiliary/TextState services with explicit
engine/graphics/Human-query endpoint doubles. Positive paths cover all highlight
styles/order, attachment seat selection, camera/epoch gates, min-size/offsets,
FISTP alpha, pointer selection/projection/clips, all caption tags/reset order,
all button-mask priorities, selected mutable order defaults and B0 fallback,
ray no-hit/static/Human/attached/StaticWeapon/Animation policies, failures and
borrower retirement. Invalid actor/model/camera/world/visual inputs and
protected writes are exercised. Strict i386 instantiates every new concrete World
method and the required Human command source, with no missing UI declarations.
Sequential application link passes; no Windows or original binary was executed.
Fixtures/binaries are fingerprinted then removed; no permanent tests added.
Only18 present frozen client entries gain partial credit. Full004ABAF0 extends
an existing partial. No complete promotion or delegated engine/Human getter
credit; frozen18783 and34 complete remain, partial2424=2419client+5engine.

Next UI-owned boundary remains full0049D6E0 ray/roll/signals selection,
004B6420 command frame/route-model/roll orchestration, reached map/inventory/
objective/modal workers and full0049BD50 dispatch. Existing runtime producers
still need to publish command14/18 and5A7BB; merely seeding these fields in native
fixtures is not runtime population. Human order/occupied actor producers remain
required external interfaces; missing UI-owned receivers are not attributed to
P22-HUM. FRAME/RENDER/OWN scheduling and activation remain pending. Loading stays
disabled; neither a rendered mission frame nor desktop acceptance is claimed.


## Roll-menu presentation and target lifecycle — 2026-11-11

[Receipt](checkpoints/p22-ui-roll-20261111.json). Resumed from planninga2d61c6
and implementationd01face. **P22-UI stays OPEN.** No paused request remains.
Read-only master advanced througha8833de to
`494cbee40566248084aaf9c8f01e7f75724d13c5`. Its Human and LIFE contracts were
inspected without merging/rebasing this branch. The shared repository remote
was externally renamed fromorigin tofeature/p22-pack during work; the latter
still points to the same GitHub URL. Only a read-only master fetch was made;
this agent did not rename it or alter other worktrees. The merged LIFE boundary
already owns the actual UI adapter and retires it before the World. It does not
enable GameApplicationState or complete UI-owned receivers.

`hud_roll_policy.h` supplies ordinary004B0F30 and command004B11F0 eligibility,
and004B13C0/004B1EE0 row population. Ordinary uses selected28 via0045D160;
command uses previous30 via0045D1B0. Actual current2C is not substituted. Native
Player mode/action/animation, Human174/160, weapon cooldown, distance1.8,
roster/status/team, Car, StaticWeapon and Animation gates remain distinct.
The ten actual roll34/38 action/caption pairs live in HudDialogsLoading, not a
menu system or second World. Action-1 is absent,50 disabled; row9 retains the
original exit action51. Attached Car menus start seats after exit; kind34 does
not. Command Car final row0 overwrites seat0 as the original does. Fresh seat
counts and repeated occupant queries remain observable. Identity captions use
wrapping DWORD -10-id arithmetic without signed-overflow UB.

Instruction checks corrected misleading decompilation: ordinary friendly
healing row1 reads the SELECTED Human's health percent and TARGET Human's medical
item; row2 reads the selected Human's medical item and target health. Nonoffline
friendly treatment checks TARGET174. The SupplyBox ordinary ammo path really
passes a boolean0/1 as the ammunition definition argument; command passes its
actual definition. This odd source behavior is retained, not repaired. The
004B0920 hostile StaticWeapon branch makes THREE seat0 queries and stores the
supplied distance normally; decompilation incorrectly appeared to store actor
pointer bits. All three query results are checked before source dereference.

`004B0B20` clears SAME roll TextState, appends localized1B69/style2F, target
caption/style31, then nine numbered/empty rows and final0 row. Role1 Human uses
short profile10 (user14 first), Door234 appends localized1B8C with`%s - %s`.
Negative caption encodes actual Human identity. An unselected seat action8..13
uses the actual occupant's short name. Disabled50/style34 takes priority over
selected/style35; ordinary/style33 remains separate. Missing negative-identity
Human leaves the number prefix only. No shared menu font or replacement text
owner is allocated.

`004B0EF0` calls the real ordinary/command population policy, clears selection84
to-1 and cycles. `004B2F50` finds the next nonabsent/nondisabled row without
wrapping, formats only on change, and starts SAME process HUD click even when
no next row exists. `004B2CE0` reads actual world bounds and computes row from
bounds5, not bounds1, using FLOAT.04500000178813934 and58.823524475097656 followed
by007E5E00 truncation. Native unordered comparisons can admit NaNs to the masked
invalid conversion/lowDWORD0; source preserves this. Original out-of-bounds row
indexing is rejected as a defined guard. Outside pointer leaves selection;
invalid/disabled row clears it without an invented text rebuild.

`004B3010/004B3030/004B3060` refresh existing text geometry, position via frame08
then24, or restore protected XYZ from actual bind-time20 while preserving W,
respectively. `004B2FC0` ordinary tick hides for no selected Human/dead Human,
then subtracts elapsed from nonzero DWORD18 without clamping. Command31 skips
the whole body. `004B27B0` hide calls root30(false) then clears30; show eligibility
failure clears30 without hiding root, as originally. Accepted show stores30,
populates/selects/clicks, formats again, stores400 ticks and shows root. A failed
population invalidates interaction readiness instead of accepting cold rows.

`004B0920` publishes actual target88/distance00, redirects focus and occupied
attachments, handles distance rejection, and on changed ordinary target performs
original hide/re-show or eligibility rejection. Concrete callable World entries
are publish_roll_target/show_roll/cycle_roll/hover_roll/advance_roll/position_roll,
selecting SAME ordinary dialog or command590 nested dialog. They obtain actual
roster, B0/DD0, Items/storage, Door properties/portal, SupplyBox loading flags,
Car loading seat metadata and StaticWeapon seats. Medical lookup reads actual
holder5 then reuses the existing reverse-holster lookup for kind13; placed-item
lookup reuses kind19. Existing Human inventory count/definition helpers and
actual item ammunition/cooldown data are reused. No Human mechanics were added
and no getter/delegated-helper ledger credit is claimed.

`GameUiRollExternalSource` explicitly requires typed Human174 focus, actual
occupied-Car254A/004D5430 eligibility and007DAF80 pickup-capacity producers.
Loans are consumed in-call. No producer is stored by the adapter or replaced by
successful empty data. Unsupported actor families7/23 have no actual loaded
owner in this graph; their concrete queries still reject. Nonoffline Human
team/name producers remain required. These conditional producers are not native
fixture population. The full UI closure checklist stays unchecked until all
mandatory binding/positive boundaries are resolved.

Populated retirement now retracts ordinary and nested-command roll88, invalidates
rows, clears text and clears30 before reporting the still-required original
signal/reset004C11C0. Existing actor cleanup may aggregate an error and continue;
retaining88 on that path would leave a dangling borrower. HUD0049EF70 therefore
aggregates ordinary and command retirement errors so both targets and command
14/18/1C retract even when reset fails. This is a source lifetime guard, NOT a
successful implementation of004C11C0 or its required signal rebuild. Signal
population/control/reset and roll action/control dispatch remain UI-owned work.
No callback captures the UI adapter past LIFE retirement.

Verification: actual HudDialogsLoading/TextState/row policy bodies pass native
ASan/UBSan/LSan100 repeated lifetimes with frame/font/actor-query endpoint
doubles. Positive cases exercise ordinary/command eligibility and row families,
source health/medical roles, actor/seat captions/styles, target redirects/changes,
three StaticWeapon queries, hover/cycle,400/wrapping timers, protected restoration,
localization/click/geometry failures, close/discard and retirement error aggregation
without stale borrowers. Strict i386 compiles every new World entry and required
external producer interface; application links without execution. A temporary
source snapshot of latest master494cbee plus this branch's37 UI source files
also compiles strictly, with zero three-way source conflicts; it is not a merge
or rebase of the worktree and not runtime acceptance. Existing tracker tests pass.
Temporary native sources/binaries are fingerprinted and removed. Only15 frozen
UI entries gain partial credit, existing0049EF70 extends; no complete promotion.
Branch-local partial2439=2434client+5engine; frozen18783 and complete34 unchanged.

Next: finish signals004BF4A0/004BFFE0/004C0370/004C0EF0/004C0F70/004C11C0 and
roll control004B2DC0/004B2830, then ordinary selection0049D6E0, full command004B6420
and remaining panel/common HUD dispatch. Signal source exports are already
retained underp22-ui-signals-runtime/p22-ui-signals-show and main's
world-retirement-hud-callees; use the research index before any new export.
No full P22-FRAME/RENDER/OWN takeover or application activation; no original
EXE/DLL/Windows fixture/Proton execution, rendered mission frame or desktop claim.


## Signals, original control consumption and vehicle presentation — 2026-11-11

This checkpoint supersedes the preceding signal/reset gap, without closing
P22-UI. Receipt: [signals and vehicle](checkpoints/p22-ui-signals-vehicle-20261111.json).
Latest read-only master remains494cbee40566248084aaf9c8f01e7f75724d13c5. The shared
remote is again namedorigin; only fetch/read operations were performed.

`004BF4A0/004BFFE0/004BF640` now publish and populate the actual signals owner,
using its original process table, ten rows, typed ActorBase target64/Human68,
category0C and selection60. Flag6C copies row10 before any selected-Human/order
query. Other pages preserve actual roster/order/formation/item/Car/Door/Supply/
Animation status policies. `004C10F0` supplies offline target eligibility.
No Human combat/order queue or item transfer is synthesized. Actor7/23 and
conditional nonoffline fields still require real owners; those branches reject
rather than manufacturing actor data.

`004C0370` rebuilds SAME TextState with localized process-table headings,
actual actor/Human captions, numbered rows, absent/disabled/selected styles and
root visibility last. `004C06E0/004C0720` scan without wrapping and always click
before formatting. `004C0600` handles pages51..59, back71, close70, disabled72
and absent-1. Page transitions perform population, cycle/click/format, then a
second click. Actual command execution must succeed before hide and final click.
`004C0760` consumes a numeric scan before its row gate, or consumes activation19/
wheel54 before dispatch/cycling. GraphCC/1006F660 is a borrowed engine endpoint;
UI does not poll or initialize another keyboard.

`006C70A0` clears both snapshots for every reverse-visited binding matching
kind/code, ignoring direction/device index. `006C71C0` matches code only against
the full scan DWORD. Either null buffer selects both actual manager outputs,
as originally. Checked in-call views expose no retained buffer loan. The single
StartupSession accessor returns its existing input state; it creates no owner.

`004C0990` builds the interpreted kind3 request fields for direction, view-range/
range40 aim, target actions/SupplyBox row indices, formation and item pages.
`004C08A0` uses actual selected Human direction and active mission-camera origin,
excludes SAME Human, queries loaded World geometry and restores exclusion before
publishing hit or miss. Inspection of the actual call and006F7C90 parameter
mapping confirms110000 is the **excluded layer** argument here; it is not the
excluded descriptor-flags mask used by ordinary selection/command rays.

`GameUiSignalCommands` is a required typed execution interface for original
Human005A45E0→Player0053D910 kind12/embedded kind3 queue and downstream packet
serialization. Latest verified B0 first/second command storage supports kind6;
there is no real kind3 runtime producer to bind in this branch. The interface
consumes the stack request/actor loans in-call and has no successful empty
implementation. It does not transfer Human state ownership to UI. Likewise,
existing order/focus/seat/transfer source contracts remain explicit.

`004C0EF0/004C0F70/004C11C0` now implement active target changes, fallback,
page resets, selected/bound Human health, live-roster/process-group checks,
soldier selection0049D340 and original show/hide/re-show ordering. Existing
process group buffers are copied only into an in-call query result, not another
persistent Players roster.0045E9A0/0045EA00 use reverse actual-roster definition/
medical searches with original fresh Human lookups.

World/HUD loading precedes UI adapter construction. Construction checks both
ordinary/nested retirement slots, creates both potentially allocating callbacks
before publishing either, then binds them to SAME World UI receivers. Optional
order/external producer services are explicit service loans: they must outlive
UI or unbind first. Actual actor retirement invokes original roll publication/
signal reset; errors aggregate and every retiring ActorBase loan retracts even
when a required producer fails. UI destruction detaches both callback captures
and retracts actor loans before HUD/dialog/text/World retirement. No actor,
frame, font, texture or scene owner is duplicated. P22-LIFE already provides
UI-before-World retirement; application activation stays disabled.

`004A8DF0` is now bound through actual existing vehicle artwork. The containing
0049BD50 vehicle-tail policy is separately callable asadvance_vehicle_display:
HUD20 suppression, command, inventory and ordinary roll gates precede current
Human/Car queries. Accepted Car shows root first, then reads SAME initialized
fuel/capacity, solver speed and drivetrain state. A small checked read-only fuel
accessor inCarLoadingActivation is the only Car source change.004F4C50 converts
actual running engine speed toRPM with60, or returns its genuine stopped zero.
Speed converts withFLOAT3.5999999046325684. Needle worker clamps only upper speed
180/RPM9000, leaves fuel unclamped, and publishes speed01/02/03 in order using
existing engine Make/Normalize/frame flags/update operations. Negative/NaN
arithmetic follows source policy. This does not implement occupied Car simulation.

Native ASan/UBSan/LSan verifies100 actual dialog/TextState lifetimes and100 actual
vehicle-artwork lifetimes, including supported positive rows/pages/actions,
control alias mutation, request fields, aim restoration/failures, callback and
borrower retirement, modal gates, arithmetic and partial publication failures.
Engine/font/actor/query/executor endpoints are doubles. Strict concrete i386
compilation and application linking pass; temporary latest-master composition
and tracker results are in the receipt. Fixtures are fingerprinted and removed.
Twenty frozen client bodies gain partial credit;004BFFE0/0049EF70 extend. No
complete promotions, delegated engine credit or frozen inventory edits.

P22-UI remains OPEN. Roll actions/control004B2830/004B2DC0, ordinary selection
0049D6E0, command004B6420, map/inventory/info/scoreboard and full0049BD50 remain
UI-owned. Fresh original evidence identifies004AED20 as the existing scoreboard
text updater, distinct from objective icons004AD8F0/004AD9B0. Info00499210 calls
00496E70 after a signed500ms visibility threshold; its positive formatting
callees still need recovery. Original exports for these next receivers are
retained without implementation credit. FRAME/RENDER/OWN execution remains
external. No original EXE/DLL/Windows fixture/Proton execution, rendered game
frame or desktop acceptance.


## Ordinary roll action/control receivers — 2026-11-11

Checkpoint aftera3de5e1, [receipt](checkpoints/p22-ui-roll-control-20261111.json).
P22-UI and the roll family remain OPEN: action7 still requires the actual
UI-owned inventory open/population/control boundary; it is an explicit error,
not an external gameplay dependency or successful callback.

`004B2830` now produces original kind2 action/identity/parameter records,
including target-kind parameters for2/38, no-target exit5, occupied-seat pickup6,
seat changes8..13 with real attachment fallback, SupplyBox row pickup and loose
item pickup. Healing29 produces kind3 command53/action15 with selected Human
as target and actual roll actor as sender. Row9 exits without querying a Human.
The common tail clears88 and hides only after successful execution/publication;
required executor, invalid borrower and inventory failures retain current state
for real error propagation/retirement. No fabricated Human command or transfer
is executed. Actual host wrappers validate SAME World actors/typed owners and
borrow the existing process input buffers. Bound signal external services also
serve ordinary roll queries when no per-call override is supplied.

`GameUiRollCommands` requires actual Human005A45B0 B0 virtualD8,005A2CF0 virtual7C
and005A2D20 virtual6C producers; pickup operations also own original Human178
publication. It extends the already-required kind3 producer. These are gameplay/
Human operations, not UI-owned mechanics.0053D910 copies the incoming stack
record into its existing owned queue; that queue's actor retirement remains its
producer's responsibility. UI retains neither the request nor another actor lease.

`004B2DC0` skips the whole body for nested command31, otherwise reads actual
Graph scan/translation and SECOND input snapshot. Digits consume code aliases
before checking rows, invalid/disabled rows return, valid rows publish selection
andFLOAT1 to activation19. Activation calls the action worker without clearing19
(unlike signal controls). Positive wheel or arrow0 precedes negative wheel or
arrow1; both consume binding54 aliases before previous/next. `004B2F00` scans
backward without wrapping, formats only when a valid row is found and always
clicks. Cold/invalid indexes are defined source guards.

Native ASan/UBSan/LSan verifies100 actual dialog/TextState lifetimes and original
policy/readiness, packet fields, transfer delegation, seat fallback, exit gate,
control ordering/alias mutation, command31 rejection, invalid actor/command/
transfer failures and cleanup. Endpoint doubles are explicit. Inventory action7
was tested as a required failure, not a positive inventory path. Strict actual
i386, existing application link, temporary latest-master composition (45 UI
files, zero conflicts) and ledger checks pass. Three frozen UI entries gain
partial credit, no complete promotions. Fixtures are fingerprinted and removed.

Next complete inventory0049DE90→004A4380/004A3920 and required positive slot/
actor/data/control callees, reusing actual loaded inventory/Items/Human owners;
existing0049F1C0 compass policy and004ABAF0 pointer selection already exist.
The retained0049DE90 source also establishes signal/roll/selector/info/map hide
order. Do not replace any of these with empty callbacks. Root evidence and the
five info-format dispatch wrappers are retained underp22-ui-roll-execution;
info's positive shared workers still require full recovery. Selection, command,
map, info, scoreboard and full HUD frame dispatch remain UI work. No application
activation, original execution, rendered game frame or desktop acceptance.


## Inventory presentation, ordinary selection and nested command roll — 2026-11-11

Checkpoint after394a822; [receipt](checkpoints/p22-ui-inventory-selection-20261111.json).
P22-UI remains OPEN. This supersedes the preceding UI inventory action7 failure
and ordinary selection gap. It does not close command/map/info/scoreboard/full
HUD dispatch or general inventory transfer/drag controls.

`0049DE90` now connects the actual modal transition to SAME existing inventory,
signals, ordinary roll, selector, info, map, pointer and compass owners. Offline
uses current Human; nonoffline uses selected Human, original006B66B0 policy and
required005A45A0 request when appropriate. It publishes flags/actors first, hides
signals/roll/selector/info/map in original order, selects pointer0, shows inventory
and refreshes actual0049F1C0 compass policy. Hide respects command visibility and
pointer-1 before panel hide/compass. Nonoffline missing selected Human refreshes
compass only. Roll action7 now calls this receiver rather than rejecting it.
No successful replacement executor or second menu/World is created.

`004A4380` publishes actual primary/secondary ActorBase loans separately from
DWORD storage and formats existing inline names with original kind/rank gates.
`004A3920` publishes root visibility before hide/drag cleanup or second-side/
skills visibility and refresh. `004A26D0`/thunk004A49F0 reset dragging, publish
fresh actual actor weight/limit text and ratios, then body/holster/pack rows in
original first/second-side order, clearing dirty8 last. All77 HudOwner texts
and actual storage/Items/texture owners are reused. No unified/shared new font
owner or persistent item/actor owner is allocated.

`004A1DB0` maps body rows to5/0/7/3/4/2. `004A1E20` publishes original empty/item
textures, fresh holder reads, uniform opacity gates and weapon amount. `004A2010`
uses actual array storage and capacity, localized volume label, four visible
slots, family0 amount/count, family2 wrapping sum/count and other-family stack
counts; absent storage dims slots, disables heading and clears bar/text. Existing
007DF650 icon resources/material binding are reused. `004A2640` and004AE4D0/
004AE570 bind actual slot counts/range/index/percentage to SAME four scrollbar
owners. There is no fabricated capacity or alternate inventory storage.

`004A4680` uses actual Human stats in original4/1/0/2/6/7/8 order, retaining
health/raw/normalized percentage conversions, width5 formatting and bar update
order. `004A39B0` performs actual SceneFC40000 picking, strict root rectangle,
original greater400ms hover delay, tag-to-displayed-item lookup, visibility,
position, geometry and existing text rebuild order. Description includes original
blank/title/weight/ammo/speed/scope records.004A4630/004ABC00 keep strict ordered
open bounds and AL-only result. Float constants, format strings and descriptor
vtable/getter evidence are retained. Native payloadkind2/3 virtual14 getters
read different fields:007E0320+38 and007E0380+34. UI uses their corresponding
serialized offsets without modifying Human weapon/ballistic source.

`004A4150` now processes signed wrapping countdown, dirty bit and fresh actor178,
then skills and description in original order. General input/drag/drop/transfer
callbacks are still distinct UI work, and actual transfers remain Human/Items
operations. Conditional non-Human178, nonoffline name/rank and remote request
producers remain explicit errors/required typed interfaces. Supported ordinary
roll action7 supplies an actual Human secondary; native tests also exercise the
conditional policy with explicit endpoint doubles, not network populations.

Visible0049FBF0 now reaches actual UI-bound refresh. UI construction prechecks
ordinary/nested dialog, inventory-refresh and Items-retirement slots; creates all
potentially allocating captures before publishing any; then installs a nonowning
noexcept Items callback. Existing OwnedEntry::forget calls it before RuntimeItem/
pack destruction. Inventory item references and tooltip contents retract without
engine calls; actual module membership precedes subsequent dereferences. Actor
cleanup also retracts matching inventory loans before original HUD cleanup.
UI destruction detaches Items observer first, then inventory refresh, dialogs and
service loans. HudPanels closes/discards geometry/text/frame borrowers before
scene release. Existing P22-LIFE must continue retiring UI before World; no
activation change or duplicate Human/Items/VM ownership is introduced.

`0049D6E0` now uses SAME HudScene private10/14 counters and6B8F8/FC actor pair.
Visibility runs first, command visibility returns before time accumulation,
offline signed100ms throttle wraps as native. Actual current Human cameraFPV/
camera3RD marker and active camera direction/range produce the ray, with1.5
attachment origin shift. First210000 and conditional110000 are excluded-FLAGS
masks here (distinct from signal aim's excluded-LAYER110000). SAME loaded World
geometry and actual Human exclusion/restoration are used; failures restore before
propagating. Original head distance special cases, five-miss hysteresis, blocked
Door.65 filter, alternate Human crosshair<=2 and ordinary<=.2 policies remain.
Roll, crosshair, eligible pair, raw pair, attachment override and active-signal
publication preserve original order; no actor/frame survives its real owner.

`004BB0E0/004B5AD0` now operate on actual nested command590 dialog. Show offsets
pointer X by.1, Z by1, clamps Y to-.2/-.6, refreshes geometry, publishes target1,
shows actual roll, selects pointer0 and locks all18 buttons in reverse native
order if accepted. Hide restores protected position/geometry, hides and restores
policy/tag1..5/16/17/18/paused states. This supplies real transitions for later
004B6420; it does not implement that full command frame or route/model producer.

Native ASan/UBSan/LSan passes100 actual inventory/HudOwner77/TextState/storage/
scroll lifetimes,100 ordinary selection runs and100 actual command/dialog
lifetimes within the inventory audit. Verified positive data includes populated
one/two-sided inventory, real stack/holder owners, counts/weights/skills/icons,
descriptions, modal ordering, signal/roll/selection policies, range resets and
nested lock/restore. Failure coverage includes localization/material/actor/item/
callback rebinding, ray/head/crosshair failures and retirement. Engine/font/actor/
query/descriptor endpoints are doubles. GameMissionItems observer and actual
World identity/owner composition compile strictly; Windows host module execution
is not claimed. Temporary master494cbee plus51 UI files compiles, zero conflicts;
no active-branch merge/rebase or reference-worktree edit. Application links
without execution. Tracker checks and19 new partial bodies are recorded in the
receipt; no complete promotions/frozen census changes. Fixtures are hashed then
removed per project agreement.

Still UI-owned: full004B6420/004B9E80 command caches/routes/model/feedback and
required control receivers,004A6FA0 map,00499210/00496E70 info population,
004AED20 scoreboard text, general inventory transfer/drag input and full0049BD50.
Required Human/Player order/action/queue, focus/seat/transport producers remain
separate; FRAME/RENDER/OWN scheduling/rendering/activation remains pending.
No rendered game frame, original execution or desktop acceptance is claimed.


## Command frame and route consumers — after4519039

[Receipt](checkpoints/p22-ui-command-frame-20261111.json). The actual command
owner now supplies004B6420 through GameUiPresentation::advance_command. It gates
on its SAME World/root visibility; updates hover/cooldown58; publishes current18
then previous14; marks selection change before policy refresh and nested-roll
hide; reads the fresh last order; visits the actual roster with fresh counts;
updates routes before Human005AC470; clears the change flag; updates/hides nested
roll; expires feedback740/744; gates point target on5A7B8/9; calls the SAME
mission-camera detached controller; then, when5C is clear, updates the existing
GEffects module and presents the temporary selected list in reverse. Soldier
selection mode3 precedes wrapping54 and pointer-kind2 rotation.

Assembly004B653E/004B6678 proves005AC470 receives an INDEX, not elapsed time.
Its Human174→00793490 and DD0→00786160 producers remain explicit, as does
00463BA0 camera simulation. The concrete existing GEffects update receives
Players.control(), not the module wrapper. Its unrecovered positive water,
weather, colour and paused-status policies still fail explicitly; UI does not
replace them with a successful callback or take over rendering/simulation.

004B9E80 now rebuilds SAME HudCommandRoutes and publishes existing order84
models. Gates short-circuit in original5C/changed/selection order. Zero orders
clear both counts; a positive rebuild clears only path count, retaining the
auxiliary strip. Each fresh Human order prepares its actual74 positions from the
prior endpoint, appends the original overlapping strip (selected blue/other
black), shows its84 model, finds selector_commands with mask1, and selects it
only for the previous Human AND the selected ORDER INDEX. The decompiler's
model[5]/constant1 test is incorrect:004B9FD9 reloads command14 and004B9FF3
compares the loop index. The endpoint for the next order is read AFTER model
publication, from actual target40 world position or order30. Empty paths, missing
models/selectors and buffer overflow fail rather than publishing success.
004AB820 now queries the SAME HUD order-info Human and original selected byte.

GameUiHumanRouteSource::prepare_route is a required, in-call loan of actual
Human order storage; GameUiCommandRouteView owns nothing. This interface DOES
NOT recover0058CCE0/0058CAF0. Original evidence shows that these mutate order74
and84, use loaded navigation/door/car endpoints and register an order descriptor;
verified master has no typed populated order record/model owner. That missing
Human ownership is external; the UI-facing preparation work remains explicitly
OPEN until it can be implemented against that owner. Do not credit the interface
as an implemented original body, or assume Agent1 has implemented it. No copied
order list, invented straight-line path or duplicate persistent model was added.

004B86D0 tags16..18 now produce real UI state: options toggles/clicks and
publishes200ms feedback then writes SAME session04=0E; camera-speed clicks/hides,
cycles the existing detached speed and publishes feedback; realtime toggles5C,
updates its button and requires the containing0069B8B0 receiver. That receiver
must honor game28>=2500/offline7, notify time0071F640 with effective64, then
005E8D50 and SOUND00428F00 with the requested value. It belongs to FRAME/OWN;
UI neither simulates its notifications nor enables application loading.
Feedback expiry toggles the ACTUAL current button visibility at signed<=0 and
clears its tag; the native borrowed inline-button pointer becomes a checked tag.
Other command controls and5A7BB projection-state production remain UI work.

Ownership/retirement order is unchanged: actual HUD scene/text/artwork/dialog/
mission-camera owners initialize before UI; all new source arguments are borrowed
only for the call; UI caches only the original current/previous/target actor
identities and scalar command state. Existing actor retirement clears these loans;
UI detaches dialog/Items/refresh callbacks before World retirement. Route buffers
are the original six arrays. No new retained actor, frame, camera or font loan,
second World, Human/Items owner or model allocator was introduced.

Native ASan/UBSan/LSan passes100 actual HudCommandLoading/HudAuxiliaryLoading
bind/frame/control/route/close lifetimes, including populated paths, selected
model publication, actual order-index query, blue/black strips, target endpoint
chaining, auxiliary retention/reset, changed/force/visibility/mode gates, exact
indexed presentation order, pointer rotation, timer wrapping,200ms feedback and
speed/realtime controls. Producer/missing-model/missing-selector/empty-path/
overflow/camera/realtime failures propagate. Engine/font/actor/order/camera/time
endpoints are doubles; this is source-policy execution, not original Human
route generation, real Windows host construction or desktop rendering.
Strict i386 actual UI frame/routes/control composition, sequential application
link and temporary master494cbee plus52 UI files pass with zero three-way
conflicts. Four new frozen partial entries; no complete promotions or census
changes. Fingerprinted fixtures are removed after checks.

P22-UI remains OPEN. Next independent UI work is map004A6FA0, info00499210/
00496E70, scoreboard004AED20, inventory transfer/drag callbacks and full0049BD50;
command route preparation/remaining controls/projection publication remain open
alongside the explicit Human/Player and FRAME/RENDER/OWN dependencies. No original
execution, rendered game frame or desktop acceptance is claimed.


## Scoreboard text — after6fe6f6e

[Receipt](checkpoints/p22-ui-scoreboard-20261111.json).004AED20 now binds through
GameUiPresentation::advance_scoreboard to the existing HudScoreboardLoading and
its own TextState/TextObject. It never allocates a second font/panel/process
score registry. The elapsed argument and root visibility are unused natively;
style-1/unknown styles return before any query. Offline7 bind sets style-1.

Style1 reads actual process FLOAT97D4BC/C0 through a required source, truncates
like007E5E00 low EAX (including defined invalid/overflow low-word handling), then
queries team time and remaining time. It replaces indexed0..3 with styles43..46,
integer scores and space-padded-minute times. Style2 visits EXACTLY32 physical
reserved Scene2 identities, not the current selected roster or pending lookup.
It counts health>0 and unsuppressed005A63F0 Humans; team0 goes to first count,
every nonzero team to second. Counts publish before current configuration240
is read; team time style45 occupies index2 only with teams enabled; remaining
time style46 uses zero-padded minutes at index2 or3. No stale row is removed.

Style3 reads team time, signed elapsed game30 and mission score. Configuration
27C chooses actual game84 loss count; otherwise the required process peer owner
must lock0072EC70, read local00982104 peerB0, read SAME config270, then release
0072ED30, including failure cleanup. The limit is read BEFORE release, as
004AEDF6/004AEE01 confirms. Limit-1 skips replacement and keeps record0; otherwise
wrapping limit-loss/limit uses style44/index0. Team and elapsed times use45/1,
46/2 even when the fraction is skipped. Actual frontend::TextObject::set is
reused:006443C0 wrapper calls006440F0 at00644419. No clear/rebuild/row normalization
or invented limit clamp is added. PE literals confirm %d, %2d:%02d, %02d:%02d,
%d/%d at00832D40/0083B264/0083B084/0083AD98.

Timer helpers0069AE80/0069AEB0 are concrete over SAME configuration/session:
230 minutes*60000 minus game30 wraps then clamps when the resulting sign bit is
set;23C seconds*1000 minus game7C wraps with no clamp. They do not advance the
session clock or implement P22-FRAME. Human team005A8820 uses existing properties:
offline kind2→0,1/4→1,others→2; remote-AI kind2/5→0,1/4→1,others→2. Actual
Human selected suppression is reused. Other network team/score/peer fields are
required GameUiScoreboardSource calls, not discovery/menu data or successful
zero defaults. These loans last only for the call; no new retirement ordering.

Native ASan/UBSan/LSan passes100 repeated panel/actual TextState/TextObject/font-
reference lifetimes across all styles/teams, hidden root, offline/unknown style,
positive counts/fractions, retained records, negative time/score, float conversion,
wrapping timers and missing font/root/team/score/peer failures. Font/frame/actor/
network endpoints are doubles; real font/desktop rendering and network execution
are not claimed. Strict actuali386 UI composition, application link and temp
master494cbee plus53 UI files pass, zero three-way conflicts. Three frozen partial
entries added, no complete promotions/census changes; temporary audits hashed
then removed. This completes scoreboard UI policy, not the remaining full HUD,
map/info/input/route preparation or external FRAME/RENDER/OWN dependencies.


## Info/modal text and help — afterb5c8424

[Receipt](checkpoints/p22-ui-info-20261111.json).00499210/00496E70 and positive
00496E40 now bind through GameUiPresentation::advance_info/refresh_info/show_info
to SAME HudInfoLoading/TextState/TextObject/73 process style handles, World,
SessionOwner, mission, profile/input and Human/Items owners. No new retained
service source, font, World, roster or peer registry is allocated. The elapsed
counter140 wraps as DWORD and is compared SIGNED>500 before querying root
visibility. Hidden frames keep accumulated time; successful refresh resets0;
failed refresh propagates without resetting. Positive show rebuilds before root
visibility. Refresh clears text before dispatch, including unknown kind.

All five original sequences00496ED0/004970F0/00497130/00497170/004971C0 and reached
formatters are implemented. Ordinary mode appends title/spacers/objectives/help.
Objective00497210 queries actual mission.related live count, skips status4,
uses actual word8/word10 offline and original active/success/failure/special
captions. Nonoffline faction captions/team fields require actual producers;
unknown local team skips that objective, including its status caption. Mission
peer mode adds elapsed HH:MM:SS; individual mode adds conditional time/frag limits;
team/objective modes preserve their distinct spacers, tables and footer order.

Native info04 stores localization IDs, not Frame pointers. That existing owner
is now vector<uint32_t>; it is cleared at bind/discard and owns no actor/frame
loan. Conditional runtime/save population is still required; no help IDs are
fabricated.00496F00 expands actual localized strings using the original ordered
36-entry00842500 token/action/mode table.006A8360 selects the FIRST matching
profile BindingSetting.index/mode; existing StartupSession::binding_name supplies
actual profile/input key labels. No new control mapping or label source was
invented.006AC1C0 chooses first TABLE match, not earliest token in the text;
006ABE80 preserves exact single quotes/spaces and1023-byte prefix clamp. One
replacement is repeated while a nonempty angle-bracket pair remains. On unknown
token,006AC1FE calls00405180 to COPY CURRENT INPUT before AL=false: this is NOT
an empty-output/error placeholder. The caller stops expansion and appends that
text. Read-only ASM resolved the missing receiver in decompiler output. Actual
TextObject localization/set/append and existing version formatter are reused.
The recovered Startup key-name branch is English; no extra language acceptance
is claimed. Defined missing-binding/no-progress guards replace unsafe originals.

00497380/004977E0/00497E60/00498430/00498730 now format real peer-field interfaces
rather than delegated table-render callbacks. Under required actual0072EC70/
0072ED30 lock, UI collects nonempty peer names, rereads live counts, sorts individual
score4C descending or mission names with C-locale stricmp, and formats original
selected/dead/rank/stat styles. Name sorting uses007E88F0 (ASCII A-Z folding in
original default C locale), not case-sensitive comparison. Clamp rank60 to0..7;
left/right glyphs use0083A8BC oklmpqrn/OKLMPQRN. Existing Human hand/weapon lookup
supplies descriptor44 ALIGNMENT via GameMissionItems::human_entry_alignment;
this is not weapon-kind or a new combat operation. Stats read44/48/4C/50 in the
original column order. Team rows alone ignore dead-state queries when game88==3;
null left row emits a spacer, null right emits nothing. Lists greater than16 and
unequal teams are supported; no invented truncation or stable tie rule.

Heading1 is team1/left: FLOAT97D4C0 or count97D4CC. Heading2 is team0/right:
FLOAT97D4BC or count97D4C8. Original00497EB6/00497EDE/00497FBA/00497FE2 prove this
order. Actual team1 peers render left and team0 right. Localized heading text is
read before the global value. Peer keys/actor/name/stat loans never leave the
call or lock scope. Temporary key vectors die before release; release also occurs
on formatter/resource/query failure. Failed acquire must itself retain no lock.

004996B0 uses localized930 and actual process00981E70.00499770 requires the real
nonoffline gameA8/6C mission-title publication.004997B0 formats signed game30 with
localized3D22.004998C0 reads actual configuration230/22C; time>0 publishes WRAPPED,
UNCLAMPED minutes*60000-game30, unlike scoreboard0069AE80; frags>0 chooses original
138/13C based on the time-limit gate. Clock fields preserve signed division and
negative remainders. These do not advance session time or take over FRAME.

GameUiInfoSource explicitly requires the actual network peer lock/records,
00725AC0 status, local team, faction-descriptor fields, mission title, address,
score/count publications. It has no default empty producer, menu-discovery data,
render callback or copied persistent peer owner. Actual World actor membership
is checked before a peer Human is dereferenced. These conditional data producers
remain external/unbound in this verified source; native doubles demonstrate UI
policy, not actual network population. Existing UI/World retirement order is
unchanged: source loans end in-call; UI retires before World; info styles/text
retire before their existing HUD scene. Help IDs contain no borrowed pointers.

Native ASan/UBSan/LSan passes100 groups with all five positive kinds (500 actual
panel/TextState/TextObject/style/shared-font lifetimes), objective states and
faction gates, recognized/unknown help, exact records/styles/clocks, signed500ms/
wrap/show gates,20-peer list/reallocation, case-insensitive names, team identities,
unequal lists/result override, and load/localization/peer failures with lock and
font/style cleanup. Frame/font/objective/peer/binding endpoints are doubles;
this is not real desktop rendering, network runtime or Windows host construction.
Strict actuali386 UI methods and temporary git-archivef74816a plus55 UI files
compile with zero three-way conflicts; native and application checks are in the
receipt. Latest master GUARD changes were inspected without merging/rebasing or
editing Agent1's worktree.23 new frozen partial bodies, one existing entry
updated; no complete promotion/census change.00496410/00496430 comparators are
ABSENT from the frozen census and receive no credit. Fixtures hashed then removed.

At the info checkpoint, map004A6FA0 and full0049BD50 still awaited implementation;
the following checkpoint supersedes those two source gaps. P22-UI stays OPEN.
Map research now confirms dispatch0→004A5B50,1→004A6DE0,2/3/4→004A65B0. Stock
Ghidra failed CFG collapse for004A5B50; original ASM/failure metadata are retained
under ignoredp22-ui-map-research. Do not mistake failed decompilation for an
external blocker or add a successful wrapper around its missing body. Existing
Human/Player/conditional network and FRAME/RENDER/OWN dependencies remain.


## Map frame, show, controls and full HUD source — 2026-11-11

[Receipt](checkpoints/p22-ui-map-frame-20261111.json). Original evidence is retained
under ignored `analysis/work/p22-ui-20261111/p22-ui-map-frame`. Frozen client SHA
8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307 is unchanged.
The stock004A5B50 CFG failure was resolved with rollback-only Ghidra signature/
stack-purge corrections (notably005A80E0's invalid2147483647 purge) and disabled
unreachable elimination. Successful corrected exports, scripts, metadata and
original ASM are retained. No project transaction was saved, new functions
created, original code executed or frozen inventory changed. Register-corrupted
expressions in decompilation were checked against ASM, rather than ported blindly.

`GameUiPresentation::advance_map/show_map/process_map_controls` now call the SAME
`HudMapLoading` owner.004A6FA0 dispatches0 to004A5B50,1 to004A6DE0 and2/3/4 to
004A65B0; the worker ignores elapsed/visibility, while the full HUD caller tests
the MAP frame at108FC, distinct from nullable center108F8.004A5630 publishes
camera flag24 and center/map visibility before pointer, zones then goals.
004A7010 returns before input access forkind0/1; otherwise second action55
invokes004A7050 and consumes actual binding aliases in both buffers AFTER the
receiver returns, even for a no-hit. Failure propagates without consuming input.

Offline frame publishes actual current Human position/005A80E0 direction, scale
1.5 and visibility, then fresh roster count/entries. Current allied entry hides
without update; other entries use A4 alpha0/1 for dead/alive.0045ED10 builds the
known-enemy snapshot from reverse actual Players.selection records with nonzero
references, health>0 and !005A6380. Actual preparation GAME MODE3 (006B4670 owner08,
not difficulty) admits the Scene2 single-Human fallback only when that list is
empty. Reverse marker publication rechecks health/unavailability. Camera flag24
uses the actual active Scene camera; full3D normalization precedes X/Z flattening
and separate2D normalization. Editor objectives update last. Neutral style only
updates current position/direction/visibility, preserving prior scale/alpha.

Network frame reads width then signed height, current identity and actual
Session71/78 VIP0069E070. It visits the original reserved roster, including
pending identity slots, with player/health/suppression/team gates. Speech geometry
reads marker WORLD matrix twice and preserves FLOAT versus extended intermediate
stores, original7 geometry fields/depth/dirty flags, team styles4C/4D, and signed
Human254%500 opacity. It deliberately keeps tiny negative residual at250 and
negative-time values above1; no clamp is invented. A4 alpha differs from A8
opacity, and stale label flags survive hidden/dead/team-rejected branches.
VIP uses A4 .3/1 and fresh current/VIP identity for1.5/1 scale. Tail order is VIP,
kind20 item markers, network goals, zones.004A51D0/004A5400 shrink reverse and
create/duplicate/link/hide/update through actual existing engine wrappers.

Kind20 markers snapshot the SAME RuntimeItemServices registry owned by
GameWorldActors.item_models(), not another actor/Item index. The small
GameMissionItems::runtime_item_identity scans tracked owners before dereferencing
a registry loan; position comes from actual holder Actor model or primary Item
model. Actor/Human membership checks precede borrowed field reads and RTTI;
Scene2 fallback also checks membership before casting. Camera and model pointers
are consumed in-call. No Human/Items/VM simulation or persistent owner is added.

004A7180/004A7A70/004A7A20 update zone faction state first, then resolve the
actual SessionOwner.live_squad FIRST record's network_identity(native04) and
specialization(native40). The selector uses zone WORLD position, Z-.5 and tag;
wrong-team/spectator match clears network_identity to-1, while no match retains
it. Click tags1000..1014 use the same actual record and reverse zone lookup.
Actor retirement retracts map zone borrowers before destruction; a retired zone
becomes neutral and cannot publish selection.004A78E0/004A7C60 preserve spectator2,
state/team cache gates, native1C/20 titles, success/fail B63/B64 and status2's
retained faction visibility.004A7940/004A7F50 preserve editor state/cache policy.

GameUiMapSource requires conditional network team/local-team/objective fields,
Human214 full speech text, and kind37 actor98 data. The actual kind37 loader and
A4 group producer are still unavailable in WorldHud.map_zone_group; populated
zones remain explicit, not a successful empty implementation. Native tests of
zone policy do not establish real zone population. Existing GameDataNode lacks
network objective18/1C/20, and HumanActivationFields retains only speech's first
byte and timer; the UI does not fabricate their missing publications. Preparation
mode is a checked read-only getter of the existing Startup preparation owner;
frame callbacks never allocate it. These conditional fields remain runtime
producer dependencies. Ordinary editor objectives and actual item registry reads
are bound to real existing owners.

`game/hud_frame.h` reconstructs0049BD50. HudSceneLoading's positive rejection is
replaced by a typed dispatcher, and GameUiPresentation::advance_hud invokes it
through actual World dependencies. Zero flags return without panel/data calls;
warming flags process full message editing/timers then hide crosshair. Active
frame updates injury only when enabled, publishes actual server-message6B6BC
from game64 and command visibility, captures current Human once, refreshes actual
0049F1C0 on Human178, then injury movie, pointer, visible info, map and camera.
Offline order is orders/spread (or previous-Human orders in command), messages,
selection; nonoffline is selection, spread, messages. Signals supersede roll,
which supersedes visible selector. Progress uses the captured Human. Flags18
control hands, unpaused status and reverse visible soldiers; timers/carnage then
compass, visible command, visible inventory, vehicle, scoreboard and genuine
004AF470 RET follow. The flag tests and command/visibility queries remain fresh
at their original points. Vehicle also uses the captured Human, rather than
silently re-querying a possibly changed roster. No session clock, camera/actor
simulation, world rendering or containing frame scheduling is added.

GameUiHudFrameSources is an in-call set of required Human order/command/camera,
console and conditional network services. Closed gates do not require unrelated
producers; reached missing producers reject explicitly. Existing actual leaves
are reused, not duplicated. A positive offline frame still requires the real
Human order owner; a native seeded source is not that runtime owner. Console
editing uses the existing explicit00720130/00720340 receiver. Actual application
loading/activation remains disabled.004AAD30's conditional team tail is now also
implemented: changed team shows mpallied for1, then mpaxis for!=1, caching only
after both writes. Offline/style0/null-Human gates skip that producer. It uses
actual005A8820 policy and the same required network source as map status data.

Initialization stays existing SessionOwner→World→HUD scene/panels→map model,
allied labels and mission records. Dynamic markers grow within that same panel;
no second map, font, Players, World, Human or camera owner is allocated. On actor
retirement, map/selection/inventory loans retract before actor/Items destruction.
On session retirement UI detaches callbacks before World; map releases allied
visuals/text, reverse dynamic marker vectors and kind-specific records, then its
model, and discards scene/frame/objective loans before the HUD scene dies. Text
owners retain the existing shared HUD font only; process editfont remains
independent. All query/squad/source loans end in-call. Failure releases partial
owned resources through the existing panel teardown and propagates.

Verification: native ASan/UBSan/LSan passes100 groups/all5 map styles (500 actual
HudMapLoading/TextState/TextObject/shared-font lifetimes), positive allied/enemy/
VIP/item/label/goal/zone/click policies, signed opacity, resize/retirement/absent
map and resource/query failures. Separate100-group audits cover full HUD ordering,
cached Human, pause/status/visibility/flag mutation and failure propagation, plus
actual status panel team/posture/movement/cache/null/failure behavior. Full-HUD
leaf/query endpoints are doubles; map/status use actual panel/text code with
engine/data doubles. These do not instantiate a native Windows World or prove
real rendering/network population. Host borrower validation is source inspected
and strictly instantiated, not a claim of executed invalid Windows objects.
Strict actuali386 concrete UI and application link pass; no Windows code ran.

Latest read-only mastere1a94952c805966a7cf3937e58cc84e50ab005f3 has identical code
to683cb7b9646ce580b67252228212329c3e629a76. Temporary57-file three-way source
composition from31b01f9 exposes three overlaps: Car fuel getter versus new Human
seat services (retain both), mesh movie helper additions (retain both), and injury
movie show/state (retain the complete verified UI show/update policy used by the
new master damage wrappers). These resolutions exist ONLY in the temporary
compatibility tree, which strictly compiles; no active merge/rebase or Agent1
worktree modification occurred. UI's existing raw IShow30(0) behavior remains;
no decoder-equivalence claim is made by naming that operation rewind versus loop.
Receipt records conflicts instead of claiming a clean automatic integration.

Nineteen new frozen partial entries;0049BD50/004A5630/004AAD30/0049EF70 coverage
updated. Branch-local partial2530=2525client+5engine; complete34/denominator18783
unchanged. All fixtures are fingerprinted and removed. **P22-UI is OPEN.** Remaining
mandatory UI work includes general0049C3C0 input/modal routing and its positive
callees, command004B79C0 controls/projection publication, inventory004A3420 drag/
transfer controls, and resolving the producer boundary/composed positive runtime
verification. Frame/Render/Own/Human execution remains explicitly separate.

Fresh read-only0058CCE0/0058CAF0 exports show preparation mutates actual Human
order74/84 paths/model and refreshes its collision/inverse-matrix data, including
real mission navigation005223B0 and actor-specific path endpoints. It cannot be
implemented by returning a UI-generated straight line or a second order owner.
Actual latest master still rejects populated Active AI orders; no matching typed
order producer was found. These exports preserve the next dependency contract,
not implementation credit. General controls exports are also retained for the
next whole control-family pass;0049C3C0 directly reaches004B79C0 and004A3420 and
must include their required positive workers before claiming control completion.


## Inventory controls, scroll callbacks and pointer motion — 2026-11-11

[Receipt](checkpoints/p22-ui-inventory-controls-20261111.json). Original exports,
metadata and ASM are retained under ignored `analysis/work/p22-ui-20261111/
p22-ui-inventory-controls`. All exports decompiled successfully from the same
fingerprinted original image; register-corrupted ECX/EDX expressions were checked
against ASM.004A1DB0 returns only the holder number in EAX and leaves ECX/EDX
unchanged; it does not return a synthetic actor/holder pair. Actual command
receivers were recovered from register setup at004A2EE1/004A2FE8 and004A361B/
004A3635/004A36CD/004A3719. No saved Ghidra project or census edits occurred.

`GameUiPresentation::process_inventory_controls` calls004A3420 over the existing
HudInventoryLoading, same four HudScrollLoading instances, inline67 inventory
texts of the77 actual HUD text owners, Scene-retained drag visual and existing
first/second input buffers. It does not poll, create a device, advance the World,
move the shared pointer implicitly or allocate another persistent owner. The
containing general input receiver must call pointer motion before inventory
controls, under its original visibility/modal gates. Both entry points now exist.

004A3420 processes scroll controls first, then preserves the actual250ms window:
new second55 press outside the window starts004A3790; held first55 after expiry
runs004A3870; mouse release runs004A38F0; a second press within the window runs
004A3830/004A35D0.004A3910 on second56 is a genuine RET and gets no credit. No
input action or aliases are consumed by inventory. The frame update's existing
signed wrapping countdown is reused. Starting a click clears an older drag,
sets flag2DF4/time2DF8, captures pointer XYZ and picks an actual cached slot.

004A34D0 dispatches body tags200..205/210..215 and all four array ranges220..223,
230..233,240..243,250..253.004A2A40/004A2BB0 publish a typed borrowed Entry
(nativeC4), source Actor(nativeE4), source holder(nativeE0), and WORLD visual
position minus current pointer; Z offset is replaced with FLOAT.1. Arrays bind
the actual item icon immediately; body slots bind it on visible-drag transition.
Uniform row4 honors offline7, signed game status>=14 and actual stage25 expected/
observed table mismatch before the secondary-Human unavailable/1A5 gates. Native
8CAB62/8CB262 are stage25 indices. The UI reads existing Startup tables; it does
not initialize, fix, equate or fabricate their values. Real GUARD publication is
still required when that conditional gate is reached.

004A2CC0/004A2D60 preserve material→position→show→update and position→update
sequences. Any changed XYZ admits the hidden drag visual; no distance threshold
is invented. If the primary Human dies while holding,004A2DE0 performs cleanup
and refresh rather than executing commands. Release outside the strict inventory
rectangle requests005A2C50 unless source holder4; it then STILL performs the
original pointer-tag routing. Tags202/212 request drop, other body targets select
actual holder/displaced entry, and array targets use the visible ROW index (not
first+row). A same-Actor/same-holder release makes no transfer. A displaced body
entry selects005A2BB0; otherwise005A2B40. It then clears drag, hides/updates the
visual, clears selections in original primary/body/pocket/pack then secondary
order, and invokes existing004A26D0 refresh. Original ignored command return
values stay ignored; missing required producers throw instead of succeeding.

004A35D0 double click skips drop rows, chooses the opposite Actor when present
(or primary itself otherwise), original source holder and destination7/index0.
`GameUiInventoryCommands` is the typed required cross-Actor/Player/remote/drop
producer. With no supplied producer, move/swap reaches the actual SAME Human
loading composition through GameWorldActors.human_loading_owner and its existing
inventory command owner. Its supported AI loading moves/swaps execute there;
unsupported queued/cross-Actor operations still reject there.005A2C50 requires
its actual producer and receives fresh005A80E0 direction. The UI does not port
Human transfers, packet queues or item spawning merely to make a UI branch pass.
All Actor/Entry membership checks precede the corresponding field/owner access.

004A3280 now connects scrollbar enter/leave/hover, up/down click, held drag and
release, with original focused component, disabled and input gates. Both axes
read actual WORLD root and normal local positions, clamp to the existing seven
geometry fields, and publish changed percentage.004AE470 preserves extended
percentage/step+.5, DOUBLE store, original floor (007E6368→007EDB99 under raw x87
control173F), then existing64-bit FISTP truncation and LOW DWORD, unsigned cap and
changed-index callback. NaN FISTP indefinite has low DWORD0; the fixture was
corrected to that evidenced result, without changing the existing correct
text_ftol helper. Callback/engine failures propagate after original state writes.
Release snaps percentage back to the actual index. Disabled button/slider and
zero-movement branches are implemented; no successful empty callback exists.

The callback004A2590 exposed a real previous indexing defect. Native tags F10,
F20,F30,F40 name scroll13,14,15,16 and map to left pockets,left pack,right pockets,
right pack. Capacity sections are left pockets,right pockets,left pack,right
pack. Existing004A2640 refresh incorrectly selected scrolls by capacity section.
It now uses permutation0,2,1,3; callback publication uses the same evidenced
mapping and refreshes the actual side/storage. Asymmetric8/9/10/11-row stores in
the focused audit verify all four ranges independently. Original loading names,
tags and artwork are unchanged; this is not a second scrollbar owner.

004ABC50 shared pointer motion is also bound through
GameUiPresentation::process_hud_pointer. Null/hidden/rotation-kind2 return before
input reads; active motion copies the original four position words to10, adds
first52 X and subtracts first53 Y, preserving Z. Without clipping, X from0 to.995 and
Y from-.745 to0 clamp and clear ONLY the corresponding FIRST delta. With a borrowed
clip rectangle, original FLOAT.002 margins clamp using extended comparisons and
retain the input deltas. Second buffer/aliases are untouched, NaNs retain native
ordered-comparison behavior, and final raw position publication has no model24
update.004AB960 exposes stored XYZ delta; the fourth lane remains a defined
guard, with no recovery claim for the original unwritten stack lane.

No persistent owners were added. Drag item/source loans retract in existing
actor and Items retirement notifications before storage destruction, including
a source replaced by later actor publication. A defined pending-hide flag retires
the old drag visual at the next UI frame/control call; if that hide fails it stays
pending. It never dereferences retired Item/Actor data. Inline text/scroll
geometry/Scene-owned visual cleanup remains existing HUD lifetime; UI callbacks
unbind before World and source loans last only for each call. Human/Items/VM
ownership is preserved. The independent process overlay/font lifecycle is not
changed by inventory input.

Native ASan/UBSan/LSan passes100 complete actual HUD/77 TextState/shared-font/
slot/scroll/drag lifetimes, all body and array tag ranges on both sides, single/
two-sided quick routing, material/threshold/250ms/health/uniform/drop/swap/move/
same-holder policies, asymmetric scroll callbacks, producer failure and source/
item retirement. A separate100-group audit covers actual slider/buttons on both
axes, exact half-step rounding, unsigned cap/NaN, disabled/zero-motion/callback/
engine failures, and actual pointer delta/motion/clip/first-versus-second writes,
hidden/null/rotation/NaN gates. Actor, Item, transfer, B0/font/engine endpoints are
doubles; actual Windows World construction, Human transfer/queue execution,
rendering and desktop interaction are NOT claimed. Strict concrete actuali386
UI and sequential application link pass. Temporary latest-master122ef38 plus57
UI files also strictly compile, retaining the same three explicitly resolved
temporary overlaps described in the prior receipt; no active merge/rebase,
reference-worktree edit or activation occurred. Native fixtures are fingerprinted
and removed.35 new frozen partial entries; partial2565=2560client+5engine,
complete34/denominator18783 unchanged. No extra C++ helper, CRT/delegated engine or genuine RET credit.

**P22-UI remains OPEN.** Inventory control POLICY is implemented and connected;
real unsupported Human/Player transfer/drop producers remain external. General
HUD/modal0049C3C0 and command004B79C0 controls/projection still require their full
positive families. Original root exports and the required Human order74/84
preparation contract remain retained in the prior map/frame evidence. Future
FRAME scheduling, real populated composed UI execution and RENDER/OWN/application
activation remain separate from this verified UI source checkpoint.

## Command input and modal presentation — 2026-11-11

Source checkpoint after66889df; [verification receipt](checkpoints/p22-ui-command-controls-20261111.json).
**P22-UI remains OPEN.** This completes the command UI input/modal policy family,
not general HUD0049C3C0, actual Human orders, containing frame execution or desktop
acceptance. Initial master799a79305c70bc8de1860ac7e438bba25a71a8ac is unchanged;
latest read-only compatibility target is pinned60f43dcc8aa6c4249a94817712324a9e18d8e6ae
(after e969714 Human damage/packet implementation). No active merge/rebase,
reference-branch/worktree edit, push or loading activation occurred.

### Recovered receivers and ordering

Actual `HudCommandLoading` now binds004B79C0 through
`GameUiPresentation::process_command_controls`. Its18 keyboard priorities,
left/right/middle press/hold/release gates, nested-roll digits/actions and camera
input follow the original.004BA2D0 routes all18 button tags and ten order tags;
004B8470 changes only the selected policy group, with actual mutable Human44
policies;004B87B0 and004B8DE0/004B8E00/004B8E30/004B8EE0/004B8FF0 retain
real Human creation/insertion/removal/change producers. Feedback200ms publishes
after successful order creation. No task list, successful empty executor or
new Human mechanics were introduced.004BB200/004BB240 are genuine RET bodies
and receive no credit; their reached argument evaluations remain.

004BA390 borrows the actual collision descriptor/model, validates its Human/order
identity before using it, selects through required Players0045DD40, and binds
pointer78 to the actual Border WORLD-bound170 bytes. Ground projection writes
real5A7BB;004BAD50 retracts that clip on release.004BA660 preserves pixel
projection/64-bit-FISTP low-word conversion, all-layer normal ray, slope/distance
thresholds, anchor stores, direction-only kinds2/6/7 and following orientation
order updates.004BAD80/004BA360 preserve static/dynamic/order-model filtering,
last-regroup suppression and nested-roll publication.004BB210 has its distinct
release behavior; middle004BB230/004BB250 mutate the original gesture flag.
Native unwritten fourth vector lanes remain defined guards, not recovery credit.
Raw ASM resolves stock decompiler register/stack ambiguities. In particular,
drag distance retains extended deltas, forward normalization sums(Y²+Z²)+X²,
right-ground normalization retains Z through its stored-FLOAT product, and drag
normalization retains X through scaling. Unordered x87 branches are preserved.

004B7E50 retains held/key/wheel/edge priority, cooldown and pointer kinds62/63;
004B81A0 sends mode before axes to the SAME detached camera and preserves FIRST53
inversion timing. Camera simulation004632C0/004633A0 remains required external
behavior, not a second camera or scalar-success substitute.004AC090 filters only
FIRST52/53 using live bounds, leaving pointer position, SECOND and aliases alone.
Actual binding records remain57; command entry requires the original containing
64-word pair for indices57/62/63. No extra bindings or new polling were invented.

Auxiliary004AB610/004AB700 update actual order4A flags and existing selector
WORLD geometry. Roll004B2CA0 accepts only populated enabled rows and formats
only changed selection.004B2BE0 instead tests the cursor WORLD position strictly
inside the actual roll root WORLD bounds; it is not selected-row eligibility.
Both are bound to the existing nested dialog/text owner.

`show_command_panel` implements004B5E40 over SAME root/buttons/roster/selection
and Human visibility services. Entry publishes camera enable, resets the native
cooldown/gesture fields, caches actual current/previous Humans, prepares Sight,
requests realtime, updates policies/notifications, then reverse-filters scene
Humans.005A5D30 is Script AI suspension, distinct from Human150 visibility;
005A6380, health, kind3/4, roster and selected-record gates remain separate.
Exit hides actual Human-owned order models, clears soldier/nested-roll state,
requests realtime/camera mode0, disables notifications and restores eligible
Human visibility before binding mode2. Input mode3 is the final successful entry
write. Producer/engine errors propagate after original prior writes.

`show_command` implements0049F410 with actual HUD saved flags18, command-allowed
36 gate, FPV58 OR59 test, actual Player-mode005469E0 and00491FD0 enable owners,
timer placement, modal hiding, pointer and current-Human refresh order. It does
not enable application loading. The same verified58 OR59 query corrects the
existing occupied-actor crosshair visibility adapter.0049E1B0 now updates the
real down_panel and refreshes six soldiers in descending order, then hands,
signals, ordinary roll and current-Human compass policy. Reached004B47A0,
004A9730,004C10D0 and timer004AF900 bind existing text geometry; timer slots
refresh ascending and write inset0 after geometry.

### Producers and lifetime

Required `GameUiCommandInputSource` supplies actual Human count/records44/4A,
order-model identity,005825C0/0058C860 mutation, Players0045DD40 and camera
input. `GameUiCommandModalSource` additionally requires actual detached enable
00462AE0 and Players0045F080→Sight0046F280.0045F080 does not reset selection.
The original00486510 singleton is GEffects0089B770, not SOUND: vtable008101F0+44
resolves004870A0(false,0,true), restoring/releasing its special model2E0. That
operation is explicitly required from the render-owned model producer; no empty
callback, fictitious sound operation, or new effect owner substitutes for it.
004870A0 is absent from the frozen inventory and receives no credit.

Initialization still uses the existing World/HUD scene/77 inline TextStates,
command Border/buttons/textures, nested roll, actual roster, FPV, Items, fonts,
input and camera owners. New gesture storage contains only vectors/flags.
All order/model/target loans are consumed synchronously. Model loans must belong
to the actual Human order list; collision descriptors are checked before an
Actor cast. A small readonly frame getter exposes actual WORLD-bound bytes;
no copied persistent clip rectangle or engine-object std::array is constructed.

Retirement retracts the auxiliary Human loan by actual ActorBase identity and
clears a retiring previous Human's drag flag before B0/orders can die. This is a
defined lifetime guard, not extra original-function equivalence. Actor/Items/VM
ownership is unchanged. Auxiliary clip78 clears before HUD scene destruction;
command text/model/route/roll cleanup and binding mode2 retain existing order.
UI notifications retire before World destruction. Process overlays still use
the independent editfont/page registry and original cleanup; empty registries
do not bypass their initialization.

### Verification and remaining boundary

Native ASan/UBSan/LSan passes100 actual command/HUD77-text/pointer/order-selector
lifetimes,100 modal lifetimes,20 actual HUD scene/soldier/hands/dialog geometry
lifetimes, populated real-dialog row selection, all ten timer texts, input64/57
separation, order/mouse/camera policies, failure propagation and retirement.
Engine/font/Actor/order/camera/gameplay endpoints are explicit doubles; fonts
are TextState lifetime resources, not actual desktop glyph rendering. Strict
concrete i386 and sequential existing application link pass, without Windows
execution. A fresh60-file three-way source composition against pinned60f43dc
passes with the same three documented temporary resolutions: Car fuel getter
plus current Human seat services, both mesh-movie methods, and the full UI
injury movie receiver used by master damage wrappers. No automatic conflict-free
integration claim is made. No active merge/rebase occurred.

31 frozen client partials are added; complete34 and denominator18783 are unchanged.
General HUD0049C3C0 and reached ordinary message/selector/soldier controls remain
the next UI-owned family. Actual Human order/path/model preparation0058CCE0/
0058CAF0, conditional occupied actors, console execution and populated runtime
composition remain explicit. FRAME scheduling/realtime/camera execution, RENDER
scene/volume/special-model producers and OWN/LIFE activation/cleanup forwarding
stay with their blockers. No whole P22-UI closure, rendered frame or desktop
acceptance is claimed.

## General HUD control family — 2026-11-11

Source checkpoint after9ff6d0d; [receipt](checkpoints/p22-ui-general-controls-20261111.json).
Actual0049C3C0 is now callable as `GameUiPresentation::process_hud_controls`
over the SAME World/HUD/Players/FPV/Items/input/panels. It uses the existing real
map, info, command, inventory, signal, roll, pointer and selector workers; no
containing frame coordinator, menu or second World was introduced. Original
ordinary positive workers00496120/004961A0/004961C0,004ACF00,0049DFE0,
0049DCE0/0049DD60 and004AAD20 are implemented and connected. **P22-UI remains
OPEN** for the remaining presentation/producer/closure boundary below.

0049C3C0 preserves HUD-zero, cancel, pause/camera suppression, console/chat,
map/latch, info/latch, HUD4, offline selected-Human resume, signals, roll,
modifier/cycle selection, layout, selector, inventory, command, soldier clicks
and ordinary roll/signal/roster priorities. Closed gates do not require unrelated
producers.0049C2B0 performs all original modal cleanup even when returning false;
only an actually consumed cancel clears SECOND46.0049C110 visits the native
listed actions in descending64 order.0049C1D0 clears SECOND then FIRST at each
index63..0;0049C1A0 subsequently consumes scans0..9, not digit characters.
Caller-owned64-word buffers remain distinct from the actual57 binding records.
Actual BorrowedGraphicsInput handles the original Alt/Shift scans without
polling or owning another input device. Required-call errors propagate before
later input clearing, rather than turning into successful placeholder events.

Message00496120 writes actual field10/line index before offline realtime,
artwork and existing004961E0 input mode. Values0/1 retain their original paths;
other values do not invent a visibility action.004961A0/004961C0 preserve the
other say-prefix byte, refuse console mode1, and use the existing graphics text
queue/caret/input state. No new font or console parser is allocated.

Weapon selector004ACF00 uses actual selector records and actual Human cached
B0 item. Human virtual48 busy and actual60 action gates run before activation.
Empty-hand, move/swap/current-item and kind13 command2/23/focus policies use the
real selected item/holder, clear ONLY FIRST/SECOND19 after success, and close
through the actual selector owner. Held32/33 and edge navigation preserve their
separate200ms repeat timers, wrap policy and elapsed reset, including native
navigation after closing. `GameUiSelectorCommands` explicitly requires the real
Human busy query and item command/queue producer; no combat/item simulation or
successful empty command was added. Existing typed transfer commands are reused.

0049C200 wraps actual Players0045DC20 with current-index equality, roll/signal
hiding, visible inventory reopen around the actual newly current Human, and
current-Human refresh.0049DCE0/0049DD60 preserve tags100..105, distinct current/
command selection operations, and signed wrapping <300ms double-click behavior.
The process0089BB80 timer is now a scalar on the existing StartupSession-owned
HudClickSound beside0089BB7C; it survives HUD close/rebind and creates no owner.
004BF210 passes the actual previous Human and SAME detached camera to the
required00462780 producer. Players0045DA40/0045DC20/0045DCC0/0045D660/0045D9D0
remain actual external operations, not UI writes to roster indices.

0049DFE0 preserves8→10→20→8 priority, down_panel position before flag write,
actual selected game-mode3 carnage policy and nonoffline scoreboard style
visibility. Its tail reuses descending soldier/hands/signals/ordinary-roll
geometry, then actual status TextState004AAD20 and current-Human refresh. The
separate0049E1B0 tail deliberately does not gain that status update. Original
XYZ constants are copied from raw words. ASM resolves the decompiler's omitted
scoreboard visibility argument. HUD20/21 latch bytes remain on the real HUD and
are available to their later original runtime producers.

Selector controls add defined lifetime guards: actor retirement or any selected
record's Item retirement synchronously clears Human/ActorBase/Item loans and
owned record sprites/texts. The existing single Items UI notification now reaches
both inventory and selector. A pending hide runs against the still-live HUD
scene at the next input/update/draw; a failed hide retains that pending state.
All borrowers have already retracted before such an error. Discarding the HUD
scene drops pending state. Human/Items/VM ownership is preserved.

Native ASan/UBSan/LSan passes100 general-dispatch cases,100 actual message
font/state lifetimes,100 actual selector/text/sprite lifetimes and20 actual HUD
scene layout/lifetime cases, plus held-repeat expiry, no-transfer/current-item,
focused kind13, signed clock wrap, warming/nonoffline/paused gates and injected
realtime/artwork/command failures. Root panel/actor/gameplay/engine/font endpoints
are doubles; actual Messages, Selector, TextState and HudSceneLoading policy and
ownership bodies execute. This is not an actual populated World or desktop
render. Strict concrete i386 and sequential application link pass. Fresh61-file
compatibility against read-only447e073ed90f9fc6fb0fdd305fe6d725115d1786 passes
with the same three temporary resolutions; no active merge/rebase occurred.
Latest master adds actual console HWND/log/FIFO ownership, not00720130/00720340
command execution. Its newer Human/DATA work is not copied into this branch.
15 frozen client partials are added, with complete34/denominator18783 unchanged;
branch partial2611=2606client+5engine. Temporary native fixtures are fingerprinted
and removed, with original evidence retained under ignoredp22-ui-general-controls.

### Remaining source-closure boundary

The general UI-owned control checkbox is complete. The full UI composition is
callable over real owners and still not fully source-closed. Required Human
order/path/model preparation0058CCE0/0058CAF0 and real mutation/queues must be
bound and exercised with actual population. Conditional attached presentation
still has positive rejections: HUD hands004A9530 seat98/B4/A0/9C, FPV actorC4
and visibility8C; nonoffline soldier004B3890/frame status also needs completion
or an explicit researched P2.2 exclusion. These must not be relabeled merely as
unpopulated data. Existing typed focus/seat/network/console boundaries require
real producers. Newly available committed master implementations should be
checked before adding more source or interfaces. Full composed positive runtime
verification remains pending. FRAME scheduling/realtime/camera, RENDER special
model/scenes/volumes and OWN/LIFE activation/callback retirement remain external.
Application loading remains disabled. No UI closure or desktop frame is claimed.

## Occupied-Actor UI presentation — 2026-11-11

[Receipt](checkpoints/p22-ui-occupied-20261111.json). Previous checkpoint19f049b.
Read-only master83e77da320a256e64dd517e33154efdeeb566753 was inspected and fetched;
no reference worktree changes, branch merge/rebase or push. This supersedes the
attached-presentation rejection list above. **P22-UI remains OPEN.**

### Source receivers and original evidence

`ActorSeatPresentation<Actor>` is a typed UI receiving boundary, implemented by
`GameOccupiedActorPresentation<Dependencies,Frames,Actors>`. It borrows the SAME
containing World dependency aggregate, Scene2 identity registry, Car/StaticWeapon
loading lifetimes, actor storage, Items module and engine frame services. It owns
no Actor, weapon, item, camera, marker, scene or resource; no call retains those
borrowers. Invalid/unregistered Actors reject BEFORE type/frame/item reads.
Original Car table00810778 and StaticWeapon table00813130 establish the slots:

| Slot | Car receiver | StaticWeapon receiver | UI behavior |
| --- | --- | --- | --- |
|8C|004D9FC0|00604A10|Loaded Car seat flags; StaticWeapon first case-sensitive SEAT substring in actual marker userdata. Signed invalid seats return0.|
|98|004DA020|00604D20|Actual Car control presence; StaticWeapon seat==0 independently of seat count/failure.|
|9C|004DA040→004C6550|00604D30→004C6550|Actual item158 amount, FTOL lowDWORD; failed138 does not gate this query.|
|A0|00611140|00611140|Re-query98 thenB4, definition18 ammo ID, reverse ACTOR storage0 stacks; first matching FRONT definition returns stack count. No Human ammo fallback.|
|B4|004DA1D0→004C57B0|00604DA0→004C57B0|Failed138 returns null; otherwise SAME actual item158 definition.|
|C4|004DA090|00604DB0→004C6D90|Car actual seat view, including mounted receiver on master; ordinary VIEW%02u has visibility only. Static/mounted104/F4 gates, visibility then scene update.|
|C8|004DA270→004C7670|00604E10→004C7670|Actual E0/E4 publication; E0 zero is inactive, else signed current/max and caption1B9F.|
|CC|004DA2A0|00604E30|Actual geometry/loaded seat marker, signed bounds, case-sensitive NO3RD substring.|

004A9530 now queries actual occupied98. A controlled seat supplies B4 to the
existing hand panel, then calls A0 BEFORE9C and publishes counts with side1.
Raw assembly resolves the stock decompiler's misleading register temporaries:
the pushed1 is the later004A9420 side argument, not an additional A0 argument.
An uncontrolled seat follows ordinary Human hand3. BOTH paths then update actual
shoulder2 and kind9 holsters. Storage validation is deferred until the original
storage branch; a mounted hand is not skipped merely because it is attached.

005A9830 validates the same Human/Actor loan and original kinds4/20/21/34, calls98,
and RETURNS C8 directly for a controlled seat, including an inactive result. It
must not fall through to Human17C/180/184 or remote progress when C8 returnsfalse.
The ordinary Human timer/offline gate/required remote producer remain separate.
004C1AD0 explicitly clears controlE0/E4 (`param_1[38/39]` in its float-pointer
export). Added actual state words and initialize-time clears expose these reads;
UI neither advances reload nor fabricates a positive timer. Cold native004C19D0
has no new coverage claim for these words.

00491FD0 keeps its native mode/equality/phase gate, stores58, then invokes actual
C4 before Human/model visibility.0045DAC0 retains camera selection before Actor8C
bit10/FPV58/59 visibility.0049D530 keeps StaticWeapon FPV-before-CC even when FPV
already hides it, and Car CC-before-FPV with short-circuit suppression. Signal
seat-special98 now uses this same World facade; it is no longer a duplicate
required external callback. Target crosshair focus consumes the existing typed
`GameUiRollExternalSource::human_focus` in-call (or the bound signal source),
checks positive results in SAME registry and rejects missing typed publication.
It never casts Human's raw174 DWORD to a native pointer.

Original exports/ASM/tables are fingerprinted under ignored
`analysis/work/p22-ui-20261111/p22-ui-occupied`.004C57B0/004C6550/00604E30/005A9830
exports were reviewed against original bytes.00611140 and the Car/static tiny
wrappers are absent from the frozen census and receive zero credit. Seven
present bodies receive branch-local partials; four existing callers gain coverage.
Complete34 and frozen denominator18783 stay unchanged; partial2618=2613client+5engine.
These branch-local rows overlap newer master credit and must be reconciled by
entry when eventually integrated, never added twice.

### Exact ownership and compatibility boundary

World `Owners` constructs the stateless facade before FirstPlayers; all actual
resource/actor owners and the dependency aggregate are constructed before binding.
After binding existing Human services/presentation/factory/consumers, bind facade
to that completed aggregate, then FirstPlayers to facade, before script/input or
module activation. The facade remains callable without a GameUiPresentation
object, including initial FPV enable and teardown. UI/registered borrowed pages
still retire before their existing World owners as previously documented.

Retirement preserves existing order: script/VM consumers, FPV close, mission
camera clear, dubbing clear, actual Actor/Human close, HUD close, inputs/shared;
then FirstPlayers unbinds the facade, facade unbinds the aggregate, and existing
World dependencies unbind. Constructor failure retracts any established facade
binding. No new persistent control/Human/Items/VM owner or shutdown pass exists.

Latest master moved StaticWeapon's control state/resource lifetime into shared
MountedWeaponControlLoading and gives Car real unique control owners. The active
branch still has older opaque Car tokens; a positive token cannot be cast into
an owner and explicitly rejects mounted readout execution. Its ordinary Car view,
flags and cursor marker readers and actual StaticWeapon readers are connected.
The typed mounted branch instantiates against actual master control owners in a
fresh66-file temporary three-way composition based on31b01f9, with strict i386
OBJECT generation of the concrete facade/vtable and all receiver families.
This is meaningful source compatibility, not a claim of live Car execution.

Five overlaps require concrete integration resolutions: keep earlier verified
Car fuel getter with master seat services, both mesh-movie methods and UI full
injury receiver used by master damage wrappers; keep master's moved mounted
control struct with UI E0/E4 fields, initialize them in actual shared control
initialize; delegate StaticWeapon presentation_failed/view to control_.failed/
view. These resolutions are tested only in the temporary composition. No shared
Human B0/DD0/movement/integrity implementation was imported or rewritten.

Native ASan/UBSan/LSan executes100 repeated actual HudHandsLoading/TextObject/
ActorBase/storage lifetimes and actual FirstPersonPlayersLoading binding calls,
with explicit weapon/item/occupied dispatch/roster/engine doubles. Positive and
closed branches cover controlled/uncontrolled hands, shoulder/holsters, A0-before9C,
controlled inactive progress suppressing Human timer, positive/signed E0/E4,
actual actor reserve arrays, Car/static crosshair short-circuit order, case-sensitive
marker readers, binding/unbinding, unregistered actors/retired items, view failure
and texture/font initialization failure cleanup. Strict concrete active/master
i386 object generation and sequential existing application link pass. No original,
DLL, Windows fixture or Proton execution, actual populated World run or desktop
frame is claimed. Temporary fixtures are fingerprinted and removed.

### Remaining full checkbox and closure requirements

Latest committed master now implements the real sight_surface_factor from SAME
DD0 ground contact/material flags and process decorations.material_sight_factor;
UI already consumes that service. It is not duplicated on this older branch.
Human174 remains raw and lacks the typed producer on both branches. Actual Actor
families20/34 are absent from both factories; their presentation cannot be
replaced with default flags/items/timers. Car mounted data needs the existing
master owner plus the small documented UI interface integration. Actual mounted
reload publication remains a Human/weapon producer. These are explicit external
owner/data dependencies, not completed gameplay. The section6 checkbox stays open
until these contracts and composed positive execution are resolved.

Nonoffline soldier name/frame UI, actual Human order74/84 preparation0058CCE0/
0058CAF0 and mutation producers, console/chat00720130/00720340 execution and full
populated UI composition verification remain open. New console HWND/log/FIFO
work does not implement that command parser. FRAME schedules/camera simulation,
RENDER owns full scene/volumes/special-model rendering, OWN/LIFE activation and
callback lifetime stay external. GameApplicationState loading is still disabled.

## Nonoffline Human presentation and zone progress — 2026-11-11

[Receipt](checkpoints/p22-ui-peer-20261111.json). Follows occupied checkpoint87d92dc.
Latest fetched/read-only master05dbdee51cc366476317011b91b1a1863041dcee adds actual
application-owned network mission metadata/request source. No active merge/rebase,
reference worktree edit or push. Its Human/peer lifecycle work stays with Agent1.
**P22-UI remains OPEN; these UI presentation branches are implemented.**

### Reused data, names and rank

`human_hud_names.h` contains read-only UI adapters over the existing actual
ControlStartupState C4 records, StartupSession lock0/local-peer identity,
HumanStore/profile, Human kind/id10 and Actor name. GameOwnerBackend binds these
concrete methods; soldier/roll/crosshair/inventory/map/scoreboard consumers call
them. No copied peer table, second Human roster or name/font owner is introduced.
All returned names are immediate string copies; no peer buffer is retained.

**Correction to the previous handoff's research locator:**005A8380 returns peer
word54, not60. The original copies nine words40..60 under0072EC70/0072ED30, with
snapshot beginning ESP+08 and return load ESP+1C (difference14, index5). This is
confirmed in rank54.asm and discriminated by native data54=3/60=99. The implementation
copies all nine produced words under actual process lock0 and releases on normal
and exceptional exit, then returns snapshot[5]. Offline rank remains actual
profile24 or0 when absent. This corrects a note; the older source rejected the
nonoffline branch and did not publish the wrong field.

005A81B0/0072EBD0 read actual record08 name, with genuine empty result for invalid
signed/bounds ID.005A82C0 nonoffline kind1 uses that peer name, other kinds use
profile user14/SHORT10 or Actor virtual04/name.005A8320 target naming retains the
separate FULL0C/model fallback and original Actor80 caption owner. Roll's no-profile
short-name fallback is corrected to Actor name, rather than the model name.
005A8820 read-only UI team query uses existing Human-kind offline/remote-AI policy
and same actual nonremote record64 with00730530's invalid-ID fallback2. Reached UI
argument is0; this helper adds no alternate68 coverage.007304B0 local team reads
actual local peer64, rejecting absent/unproduced records rather than substituting
a Human team. These are data readers; Human AI/control/transport remain unchanged.
Newer master already has some equivalent damage/team readers; reconcile original
entries and share their actual data on integration, never sum duplicate ledger
credit or construct another process owner.

These readers replace nonoffline name/rank/team rejections and unnecessary
external callbacks in soldier, roll, target, inventory, map and scoreboard UI.
Map/info local-team calls now use the SAME process data directly. Required score,
objective/zone/speech and locked peer-table presentation sources remain distinct.
No transport, peer discovery or simulation is supplied by a successful callback.

### Soldier source004B3890/004B3EC0

004B3890 stores the rank cache before formatting. Offline names preserve previous
kind4/5/name-only and first rank-table behavior. Nonoffline style0 queries Human
storage74+10: this is pointer-array index4, **clothing**, not the hand. A present
clothing definition44==0 chooses27A6+rank; absent/other chooses27B0+rank. Other
styles query actual005A8820(0), team1 choosing27A6, others27B0. Localize rank
before acquiring the short name and write actual text record0/style3A. There is
no fabricated uniform/item/peer to force a positive selection.

004B3EC0 still performs rank/dirty inventory work before the dead latch. If live,
nonoffline optional mp_vip compares Human.id10 to actual0069E070: game71 true,
78!=-1 and SAME Scene2 command identity lookup00475D50, including pending reserved
slots. It is not the current/local peer. Health/stamina/photo colour and mode
updates remain in original order. Disguise005A6280 and player/AI/ear/distance tail
are offline-only. Visibility range/text uses offline7 OR actual config006B66B0
remote-AI predicate. The ordinary alive closed gate preserves existing visibility
text; death clears it unconditionally and only repopulates under that predicate,
then closes the dead latch. No B0/DD0 simulation is added to display these values.

### Human210 and actual zone progress

Human210 already exists as activation.field210; no Human state field or owner was
added.005A9920 now resolves it through the actual World Scene2 command registry,
requires kind37 and actual F1.005A9830 then resolves the identity AGAIN and005EC840
rechecks F1. Original progress math is unsigned DWORD wrapping: maximum=A8*1000;
current=maximum-EC-(state98==2?A8*500:0); caption=A0. Results retain signed display
bits; no timer simulation, clamping or duration default is invented.

Actual kind37 actors remain absent from the current/master factories. New typed
`GameUiZoneSource` requires their real F1/98/A0/A8/EC data, consumed in-call. Its
positive source must be provided by that Actor owner; a missing source throws.
Absent/wrong-kind identities and inactive F1 are genuine inactive results, and
do not require unrelated data. Pass this source to advance_hud_progress or via
GameUiHudFrameSources.zone. Existing Human timer, controlled-seat suppression and
offline gate remain unchanged. This implements the UI lookup/math receiver; it
does not claim a populated zone runtime or create a synthetic persistent owner.

### Verification and remaining source boundary

Native ASan/UBSan/LSan passes100 actual HudSoldierLoading/TextObject/ActorBase/
storage/ControlStartupRecord lifetimes. Explicit engine/Human/item/zone endpoints
are doubles. Cases discriminate rank54/60 and lock cleanup, clothing present/
absent/definition44 and team styles, actual peer bounds, profile/user/short names,
VIP selection, rank change after death, visibility preservation/death clear,
nonoffline disguise/tail suppression, offline range/ear work and texture/font/
visibility initialization failures. Actual session-word helper drives selected
identity cases. Zone policies cover two lookup/F1 checks, absent/wrong-kind/
inactive/vanished owners, ordinary/state2 arithmetic and DWORD wrap. The initial
fixture held a peer reference across vector growth; ASan caught that fixture bug.
Its borrow is now valid, and the final audit is clean. Product readers retain
no peer reference. Temporary fixtures/objects/scripts are fingerprinted and removed.

Strict active concrete i386 OBJECT generation and a fresh67-file temporary
latest-master composition pass, with the same five occupied-checkpoint resolutions.
The new master network metadata/request changes are retained. Existing application
compile/link runs sequentially and passes. Six new frozen client partials and ten
coverage updates; partial2624=2619client+5engine. Complete34/denominator18783 remain
unchanged. These counts are branch-local and overlap master entries. No original,
DLL, Windows fixture, Proton, actual populated World or desktop/game frame execution.

The nonoffline name/rank/team/progress UI classification checkbox is complete at
the source consumer level. Actual non7 peer3C spawn/configuration/retirement is
still open on master; process record readers do not replace that producer. Zone37
and Actor20/34 ownership, typed Human174 focus, actual Human order/path/model/
mutation and mounted reload producers, recorded master owner integration and
populated composed verification remain required. Console/chat00720130/00720340
execution still needs original ownership resolution and binding. FRAME/RENDER/
OWN scheduling/rendering/activation remain external. Application loading stays
disabled; no whole UI closure or desktop acceptance is claimed.


## Final UI source closure — after74220d4, 2026-11-12

[Closure](p22-ui-source-closure.md) records the complete current boundary and
initialization/retirement, external producers, actual positive verification and
limits. [Receipt](checkpoints/p22-ui-source-closure-20261112.json) fingerprints
this checkpoint. Earlier unchecked route/preparation and populated full-World
execution prerequisites are now explicitly split by ownership:004B9E80 UI
consumer and0049BD50 UI dispatcher are implemented;0058CCE0/0058CAF0 mutate
Human orders/path/model and remain P22-HUM. FRAME/OWN must supply populated
containing execution. They do not require another UI ownership architecture.

New concrete00720130/00720340/00720940 console source binds ordinary HUD editing
to the existing process descriptor/values, preserving prefix/parser/status/gates,
actual axis limits, happyammo and aliases.0045F020 reverse roster healing is
implemented as an action consumer;005A6060 remains Human-owned. Actual committed
master damage binds self-kill, and its existing reload consumer now reads the
SAME happyammo value. Chat/kill packet production and00742480 application/network
console belong to their original process/application owners, via required typed
actions. The visual console HWND/FIFO is a distinct owner and cannot substitute.
Chat108 has no initial writer; UI does not invent a recipient or queue payload.

004DDE20 now supports both original mode branches and fixes reversed observer
friendship in the command roll. Actor37 data faces bind map group/state/progress;
absent occupied Actor families can expose78/88/94 and seat presentation on the
actual Actor. No new Actor/gameplay owner or retained field/frame is introduced.

Scoreboard process scores and local peerB0/config270 now read real Startup data
under actual lock0, including failure cleanup. Info identities/name/stat40..60/
team64 also bind actual records;00725AC0 uses NULL3C and Player1/Remote7 byte70,
distinct from health/suppression. Latest master's kind12 team captions/side and
kind13 mission title bind actual application metadata. Active older layout uses
explicit legacy adapters. Four shared data lines exactly backport already
committed master declarations; no duplicate score registry or score simulation.

Native ASan/UBSan/LSan pass100 console/edit/input/roster lifetimes,100 Car query
cases,100 actual SceneActor/ActorBase/HudWorld zone/data-face lifetimes,100 real
process/configuration consumers on each old/master metadata layout, and100
composed HUD dispatcher→message/parser/input and scoreboard/TextObject/locked
score cases. Gameplay/Actor population/engine/World endpoints are doubles where
specified; no real desktop rendering is inferred. Resource and borrower failure
audits from preceding checkpoints remain valid; no broad unchanged-code reaudit.
Strict concrete active i386 and fresh76-file master05dbdee composition generate
objects; sequential existing application link passes. The five existing conflict
resolutions are unchanged, and new happyammo/data declarations merge without a
new conflict. Six new frozen partials, no complete promotion/census change.
Temporary fixtures are fingerprinted and removed after verification.
