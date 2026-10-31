# Menu family delivery checklists

This completes the inventory planning checkbox **source-only**. Credits and Cinematics are desktop accepted;27 delivery
items below remain open. The [coverage catalog](menu-family-delivery.json) maps
all 28 populated owner-scoped roles, twelve shared/auxiliary products, 71 routes,
512 control sites, thirteen dynamic categories and 20 prompt-helper sites.
[Identity rules](menu-definition-resolution.md) preserve all 30 declarations;
reserved hero slots 2/3 are not delivery targets. Planning was source-only.
The subsequent Credits implementation is desktop accepted2026-10-31; see [its contract](credits-flow.md).
[Cinematics is desktop accepted2026-10-31](cinematics-flow.md).

## Delivery and verification rules

Each checked item must include original evidence, real dependencies, native audits,
strict i386 build, compiled-client integration and actual user desktop acceptance
for the claimed environment. Fixture receivers establish payload/order/ownership
only; real game/network/media handoffs need actual receivers. Source-only or UI-only
acceptance stays separate. Preserve the accepted Main/Exit launch checkpoint.

Minimal Main entry wiring belongs to the selected flow; completing every Main
destination is not a prerequisite to implementing Credits or another individual
flow. The final Main traversal waits for its genuine endpoints. Do not expand
shared foundation without an original caller proving the selected flow needs it.

Profile/settings/save trials use isolated writable copies. No original game/DLL/
Windows fixture/Proton execution by the agent, no permanent new tests, and no
commit until requested. Interactive changes need user desktop confirmation.

## First delivery candidate

`media.credits`: Main Credits → status12 retained type10/sequence0 → scrolling
text/music → status13 restore/start of the saved owner. The complete flow is now
desktop accepted2026-10-31 after the user confirmed the requested checklist.
[Credits contract, resource correction and validation](credits-flow.md).

The earlier movie-file gate was incorrect:00688040 uses `credits.bik` as a selector
for text and opens no movie/voice. Real `Text/english/subtitles/credits.txt` and
ANSI/CE fonts are installed. The historical loose-movie absence is not a blocker.

## Main menu

Existing Main/Exit is accepted. Complete the nine-action hub without claiming other flows until their own delivery gates pass.

Primary numbered roles: `main:1`.

