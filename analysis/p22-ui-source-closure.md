# P22-UI source closure — 2026-11-12

**P22-UI is source-closed on `feature/p22-ui`.** The UI-owned source receivers,
positive presentation policies and concrete World adapter are implemented and
connected. Remaining populated execution requires producers/coordinators owned
by other P22 parts. This follows the user's explicit completion boundary:
remaining work performed by those parts does not keep the UI source task open.
It does **not** mean application activation, a complete session frame, real
engine rendering or desktop acceptance. No loading path was enabled.

[Final receipt](checkpoints/p22-ui-source-closure-20261112.json),
[implementation history](p22-ui-implementation.md),
[delivery checklist](p22-ui-plan.md). Reference research remains on
`research/p22`; its implementation observations are historical. Initial inspected
master was `799a79305c70bc8de1860ac7e438bba25a71a8ac`; final inspected master is
`05dbdee51cc366476317011b91b1a1863041dcee`. No reference branch/worktree mutation,
active merge/rebase, push or original/Windows execution occurred.

## Concrete callable boundary

`host/game_ui_presentation.h` supplies `GameUiPresentation(World&, Parent&,
GameOwnerBackend&, Graphics&)` and `GameUiSceneCallbacks`. The constructor checks
SAME SessionOwner/backend/Startup/device/scene/request; methods reject retired
World owners. It allocates no World, roster, Human, camera, HUD, font, SOUND or
Items substitute. The adapter borrows its World and must die before World close.

| Original service | Implemented, connected UI source |
| --- | --- |
| `0049B9B0/0049B9F0` | Actual HUD route prepass and render orchestration, camera/no-GI/visibility gates, original repeated map/info drawing and command geometry. |
| `0049BD50` | `advance_hud`: full offline/nonoffline update order over real loaded panels, selected/current Human, actual session/configuration/time inputs. Timers, message editing, soldier/selection/hands/crosshair/progress, mission/objectives, map, info, scoreboard, inventory, command/signals and vehicle consumers are connected. |
| `004909D0` | Existing FPV **activation**, distinct from frame tick. Actual FPV owner consumes same Players/Human/Items/camera/scene. `advance_first_person`, animation completion and opacity endpoints bind `00491660/00491C60/00491E90/00491E30`; camera/world simulation stays outside UI. |
| `0048F2D0/0048F390` | Actual mission text tick/draw, preference and graphics-state handling. |
| `006E8490` | Independent process editfont texture/matrix/glyph resources, registered-page lifetime, batching/reversal, visibility/status gates and raw driver-state restoration. Startup initializes even with no registered pages; shutdown releases before driver. |
| `006C9260` | Stage1 HUD prepass → required render pre-scene → optional debug; stage2 HUD/full viewport → mission text → genuine network RET → session fade → actual effects fade → process text. Status14 and error propagation preserved. |
| Required UI controls | Real general, modal, command, map, roll, signals, selector and inventory policy; original input bindings/64-word views. Gameplay, camera, realtime and packet actions use required typed producers. |
| `00720130/00720340` | Real bounded console parser over the existing process descriptor/value owner, connected by default to actual HUD editing/submission. Axis sensitivity and process variables/happyammo are concrete; original item/health/kill/chat policies require their real action owners. |

The supported positive source paths were executed in native sanitizer audits,
not just compiled. Prior receipts cover real HUD/text/FPV/overlay/panel bodies,
resources, order, visibility/status gates and failures. The final composed audit
executes `0049BD50` into actual HUD edit/parser/input and `004AED20`/TextObject/
locked process-score bodies for 100 lifetimes. Other panel/World/gameplay/engine
endpoints in that audit are doubles. It is not a populated Windows World test.

## Final data and action bindings

