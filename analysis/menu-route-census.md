# Frontend owners, dynamic products and reachable routes

The complete owner/construction census is recorded in
[the catalog](menu-route-census.json), with [original byte evidence](menu-route-census-map.json).
This closes the second inventory checkbox **source-only**. It adds no runtime
screen, original function-body credit or desktop acceptance. Main/Exit and the
built application remain unchanged. The earlier [fourteen static registrations](menu-screens.md)
are reused, with their original identifiers and builders.

## Completion boundary and identity

The original base client contains **eight concrete owners and thirty numbered
registration candidates**, including conditional declarations. Each concrete
owner also constructs the shared runtime dialog. Three description overlay
construction sites and one engine information panel sit outside those numbered
registries. Twenty-three direct constant-kind-five construction sites and
thirteen dynamic product categories identify embedded pages, lists and templates.
None of these totals is a count of distinct finished screen designs.

Identity is **owner plus slot**. A numbered definition's identifier is
`slot << 26`; the same identifier belongs to different registries. Main Options
is family six, in-game Options is family two. A model name is a resource key,
not an identity. Hero slots two/three/four all declare `CONFIRM/confirm`, but only
four has the confirmation builder and an established incoming UI route. The
[completed resolution](menu-definition-resolution.md) classifies slots two/three
as reserved empty registrations, preserving their IDs. All thirty declarations
remain; twenty-eight have populated construction contracts.

The census includes construction, conditional availability, incoming navigation,
shared stack/dialog operations, derived dispatchers and controller/service
boundaries. The [following inventory checkbox](menu-contracts.md) now records
per-control IDs, assets, captions, initialization, dependencies and return contracts.
Family implementation
and desktop traversal remain separate work. Controller transitions identify
required services; their presence does not establish a functioning game/network
handoff in the reconstruction.

## Owners and numbered candidates

All eight share init `0066AD10`, shutdown `0066B140`, input `0066B3E0`, fixed/update
`0066BE90/0066BEA0` and render `0066DE00`. The table identifies content at virtual
`+38` and derived events at `+34`. `006C0EF0` replaces the active owner using an
original descriptor. Its kind-six subtype selects five of the derived owners;
subtypes one/two and three/four share owners, rather than creating four registries.

| Owner / type | Vtable | Content / events | Numbered declarations | Entry |
| --- | --- | --- | --- | --- |
| Ordinary / 2 | `00815E30` | `00627060` / `00626B20` | 1–14, existing inventory | Null descriptor; return-to-menu/controller paths |
| Retained in-game / 11 | `00815750` | `0067FB00` / `0067F550` | 1 main, 2 Options, conditional 3 player | `0069E620`, controller statuses 14/17 and session callers |
| Hero statistics / 9 | `00815C30` | `006971B0` / `00696DC0` | 1 statistics, 2/3/4 confirmation candidates | Descriptor kind 6, subtype 0 |
| Campaign debrief / 14 | `00815C90` | `0068BDE0` / `0068BD70` | 1 dynamic campaign model | Kind 6, subtype 6 |
| Mission debrief / 5 | `00815CF0` | `0068D2F0` / `0068CE00` | 1 debrief, conditional 2 diary | Kind 6, subtype 5 |
| Team selection / 3 | `00815D50` | `00691280` / `00691020` | 1 team, conditional 2 selection | Kind 6, subtypes 1/2 |
| Item selection / 4 | `00815DB0` | `00675980` / `00673DD0` | 1 equipment, 2 selection | Kind 6, subtypes 3/4 |
| Standalone connection / 8 | `00815E98` | `0068A460` / `0068A710` | 1 console, 2 password | Controller status 7/8; session mode 2/1 |

`008157D8` and `00815850` are intermediate abstract tables. Their derived
content/event entries are purecall `007E5A35`; constructor writes to those tables
do not establish additional concrete screens. The same table scan independently
matches both common init and render pointers.

The remaining sixteen numbered declarations are:

| Owner:slot | Definition / model | Augmentation |
| --- | --- | --- |
| ingame:1 | `MAIN INGAME MENU` / `main_ingame` | `00680680` |
| ingame:2 | `OPTIONS` / `options` | `0061CF60` |
| ingame:3 | `PLAYER SCREEN` / `player_screen` or `player_screen_2` | `00682980` |
| hero:1 | `HERO STATS` / `heros_screen` | `00697600` |
| hero:2 | `CONFIRM` / `confirm` | Reserved empty registration; original ID retained |
| hero:3 | `CONFIRM` / `confirm` | Reserved empty registration; original ID retained |
| hero:4 | `CONFIRM` / `confirm` | `00697510` |
| campaign:1 | `Campaign_debrief` / selected model | `0068C0F0` |
| debrief:1 | `debriefing` / `debriefing` | `0068EAD0`, `0068F3D0` |
| debrief:2 | `DIARY` / `diary` | `0068D5C0` |
| team:1 | `team members selection` / same | `00691CB0` |
| team:2 | `SELECTION SCREEN` / `selection screen` | `00691840` |
| items:1 | `Items selection` / `items selection` | `006739A0`, `00674560`, `006763B0`, `00676D10`, `006761A0`, `006755F0`; data initialization `00677D00` |
| items:2 | `SELECTION SCREEN` / `selection screen` | `00675F30` |
| connection:1 | `MP Connect console` / `confirm` | `0068A990` |
| connection:2 | `PASSWORD` / `password` | `0068AD20` |