- [x] **main.labels — Recover live profile/save/date/version labels and original eligibility.**
  Evidence: `00623DD0`, `00626340`, `00626B20`.
  Requires: selected profile, stored profile save caption. After: `profiles.manage`.
  [Main acceptance](main-menu-state.md#desktop-acceptance): desktop accepted2026-10-31.
  Main caption parsing reuses the profile reader; broader save metadata gates remain open.
  Native: Audit show/update callbacks against actual current profile and save metadata; refresh after selection and absent save without fabricated values.
  Desktop: Verify labels and eligibility on entry, profile change and absent/valid last-save cases; preserve pointer/keyboard Exit.
  Failures/limits: Absent/malformed data follows original disable/error policy; unsupported destinations stay unavailable.

- [ ] **main.traversal — Complete all nine Main actions and return-to-Main behavior.**
  Evidence: `00623DD0`, `00626B20`, `0066C950`.
  Requires: all destination flows. After: `main.labels`, `media.cinematics`, `options.full`, `profiles.manage`, `saves.resume`, `single.entry`, `multi.browser`, `media.credits`, `multi.standalone`.
  Native: Audit the original event order and each exact target ID; maintain root Back/Exit modal semantics.
  Desktop: Traverse Resume, Profile, Single Player, LAN, Internet, Cinematics, Options, Credits and Exit, returning with original focus/visibility where applicable.
  Failures/limits: An unavailable network/game destination is explicitly excluded from full acceptance; no successful empty service completes the hub.

## Shared dialogs

Named confirmations and eight owner-scoped slot-zero instances share artwork but retain different protocols. Password/address dialogs belong to their actual multiplayer flows.

Primary numbered roles: `main:4`.

- [x] **dialogs.runtime — Integrate genuine runtime requests and preserve named confirmation semantics.**
  [Accepted through the delivered Options caller2026-10-31](runtime-dialog-integration.md).
  Evidence: `0066C1D0`, `00668790`, `006688A0`, `00668CD0`, `00669090`, `006693A0`.
  Requires: engine assets, real request context.
  Native: Use actual queue/stack/text/control code to audit modes 0/1/2/3 and unknown words, raw/localized payload lifetime, Escape/Yes/No/OK, reply order and force-close.
  Desktop: Verify compiled-client caller-to-dialog-to-reply with mouse/keyboard and underneath visibility/focus; retain accepted named Exit behavior. Diagnostic-only acceptance does not establish application caller integration.
  Failures/limits: Load/bind failure, busy-open guard, inactive/unknown sender and owner destruction preserve original behavior and native cleanup policy.

- [ ] **dialogs.callers — Validate every owner request/reply and context lifetime in its delivered flow.**
  Evidence: `0066D780`, `0066D7A0`, `0066D7C0`.
  Requires: delivered requesting flows. After: `dialogs.runtime`, `profiles.manage`, `options.full`, `multi.create`, `single.items`, `single.team`, `single.debrief`, `ingame.actions`, `ingame.player`.
  Native: Cover all 20 source helper sites, including raw text and mode 0/force-close, with live/rejected reply sequencing and retained callback contexts.
  Desktop: Exercise requests from the actual delivered profiles/options/network/setup/in-game flows; record unavailable environments separately.
  Failures/limits: No fixture-generated reply may stand in for a missing application operation; forced close and partial teardown must not use freed contexts.

## Credits and cinematics

Retained-owner status 12/13 sequence 0 differs from ordinary four-movie startup; selected-profile movie availability/persistence is real data. Either complete flow may supply the shared retained-video controller; the Credits resource is not a prerequisite for Cinematics.

Primary numbered roles: `main:7`.

- [x] **media.credits — Deliver Main Credits through retained video and return.**
  Evidence: `00626B20`, `006C20F0`, `0066EAE0`, `00688040`, `00688C10`.
  Requires: real Credits text/fonts, retained owner controller, menu music.
  Native: Actual script/font geometry and queue/controller lifecycle pass; restore precedes shutdown/delete/start. [Verification](credits-flow.md).
  Desktop: Activate Credits from compiled Main, complete and Escape/other original permitted input, verify audio/input/focus restoration and close during playback. Real scrolling text/fonts are required; the user confirmed the requested desktop checklist2026-10-31.
  Failures/limits: No Credits movie/voice is opened. Missing/empty script follows original empty-text completion; partial font/backend failures clean up the retained owner.

- [x] **media.cinematics — Deliver available-movie list, selected playback and return.**
  [Desktop accepted2026-10-31](cinematics-flow.md).
  Evidence: `00614140`, `006C20F0`, `006B1290`.
  Requires: selected profile, isolated profile copy, movie list, video audio, retained owner controller.
  Native: Audit generated available rows, selected filename, scrolling/selection, unlock/persistence effects and retained source screen/selection.
  Desktop: Enter from Main, select/play permitted movies, skip/finish and return to the same owner/list; Back returns Main.
  Failures/limits: Empty/unavailable list, absent movie, decode/open failure and profile-save failure retain real behavior; no hardcoded successful list.

## Options

Front-end Options pages, entry/apply/Back, binding capture/editors and persistence
are user desktop-accepted2026-10-31 after construction, ABI, text and rotated-parent corrections. [Current result](options-flow.md). In-game family2
remains separate and must not inherit family6-only observer matching accidentally.

Primary numbered roles: `main:6`, `ingame:2`.

- [x] **options.full — Deliver all front-end Options pages, binding editors and apply/Back.**
  Evidence: `0061BEA0`, `0061CF60`, `0061DBC0`, `0061E140`, `0061E390`, `0061F1D0`, `0061F350`, `0061F5A0`, `00620950`, `00620960`.
  Requires: engine assets, selected profile, isolated profile copy, settings input persistence. After: `dialogs.runtime`.
  Native: Compose actual factories/widgets/observers with all page definitions; audit aliases, visibility/focus, generated bindings, duplicate/cancel capture, key repeat/wheel, programmatic versus notifying writes and exact apply/queue order.
  Desktop: Enter from Main and exercise graphics, sound/music, Controls/mouse/bindings, game and network settings; verify conditional availability, previews, Back/apply, active-binding prompt and reopen/restart persistence on isolated copies.
  Failures/limits: Failed resources/settings writes, missing frames and cancelled capture clean up correctly; no enabled page/button may end in an unsupported service.

- [ ] **options.ingame — Deliver Options under retained in-game owner.**
  Evidence: `0067FB00`, `0061CF60`, `0061BEA0`, `0067F550`.
  Requires: real running game, retained game owner, isolated profile copy, settings input persistence. After: `options.full`, `ingame.pause`.
  Native: Audit family 2 target composition and explicit observer context/ID binding, -1/-2 entry and settings-first Back dispatch using actual settings code.
  Desktop: Open Options from a real compiled running-game menu, change/reopen settings and return to the correct game/menu without stale input or audio.
  Failures/limits: Game/session ownership cannot be replaced by a successful fixture owner in production; unavailable real game leaves full acceptance pending.

## Profiles

Reuse real startup profile selection/load/save and directory adapters; menu CRUD, validation and list synchronization are additional behavior. All persistence trials use isolated writable copies.

Primary numbered roles: `main:2`.

[Integrated profile implementation](profiles-flow.md): native flow/filesystem/edit/
staging audits pass. User accepted the corrected English profile flow2026-10-31;
[acceptance](profiles-flow.md#desktop-acceptance).

- [x] **profiles.select — Deliver profile list selection, labels and Back.**
  Evidence: `006211B0`, `006210B0`, `006211E0`, `00622A90`, `006AF1D0`.
  Requires: isolated profile copy, profile directories. After: `dialogs.runtime`.
  Native: Audit iterator-generated retained text, selected/current names, difficulty/description state, real selection/load/apply and cleanup ordering; include empty list and stale selection.
  Desktop: Enter Change Profile, navigate/select actual copied profiles, verify current profile/settings and Main labels after Back.
  Failures/limits: Aliased input, missing directory, failed load and empty selection keep original outcomes; never fabricate a default successful profile.

- [x] **profiles.manage — Deliver create/rename/delete, validation and confirmation.**
  Evidence: `00621470`, `006211E0`, `00622A90`.
  Requires: isolated profile copy, profile directories, edit list controls. After: `profiles.select`.
  Native: Audit actual edit/list callbacks, duplicate/invalid names, case/physical mapping, delete confirmation and directory/file failures; preserve original prompt codes and focus restoration.
  Desktop: Create, select, rename, cancel and delete only copied profiles; reopen/restart and compare persisted files plus error dialogs.
  Failures/limits: Original profiles remain untouched; partial filesystem failures must not report success or leave UI selection/context dangling.

## Saves and loading

No extra numbered save/load screen is inferred. Main Resume, mission-list last save and in-game/controller operations are explicit service boundaries. Profile.sav persistence is not a gameplay-save implementation.

Primary numbered roles: none inferred.

- [ ] **saves.metadata — Recover actual save metadata, labels and eligibility at existing entry points.**
  Evidence: `00626B20`, `006391E0`, `0067F550`.
  Requires: isolated profile copy, save format research.
  Native: Research original save/path/query callbacks before implementation; audit valid/missing/malformed/incompatible metadata and preview ownership through real readers.
  Desktop: Validate displayed names/time/preview and enabled state with isolated actual saves at each proven UI entry; do not create a guessed save browser.
  Failures/limits: Do not infer loadability from file existence or treat profile payload loading as a game-save result.

- [ ] **saves.resume — Deliver last-save activation through a real controller handoff.**
  Evidence: `00626B20`, `00638D30`, `0068CE00`, `006C20F0`.
  Requires: isolated profile copy, real game loading receiver. After: `saves.metadata`.
  Native: Audit selected path, validation, progress/error propagation and exact controller request on success/failure; native receiver traces prove only the interface.
  Desktop: Activate Main/mission/debrief last-save paths with actual compatible saves and verify the real receiving controller/game or explicitly leave that gate pending.
  Failures/limits: Missing/corrupt/version-mismatched save and cancellation must return through proven behavior; no synthetic successful load.

- [ ] **saves.ingame — Deliver proven in-game save/load/restart operations and return.**
  Evidence: `0067F550`, `00680680`, `006C20F0`.
  Requires: isolated profile copy, real running game, real game loading receiver. After: `saves.resume`, `ingame.pause`.
  Native: Recover each original eligibility/operation/status branch before wiring; audit request payloads, progress mode 0/force-close and error paths without inventing standalone UI.
  Desktop: Exercise permitted operations on an actual running compiled game with copied saves, including restart/cancel/failure and correct resumed owner.
  Failures/limits: Native operation counters or fixture data never establish successful gameplay save/restore; production settings/profiles remain isolated.

## Single player

Hero/team/equipment/debrief interfaces can be audited with actual decoded data and isolated profiles; real descriptor/briefing/game and post-mission transitions require their real receiving owners. Gameplay implementation is outside the menu milestone.

Primary numbered roles: `main:3`, `main:5`, `hero:1`, `hero:4`, `team:1`, `team:2`, `items:1`, `items:2`, `debrief:1`, `debrief:2`, `campaign:1`.

- [ ] **single.entry — Deliver Campaign/Lone Wolf/Carnage selection and mission selection.**
  Evidence: `0063B590`, `0063B740`, `00638D30`, `006391E0`, `00639A70`, `0063A010`.
  Requires: game data human store, isolated profile copy.
  Native: Audit mode/mission eligibility, generated campaign/mission groups, selection/preview and exact descriptor payload; distinguish slot5 from Cinematics slot7.
  Desktop: Enter all proven modes from Main, select permitted missions, Back and reopen; verify activation with actual decoded data and downstream transition when available.
  Failures/limits: Empty/locked campaigns, missing preview/data and invalid selection keep original gates; no fabricated available mission list.

- [ ] **single.hero — Deliver hero stats and named confirmation.**
  Evidence: `006971B0`, `00697600`, `00697710`, `00697A50`, `00696DC0`, `00697510`.
  Requires: game data human store, descriptor data. After: `single.entry`.
  Native: Audit specialization/stat updates, accept/Back confirmation and original HumanStore values/payloads; preserve empty slots 2/3 and slot 4 ID.
  Desktop: Traverse actual hero entry, change permitted values, accept or cancel confirmation and verify next/previous descriptor state.
  Failures/limits: No renumbering, empty-slot alias or silent default data; failed preparation stops progression.

- [ ] **single.team — Deliver squad/team roster, selection modal and description.**
  Evidence: `00691280`, `00691CB0`, `00691840`, `00693590`, `00691020`.
  Requires: game data human store, descriptor data, description assets. After: `single.hero`, `dialogs.runtime`.
  Native: Audit real roster constraints, conditional slot2/auto-assign, photo callbacks, medals/description and selection/confirmation return.
  Desktop: Use real decoded roster data to select/reassign soldiers, open/cancel selection, inspect description and verify forward/Back descriptor behavior.
  Failures/limits: Absent conditional slot, missing soldier/frame and invalid assignments retain gates; temporary visual fixtures do not prove campaign progression.

- [ ] **single.items — Deliver equipment/inventory/store, selection and validation.**
  Evidence: `00675980`, `006739A0`, `00674560`, `006763B0`, `00676D10`, `006761A0`, `006755F0`, `00677D00`, `00675F30`, `00673DD0`.
  Requires: game data human store, descriptor data, inventory services, description assets. After: `single.team`, `dialogs.runtime`.
  Native: Audit real item/category data, generated equipment slots, pick/drag/select/description, conditional modal, inventory validation, prompts and 020000c1/c6 preparation.
  Desktop: Equip/unequip permitted items, change soldier, inspect descriptions and exercise validation/confirmation then forward/Back with original data.
  Failures/limits: Reject invalid/over-capacity/unavailable items and resource failures according to recovered code; no dummy inventory/progression success.

- [ ] **single.briefing — Recover and validate descriptor-to-briefing/game handoff.**
  Evidence: `006C0EF0`, `006C20F0`, `0066C950`.
  Requires: real briefing receiver, real game entry receiver. After: `single.items`.
  Native: Research kind 5 briefing owner/interface and kind 7 receiving path before implementation; audit actual request data and cleanup/ownership at the boundary.
  Desktop: Verify actual receiving owner acknowledges entry and Back where original behavior allows; record absence as a blocked integration gate, not another guessed registry screen.
  Failures/limits: Fixture receivers can validate payload/order only. Do not expand into speculative gameplay or claim a successful handoff without a real receiver.

- [ ] **single.debrief — Deliver mission debrief, conditional diary and next/retry/return.**
  Evidence: `0068D2F0`, `0068EAD0`, `0068F3D0`, `0068E0F0`, `0068D5C0`, `0068CE00`.
  Requires: real mission result data, isolated profile copy, descriptor data. After: `single.briefing`, `dialogs.runtime`.
  Native: Audit actual results/soldier panels, diary gate, multipage text, prompts, save preparation and next/retry/Main-return requests; decoded captured result data tests UI independently.
  Desktop: Present real post-mission results, traverse diary/Back, continue/retry/return Main and verify persistence and receiving state.
  Failures/limits: Captured-result UI acceptance is separate from live post-mission acceptance; absent/corrupt results and failed saves cannot advance silently.

- [ ] **single.campaign — Deliver campaign debrief variants and sentinel failures.**
  Evidence: `0068BD10`, `0068BDD0`, `0068BDE0`, `0068C0F0`, `0068BD70`.
  Requires: campaign model resolution, real mission result data, descriptor data. After: `single.debrief`.
  Native: Audit all 20 table rows:11 model variants of one role, non-null empty load attempt, four selectable null failures and four unselectable padding rows; no fabricated Back for RET dispatcher.
  Desktop: Resolve actual layered models, present reachable campaign-result variants and verify real controller continuation; guard-admitted rows need actual reachability evidence.
  Failures/limits: Do not coerce empty to null, substitute a model or count variants as separate screens; unresolved resources leave visual acceptance pending.

## Multiplayer

UI contracts can use controlled server/session records for native audits. Functional LAN/create/join and any proven lobby/pre-game behavior need real local service/session endpoints; unavailable legacy Internet service remains explicitly unverified. Recover lobby ownership from existing owner/service routes before adding UI, rather than guessing another registry.

Primary numbered roles: `main:8`, `main:9`, `main:10`, `main:11`, `main:12`, `main:13`, `main:14`, `connection:1`, `connection:2`.

- [ ] **multi.browser — Deliver LAN/Internet browser with real refresh/selection/Back.**
  Evidence: `006333C0`, `00633F80`, `006344B0`.
  Requires: real local network service, server list controls.
  Native: Audit actual server-row population, refresh/filter/selection eligibility, expiry and LAN/Internet preparation against controlled records; fixtures never stand in for a production discovery service.
  Desktop: Use controlled local endpoints for LAN discovery/refresh and Back; verify Internet results only if the actual service works, otherwise retain explicit unavailable status.
  Failures/limits: Empty result, timeout, stale server and unavailable legacy service differ from success; no permanent canned server list.

- [ ] **multi.create — Deliver create-server settings, map selection and connection preparation.**
  Evidence: `00628490`, `00628E00`, `006333C0`.
  Requires: game data human store, real local network service, isolated profile copy. After: `multi.browser`, `dialogs.runtime`.
  Native: Audit original map/category rows, settings validation, preparation prompts/replies and connection requests with real map metadata.
  Desktop: Choose valid maps/settings, create a controlled local session, cancel or continue into the real connection owner.
  Failures/limits: Invalid settings/map, bind failure, service error and prompt cancellation preserve original cleanup and return behavior.

- [ ] **multi.filters — Deliver server filters and selected game information.**
  Evidence: `00638590`, `0062F3E0`, `00633F80`, `006333C0`.
  Requires: server records, real local network service. After: `multi.browser`.
  Native: Audit filter state and resulting visibility, information binding lifetime and Join eligibility while server data changes.
  Desktop: Change/reopen filters, inspect a real selected server, return to browser and Join only when available.
  Failures/limits: Disappearing/stale records cannot retain dangling callbacks or join a different server silently.

- [ ] **multi.addserver — Deliver Add Server address/port editing and cancellation.**
  Evidence: `00636FF0`, `00637440`, `00626B20`.
  Requires: edit controls, real local network service. After: `multi.browser`.
  Native: Audit two visible fields/defaults, hidden password-model binput, actual validation/add callback and discard-before-pop.
  Desktop: Add a controlled endpoint, test invalid input and cancel/Back, then verify the real browser update.
  Failures/limits: Address/port rejection and network failure are real outcomes; do not reuse password callbacks or silently accept invalid input.

- [ ] **multi.join — Deliver ordinary Join/password/connection console and lobby/game handoff.**
  Evidence: `006333C0`, `00637990`, `006377C0`, `00637970`, `00632080`, `006322C0`, `00632490`.
  Requires: real local network service, real network game receiver. After: `multi.create`, `multi.filters`, `multi.addserver`.
  Native: Trace actual lobby/pre-game session ownership from original status 9/session callers before coding; no standalone lobby registration is inferred. Audit selected server copy, password+82 predicate, modal10 entry/focus, cancellation/discard and console lifecycle; success requires the actual receiving session/game owner.
  Desktop: Join controlled protected/unprotected local sessions through real callbacks, test wrong password/cancel/disconnect and verify the real receiving owner.
  Failures/limits: Wrong/stale password, timeout and connection rejection must clean up without a synthetic connected state.

- [ ] **multi.standalone — Deliver standalone connection console/regular password and return.**
  Evidence: `0068A460`, `0068A710`, `0068A990`, `0068AD20`, `0068AED0`, `006C20F0`.
  Requires: real local network service, real network game receiver. After: `multi.join`.
  Native: Audit controller status 7/8 session modes, timer state5 regular slot2, edit callbacks, Exit Game action and actual status 9 handoff; preserve distinction from ordinary modal10.
  Desktop: Enter both real controller connection modes; exercise password success/failure, Back/Exit and receiving state.
  Failures/limits: No modal substitution, owner merge or fake session success; missing external service leaves functional gate pending.

## In-game menu

A standalone native audit can inspect the retained-owner protocol and real UI builders using supplied data. Full pause/resume/session/save acceptance requires an actual running compiled game owner; no successful placeholder game.

Primary numbered roles: `ingame:1`, `ingame:3`.

- [ ] **ingame.pause — Deliver retained pause-menu entry and ordinary resume.**
  Evidence: `0069E620`, `0067F350`, `0067FB00`, `00680680`, `0067F550`, `006C20F0`.
  Requires: real running game, retained game owner, ingame model resolution.
  Native: Audit -1/-2 entries, mode guards, retained+5b8 ownership, conditional products, queue/input/audio/pause order and status 15 restoration using actual code plus trace-only receiver fixtures.
  Desktop: Open from a real running compiled game, navigate allowed actions and Back/resume repeatedly; verify the same real game owner, input/audio and teardown.
  Failures/limits: Fixtures prove protocol only; absent running-game integration or main_ingame resource cannot pass full delivery.

- [ ] **ingame.player — Deliver player/equipment/soldiers, description and information panel.**
  Evidence: `00682980`, `006835E0`, `00684390`, `00684890`, `0067A770`, `006835A0`, `00496450`, `00496E40`, `0067F550`.
  Requires: real running game, real session player data, ingame model resolution, inventory services. After: `ingame.pause`, `single.items`.
  Native: Audit both player models, conditional roster/store/description, session actions and info-panel first-Back dismissal; shared templates retain owner-scoped IDs.
  Desktop: Use actual permitted session modes to enter player UI, change allowed selections, show/hide info/description and return correctly for -1/-2 entries.
  Failures/limits: No fixed fake roster/session, model substitution or hidden-panel pop; unavailable modes remain outside live acceptance.

- [ ] **ingame.actions — Deliver original guarded game-action confirmations and maintenance returns.**
  Evidence: `0067F550`, `006827A0`, `006C20F0`.
  Requires: real running game, real game action receivers. After: `ingame.pause`, `ingame.player`, `saves.ingame`, `options.ingame`.
  Native: Recover each b5/b6/b7/b8/c5/c6 operation and eligibility from original code before wiring; audit prompts, status 15/16 and underlying status words, progress/force-close and ordered unwrap.
  Desktop: Exercise permitted real game/session actions, cancel/confirm, maintenance and return-to-front-end; validate actual receiving owners and preserved copied persistence.
  Failures/limits: Never infer an operation from its label alone or substitute an acknowledged no-op; game-action fixtures cannot establish full acceptance.

## Coverage and acceptance boundaries

The machine catalog assigns one primary family to every populated numbered role
and shared/auxiliary product. Cross-family route/control dependencies are retained
rather than counted as new screens. All 71 route records retain original endpoints
and evidence; all 512 source control sites and20 shared prompt sites are mapped.
The unparented ordinary item-template hook remains an explicit invocation/data
research dependency, not an invented prerequisite for unrelated flows.

Actual game requirements are explicit for pause/resume, player/session actions,
in-game settings, save/load/restart and receiving descriptor/briefing/result paths.
Native fixtures with genuine decoded GameData/HumanStore/copied profile data can
audit setup, inventories, results, conditional pages and boundary calls without
running a game. They do not establish game/session progress or live return.

Every family must also preserve captions/layout/styles, original input/focus/
queue timing, sound, partial-failure cleanup and ownership. Global fidelity and
full traversal remain plan section 4; they do not replace per-flow verification.

This planning milestone audits catalog coverage, dependency closure/order and
real environment distinctions; existing native/tooling/evidence/ledger/build
checks remain applicable. No C++ changed, so no new sanitizer fixture is needed.
No new delivery, desktop acceptance or body credit is claimed.

Verification passes the complete coverage/acyclic-dependency audit and thirteen
damaged-catalog cases. The existing native suite/all 158 tooling tests, related
original evidence maps, frozen ledger and strict Windows target checks pass.
Built/staged application SHA256 remains
`5137074b01e40adb695b66dcf11b3f8bed0e2245bd8f03a27800a9d404e553eb`.
Verification logs
are ignored under `analysis/work/menu-family-verification-20261031`.