Nonoffline names/rank/team use actual C4 process records. Human rank `005A8380`
is word54; info-table rank is separately word60. Soldier clothing/profile/VIP/
death/visibility policy uses real existing owners. Scoreboard scores now read
actual process `team_scores44c`; local losses read peerB0 and SAME config270
under actual Startup lock0, released on failure and on limit-1 too. The four-line
ControlStartupState field addition exactly backports declarations already on
master; it creates no score registry or score producer. Info names/stat40..60/
team64/identities and sorting likewise use the actual locked record vector.
`00725AC0` is NULL peer3C → unavailable, otherwise Player1/Remote7 byte70;
health and mode-gated Human suppression remain distinct.

Latest master's actual kind12 objective `18/1C/20` and kind13 mission `6C`
metadata now feed map/info directly, without another descriptor owner. Older
branch data layouts require explicit legacy adapters, rather than treating
unrelated offline fields as network metadata. The modern bodies instantiate
and the actual master metadata types execute in native sanitizer tests.

Car `004DDE20` is shared by crosshair/roll. Offline friendship receives
**observer first**, before health; the earlier reversed roll argument was fixed.
Nonoffline health/control/suppression/team checks preserve order. Same-World
occupied presentation binds actual Car/StaticWeapon seat flags, hands/item
amounts, view and reload progress; latest master supplies real mounted owners.
Actor37 progress/group/state and absent occupied-family78/88/94 data faces live
on their actual Actor owners, with no cached actor/frame loan or second World.
Future owners implement these typed faces; the UI policy needs no replacement.

Console dictionary keys are case-sensitive; 54 item aliases are insensitive.
The original second-byte comma prefix is preserved. Integer base prefixes,
quote trimming, status codes, 99-byte chat clip, reverse healing, fresh Human/
ammo queries and echo-before-dispatch/failure-buffer retention are verified.
`anorexy` and retail script output are genuine original no-op bodies, not new
successful placeholders. Integer overflow, unterminated buffers and the native
one-character quote underwrite are guarded; no generic CRT/lexer/tree equivalence is
claimed. The one shared Human-service change passes the SAME process happyammo
value into its existing reload service. On master, self-kill binds the actual
Human damage service; no health/combat implementation was duplicated.

## Initialization and retirement

1. Existing Startup/input/SOUND/device/services initialize the process overlay's
   independent editfont/matrix/glyph resources; failures unwind before device.
2. Existing SessionOwner constructs scene/text/HUD parents. Existing World owns
   Players, Humans/Items, camera, FPV and panels; no UI replacement owners.
3. World binds dependency services and occupied presentation, then binds that
   facade to FirstPlayers **before** activation. Panels bind to the actual HUD
   scene/fonts/artwork, with map zones from actual Actor-owned data faces.
4. After World readiness, the containing owner constructs GameUiPresentation,
   then GameUiSceneCallbacks. All allocations/prechecks precede nonthrowing
   retirement-callback publication. FRAME/RENDER install and invoke borrowers.
5. Stop/uninstall containing callbacks and destroy the UI adapter before World
   close. This unbinds item/dialog/inventory refresh and required source loans.
   Original actor retirement retracts selector/order/inventory/map/crosshair/
   dialog loans before their actors/items die. Registered borrowed pages must
   be removed before page owners; managed page handles do this automatically.
6. Existing ordered session finish preserves VM/script, FPV, camera, dubbing,
   Actors/Human/Items and HUD retirement. The stateless occupied facade remains
   bound through those consumers and unbinds last. World precedes parent scene/
   text/fonts. Process overlay/pages/wrappers/texture retire before driver.

No new persistent actor, frame, camera, font, texture or scene borrower is added
by the final parser/process/data-face changes. External action/data sources are
in-call, except explicitly bound signal-source borrowers which must unbind
before their producer lifetime ends.

## Remaining work belongs to other P22 parts