Construction is conditional on actual resource/binding success; declaration
counts do not guarantee that a product exists. The catalog preserves these gates:

- In-game slot three exists only when `(00982100 & 7) != 7`. `006B66B0` chooses
  `player_screen_2` when true and `player_screen` otherwise. Initial slot is three
  for that branch with owner `+F0 == -2`, otherwise one.
- Diary exists only when `0068C2E0` returns true: `*(008AE924 + 4) == 0`.
- Team selection slot two needs `006B8F90(profile+E8) == 0` and
  `*(008AE924 + 8) != 2`. Its auto-assign button adds a narrower profile-zero gate
  before binding modal selection in `00693590`.
- Campaign model getter `0068BDD0` reads word zero of the record selected by
  `0068BD10`. For profile campaign 0/1 and selector result 0–7 it selects
  `00841668 + (result + campaign*10)*24`; otherwise record zero. The catalog
  records all sixteen selectable rows: eleven nonempty models, one empty model
  pointer and four null pointers. Rows 8/9/18/19 are outside this selector's range.
  Null aborts content creation; an empty string is preserved as a different case.
  The [resolved table](menu-definition-resolution.md#campaign-sentinels-and-variants)
  also classifies all four unselectable zero-padding rows. These records do not
  add numbered screen roles.

## Shared, embedded and auxiliary products

Common setup `0066C5B0` creates the cursor and `0066C1D0` runtime dialog before
calling derived content. Slot zero uses `confirm` but has its own definition and
request state. Its push/dismiss modes, keyboard/reply and force-close behavior
reuse the [confirmed runtime contract](menu-runtime-confirmation.md). The catalog
records all eighteen direct `0066D7A0` prompt-helper sites: profile operations,
multiplayer creation, Options, equipment validation, team/debrief navigation and
in-game requests. Prompt requests do not introduce another numbered registry.

Team `00691280`, items `00675980` and the `006B66B0`-true player branch of
`00682980` load `DESCRIPTION/description`, bind a null definition and store an
owned overlay at frontend `+FC`. Team writes table `00815918`; items/player use
`008156B8` (player constructor `006835A0`). These are separate owner instances,
with selection-driven content and shared update/render handling.

In-game init `0067F350` also creates the `+4BC` information panel when the mode
gate is not seven. `00496450` loads `ingame_info` and finds root `game_info`;
`00496E40` changes visibility. `0067F550` Back hides this visible panel before
attempting Options/player/pop/game-return routing. It is an engine/model panel,
not a registered `Screen` subclass.

The catalog keeps every direct constant-kind-five site, including repeated
conditional/template sites, under its real parent:

| Parent family | Embedded groups/templates | Construction evidence |
| --- | --- | --- |
| Both Options owners | `GROptions`, `graphics setup`, `sound setup`, `keysscreen`, `keysscreen01`, `gameoptions`, `network settings` | `0061CF60`, `0061DBC0`, `0061E140`, `0061E390`, `0061F1D0`, `0061F350` |
| Create server | `MP_MAPS_PANEL` | `00628E00` |
| Equipment and player | `P_Equipment`, conditional `scroll13` sites and `scroll14` | `0066EF00`, reached from `00674560` / `00684390` |
| Equipment store | `P_visualstore`, `Panel02` | `0067C270`, `006761A0` |
| Player | `P_visualstore`, `soldierspanel`, `Panel02` | `0067A770`, `006835E0`, `00684AF0` |
| Debrief | `panel_soldier00` template | `0068E0F0` |
| Team | `Panel01`, `medals_panel`, `Panel02`, `P_SelectedSol`, `Panel03` | `00691E10`, `00692950`, `00693030`, `00693270`, `00693200` |

Options navigation switches groups within the same screen. Controls has the
additional bindings group, whose generated rows come from `0061F5A0`. Diary
`multi_page` is a kind-37 nested product (`0068D5C0`), not a kind-five page or
another registered screen. Cinematics, map selection, equipment stores, soldier
panels and hero statistics generate rows from original data. Their constructors
remain explicit in the catalog. Ordinary owner virtual `+5C` is `0067C860`,
an auxiliary item-template builder; it is not the in-game or item owner's
`+5C` implementation. Its invocation/data contract belongs to detailed dependency
recovery and establishes no additional numbered screen.

## Route boundaries

The catalog records seventy-one concrete route edges and shared rules. Main
entries retain the five-handler priority from `00626B20`: profile, multiplayer,
mission, Options, connection, followed by common Back/Credits/Resume behavior.
Main static edges and the named Exit dialog reuse existing evidence.

Multiplayer `00633F80` selects create/filter/info and modal Add Server using the
ordinary registry. `006333C0` LAN/Internet events prepare services then select
eight. Join `0200002A` copies the selected server; when server `+82` is nonzero,
`00637990` clears the edit, queues modal slot ten and focuses it. Otherwise it
pushes connection console thirteen. These differ from standalone connection:
`0068AED0` timer state five clears its edit, queues **regular** selection of slot
two and focuses it. No common password event is inferred. Cancel/Back follows
the corresponding owner handler. Successful connection selects controller status
nine; real network/game initialization remains a delivery dependency.

Single-player activation `0063B740` publishes mission/standalone events
`02000033/74`; `00638D30` prepares original data and either selects ordinary slot
five or requests descriptor progression. Hero/team/equipment/debrief events
similarly prepare original data, use shared prompts where required and publish
next/previous/return events. Common `02000007/08` clear screens and select status
three/two; `006C20F0` moves through original descriptors and `006C0EF0` creates the
selected owner. Descriptor kind-five briefing and kind-seven game entry are
controller/game boundaries, not additional frontend registries. Equipment's
`020000C1/C6` preparation and mission save/restart paths remain real service
requirements; this inventory does not implement them.

Credits sets status twelve. `006C20F0` constructs a retained video wrapper
(type ten), uses `credits.bik` or the selected filename, and restores its saved
owner at status thirteen/stage zero. Cinematics therefore returns through a
controller lifecycle, not an invented Credits/Cinematics registry.

In-game `0069E620` allocates the retained menu wrapper and saves the running
owner at `+5B8`. Status fourteen enters with `-1`; seventeen and selected session
paths use `-2`. Additional actual callers are `00699900` and `00728450`, including
warmup/session-player changes. `0067F550` delegates settings first, then handles
panel dismissal, local pop or status-fifteen resume according to mode/entry/data
guards. The controller shuts down/deletes the wrapper and restores the retained
game. Its status-sixteen branch unwraps before game maintenance; game-operation
requests also write underlying controller statuses. These are required running-game
routes. Save/loading names alone do not prove another numbered UI registration.

Campaign debrief's custom dispatcher `0068BD70` is a `RET`; its builder creates
available title/text content. Shared dispatch/controller progression applies.
No unobserved Back control or custom callback is supplied for this owner.

## Evidence closure and verification

Client SHA256: `8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`,
image base `00400000`. Original binaries/assets remain untouched and ignored.
Read-only Ghidra references were independently matched by a raw relative-call
scan of both PE CODE sections (`.text` and `.psbf`). Both init/render pointer
patterns were scanned across every section. The evidence map preserves:

| Original target | Direct callers |
| --- | ---: |
| Root definition constructor `00625E80` | 19 |
| Shared screen constructor `00668120` | 12 |
| Shared screen binder `00668470` | 1 |
| Runtime prompt helper `0066D7A0` | 18 |
| Retained owner constructor `0067E840` | 1 |
| Definition helper `00615090` | 220 |
| Definition helper `00615360` | 203 |
| Frontend model loader `0066BFA0` | 13 |

All 487 call sites match the saved reference sets. The 423 definition-helper
sites cover the construction roots used to enumerate embedded products. Inline
ordinary registrations are reused from their existing inventory, rather than
assuming every registration calls the out-of-line constructor. Twelve screen
constructor sites account for registry loops, common runtime construction and
all three description sites (the player uses `006835A0`). Thirteen model-loader
sites additionally account for cursor setup. Direct helpers, vtable discovery
and the controller factories agree on the concrete owner set.

Ninety new valid function exports are retained in ignored `analysis/work`.
Two early export batches stopped at missing/guessed definitions; provenance
counts actual successful exports, with exact-address follow-up batches completing
the needed evidence. No original game, DLL, Windows fixture or Proton execution.

Verification passed:

- `python3 -B tools/validate_frontend.py --map analysis/menu-route-census-map.json`:
  49 instruction/data anchors, 235 inspected table slots, 487 direct calls and
  338 string/reference anchors; exact client fingerprint.
- Temporary catalog audit: owner/type/getter identities, all thirty encoded
  slot IDs, unchanged static registrations, route endpoint closure, explicit
  hero-two/three exceptions, embedded ownership/kind/frame keys, campaign records,
  raw call/table closure and all ninety actual exports. Four damaged-map cases
  and five damaged-catalog cases rejected. Temporary scripts removed.
- Existing static registration, screen construction, runtime confirmation,
  application routing and Options widget evidence maps passed.
- Existing native suite and all 158 tooling tests passed. Frozen function-progress
  and whitespace checks passed. Three strict Windows targets were already up to
  date (`make -n`); no fresh compile or Windows execution was needed.
- Application SHA256 is unchanged:
  `5137074b01e40adb695b66dcf11b3f8bed0e2245bd8f03a27800a9d404e553eb`.

No C++ changed, so no new native sanitizer fixture was needed for this metadata
milestone. Prior widget sanitizer acceptance remains linked from its contract.
Logs and a manifest are retained in ignored
`analysis/work/menu-route-verification-20261031`. Original-function ledger remains
**369 partial (364 client/five engine), zero complete, frozen 18,783**.