| Owner | Concrete required work; no successful fallback |
| --- | --- |
| P22-HUM / Player / Items | Typed Human174 and peer3C with actual lifetime; order change/count/records/mutable44; `0058CCE0/0058CAF0` order/path/model preparation; `005AC470` Human/weapon presentation; actual selection/action/transfer/queue/Sight receivers; health setter `005A6060`, live item creation `007D7930`, mounted reload publication and Human214 speech. Master damage producer is bound where available. |
| Actor/OWN population | Absent kind7/20/23/34/37 owners supply actual data/virtuals; Car254A/`004DDD10` eligibility; peer3C membership, process address00981E70 and team counts97D4C8/CC. UI supplies the formatting/policy/checked typed receiving boundary. Cold opaque Car tokens on this older branch cannot substitute for master's mounted owners. |
| P22-PACK / application services | Actual chat `0072EA90` durable payload/shared queues, application/scalar kill requests and their delivery/retirement. Chat target108 is unwritten until its actual application consumer produces it. Unknown-command `00742480` belongs to process98C0FC's network/application console, distinct from visual HWND/FIFO and menu text. UI does not allocate a second owner or fake these actions. |
| P22-FRAME | Call completed HUD/FPV/text/control endpoints in original session/module order with actual time/input/current Human and mission camera. Supply realtime/camera producers; execute populated integration. |
| P22-RENDER | Actual full scene0069B870, volume00482260/procedural00482E50, pre-scene/debug/special-model and general presentation. UI callbacks already preserve stage/order/state contracts. |
| P22-OWN / LIFE | Install adapters/callbacks under actual activation; loaded/status forwarding, failure rollback, finish/unload and borrower-before-owner retirement. No application loading activation was enabled here. |
| Desktop acceptance | Actual original mission populations, rendered scene/camera/HUD/FPV/text/fades, failure return/repeat and second mission. Entirely pending; engine doubles are not desktop rendering. |

Consequently original orchestration functions remain **partial** in the frozen
ledger: native typed/STL/RAII ABI, missing external gameplay/packet/Actor/render
producers and unobserved containing execution do not justify whole-function
completion. Six newly implemented frozen bodies receive partial credit only;
no census change or complete promotion. Strict concrete i386 object generation
and sequential existing application link pass. A fresh 76-file composition over
master passes with the five documented existing integration resolutions; this
is not a conflict-free merge claim. See the receipt for exact hashes and limits.

## Master integration checkpoint — 2026-11-12

Resolved the21 reported conflicts with master `949282e03af00145ece4ec44dc06d282356d0def`
and UI `ebdcbf59bc6a59880bb664c0a5934e46105669d6`. Master's main HANDOFF is preserved
byte-for-byte. The Human dependency checklist and independent critical-path research
remain; the plan now points at the completed UI source boundary.

The combined source preserves master Car fuel/optional-clock/reaction and shared
mounted-control owners. UI reload E0/E4 state and explicit initialization live in
that shared control; display getters delegate to it. Both Human process query
interfaces remain available. Master's actual Zone37 map field readers and
Human death/objective/peer refresh consumers coexist with UI's full frame policies.
The older statements about absent Zone37/peer owners describe the original UI
checkout; they are not claims about this combined master source. Runtime UI data
faces and action producers still need their containing callers.

Auto-merged duplicate map/roll methods were removed. Master's signal-hide path
now clears UI's actual typed64/68 borrowers before selection/visibility updates.
Injury update/fade retains master's active-movie accessor and CRT validation.
The concrete i386 UI compile also exposed0051A880 passing a four-lane native
wheel vector to the actual XYZ engine position interface; it now passes XYZ only.

The deduplicated ledger union is **34 complete /2,997 partial** (2,992 client and
five engine partials), with240 additional original identities relative to master.
The24 shared changed records retain both evidence sets and count once. No
status promotion, new function credit or frozen inventory change comes from the
merge. README is regenerated by the existing tracker; all27 tracker tests pass.

Strict concrete World/UI i386 object generation and sequential launcher/probe
compile/link pass. A focused native ASan/UBSan/LSan audit passes100 lifetimes for
the merged movie/retirement, signal-loan cleanup, peer rank/name/lock failure,
mounted progress/view, pointer mutation-prefix and sound XYZ/gate policies.
Engine/action endpoints are doubles. No Windows/original execution, application
loading activation or desktop acceptance is claimed. Temporary audits are removed
after fingerprinting; retained logs stay in ignored WORK.
[Merge verification receipt](checkpoints/p22-ui-master-merge-20261112.json).
