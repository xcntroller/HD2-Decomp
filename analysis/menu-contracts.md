# Menu control and dependency contracts

The third inventory checkbox is complete **source-only**: IDs, assets, controls,
captions, initialization, dependencies and return paths are recorded for all
thirty numbered declaration candidates and the shared/auxiliary products of the
[eight-owner census](menu-route-census.md). No C++ or interactive behavior changes.
This records original construction contracts; it does not deliver another screen
or earn original-function body credit. [Duplicate/placeholder resolution](menu-definition-resolution.md)
now classifies 28 populated owner-scoped roles and two reserved empty hero slots,
preserving all declarations and IDs. [Family delivery checklists](menu-family-delivery.md)
now specify the open flows and their independent/real-environment validation.

## Catalog and scope

| Artifact | Contents |
| --- | --- |
| [Contracts](menu-contracts.json) | Thirty owner/slot records, twelve shared/auxiliary products, assets, captions/styles, lifecycle and return rules, callback/service dependencies |
| [Construction programs](menu-control-programs.json) | Fifty-seven source functions, controls, initializer words and destinations, frame-name sources, original branch conditions and block successors |
| [Expressions](menu-contract-expressions.json) | 25,794 referenced SSA nodes with closed dependencies, retaining runtime inputs, loads, call effects, alternatives and loops |
| [Byte evidence](menu-contracts-map.json) | Original instruction/data anchors, complete kind-dispatch table, helper calls and literal resource references |

The completion boundary is every original reference to the nineteen definition/
initializer helpers below, their construction ancestry (including virtual/shared
builders), the underlying record append boundary, and all three prompt helpers.
Existing fourteen-screen static records and all seventy-one census routes are
reused. This includes nested Options, generated bindings/lists, item/soldier
templates, conditional player/selection/diary content, campaign-selected models,
shared confirmations and the retained in-game information panel. A source site
inside a loop is one catalog record, not a count of runtime controls.

| Source evidence | Sites |
| --- | ---: |
| Explicit definition `00615090` | 220 |
| Screen child `00615360` | 203 |
| Nested child `0061DA10` | 89 |
| Initializer helpers, including record constructors and append wrappers | 959 |
| Additional inlined record appends | 18 |
| Profile iterator callback record template | 1 |
| Shared prompt calls `0066D780/0066D7A0/0066D7C0` | 1 / 18 / 1 |

The 512 control sites include 452 literal frame arguments and sixty generated
arguments. The actual local string tables and their instruction locations remain
in each construction program. Dynamic model/frame/text values remain linked to
their original data inputs, rather than being expanded into invented rows.

## IDs, ownership and initialization

Numbered identity is owner plus slot; its root ID is `slot << 26`. In particular,
ordinary Options is `main:6`, while retained in-game Options is `ingame:2`.
Explicit definition arguments remain full original DWORDs. The two child helpers
encode one-based current list counts:

```text
00615360: parent.id | (((root_child_count + 1) & 63) << 20)
0061DA10: parent.id | (((nested_child_count + 1) & 255)
                     << ((parent.id & 0x000ff000) ? 4 : 12))
```

Conditional construction changes those counts. Follow each control's source
receiver and CFG, not its position in a flattened JSON array. Programs can be
shared by multiple parents; the static `00623DD0` program also narrows each of its
53 controls to the existing ordinary slot and preserves its known static ID.

The sixteen initializer helpers are `00614DA0/DD0/DF0/E20/E50/E80/F10/FA0`,
`0061E0A0`, `00625D20/D50/DE0`, `00628DA0`, `00670850`, `00676110` and `00680B00`.
Their catalog signatures preserve thiscall receivers versus cdecl stack arguments.
The original record is six DWORDs: target, code, argument, words at `+0C/+10`,
and shared payload at `+14`. Integer words can contain pointer or float bits.
Unwritten words remain unspecified; they are never filled with zero.

The 959 helper calls comprise 363 record producers inserted into definition
lists, four diary records queued during construction, and 592 append-wrapper
calls. Producer records retain their exact consumers; two player records reach
one append through alternative SSA paths. Control event lists are at definition
`+10`; root event wrappers use `+4`. Data lists at `+20` remain separate.
The eighteen inlined records retain their reaching stack words, with exact
instruction stores where SSA optimization omits invariant writes. Two unused
`+10` words have no reaching SSA definition/local write and explicitly preserve
original prior/unspecified stack storage.

The append closure independently finds all 380 calls to `00615D50` and eleven
calls to `00616250`. This exposes the profile iterator `006210B0`, passed by
`006211B0` to `006AEF50`, which has no ordinary direct caller. It creates retained
text from the incoming profile name and appends target zero, `01000016`, argument
`13` at `0062115C`. Conversely `0065CD40` at `0065CD73` copies tagged runtime
observer records to control `+18`; it is not a definition initializer. The census
already establishes the in-game embedded `0067AD70` table and ordinary auxiliary
`0067C860` virtual builder; both stay under their actual owners.

Common initialization remains `0066AD10 -> 0066C5B0`: cursor/runtime dialog,
derived content, successful model binding, root initialization, then initial
selection. Every candidate records its derived `+40` initialization and `+3C`
cleanup entry. Factory `0065D150` uses the complete `0065E048` kind table (2–40),
lookup flags `10021`, visual `+2C`, data `+78`, events `+70`, then successful
append. Original model/binding failure branches and the documented native RAII/
allocation policy remain distinct. No unavailable service is reported successful.

## Captions, styles and assets

There are 343 literal localization references to 255 IDs, including shared
prompts. Each located English record has raw offset, line range, byte size and
SHA256 in the catalog; caption text is not copied into tracked files. Twenty-one
IDs are unlocated in the loose English `TEXTY.txt`; their original references
and layered-localization dependency remain explicit. Dynamic captions retain
their raw/shared payload or callback/data source. `01000015/17` records are not
confused with raw text/prefix messages `01000014/9B`.

All ninety sequential text styles from `00641E30 -> 00641D90` are recorded,
including the forty later `00643790` constructor records. Font, color, height
bits and remaining semantic parameters are preserved. A style occupies `34`
hex bytes: eleven DWORDs, byte fields at `+2C/+2D`, final DWORD at `+30`;
padding at `+2E/+2F` is not invented. The entire 6,494-byte original initializer
is anchored, and the original known styles provide independent cross-checks.
Font registration `0063DC50/0063F1E0` selects BIG/SMALL as IDs 2/3 and SYMBOLS
as ID 4. Width selects Set 1 below 800, Set 2 below 1024, otherwise Set 3; language,
CE/Japan and system-font fallback branches remain actual dependencies.

The asset manifest contains 34 model keys. Twenty base `models.dta` members
decode natively, referencing 76 material keys; all 76 resolve and hash-verify
through the native `Maps.dta` reader. Fourteen loose ANSI/SYMBOLS font files are
fingerprinted. Seven Options frame aliases from `0061DBC0` (`nextdetail` through
`use_high_models`) map to `GD00`–`GD06` before graphics controls are built.

Fourteen keys are unlocated in the base model archive: eleven campaign `f_s_*`
keys, `main_ingame`, `player_screen`, and `player_screen_2`. This is **not evidence
that the installed layered game lacks them**. Original language/Patch/resource
directory priority remains required; ISD1 loading is delegated and the native
ISD0 extractor cannot establish that layer's member availability. Higher layers
can also override the twenty located base members. No original DLL was executed
to resolve this uncertainty. Sound, credits/cinematic filenames, profile/save,
GameData/HumanStore, controller and session services are recorded separately.

## Return contracts and delivery dependencies

Each numbered contract carries an explicit return rule and original evidence,
alongside its outgoing census routes. Owner rules cover current-stack handling,
prompts, retained game/video owners and real controller/service handoffs.

- Ordinary `00626B20` dispatches profiles, multiplayer, mission, settings and
  connection handlers in priority order. Profile Back runs `00622A90` before
  regular pop; password/Add Server Back discards the edit with `10000200(1)`
  before modal pop. Named confirmation Escape sends `14001200(1)` then
  `06001005`. Initial Main retains the accepted Exit policy.
- Settings `0061BEA0` handles both owner-family IDs. Back maps to `020000C4`;
  an active binding editor requests mode2 prompt label`2530` hex, with both
  replies `020000C7`. Otherwise it hides the six groups, queues `0600106F`
  apply, then `06001003` pop. Profile/input persistence and queue order matter.
- Team/items selection Yes publishes `0600106A` then `06001005`; Cancel only
  pops. Team `00691020` also clears selection before Back and uses a mode1
  prompt outside slot2. Other validation and descriptor transitions retain the
  original owner dispatch/service dependencies.
- Hero slot4 sends its original Yes/No action then modal pop. Slots2/3 retain
  their reserved empty registrations and IDs. Campaign `0068BD70` is `RET`; no custom
  Back action is fabricated for its title/text builder.
- Ordinary password entry `00637990` remains modal slot10. Standalone
  `0068AED0` timer state5 selects regular slot2. Connection cancellation/success
  needs its actual network/controller owner, not a shared invented event.
- In-game `0067F550` delegates settings first and dismisses a visible `+4BC`
  information panel before local pop/resume logic. Entry/mode/session guards
  control status15 restoration; status16 unwraps before maintenance. Credits/
  cinematics restore their saved owner through `006C20F0` status13.

The catalog records 89 callback binding dependencies and the direct/indirect
call dependencies of 130 inspected functions. A callback pointer is evidence of
a required implementation; it does not establish that its body, engine service,
game handoff or dynamic data source has been reconstructed. These source graphs
are analysis records, not executable host definitions. Main/Exit is still the
only delivered numbered flow; full Options remains disabled pending delivery.

## Verification

Client SHA256 is
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`,
base `00400000`. Read-only Ghidra exports and isolated native asset extracts are
retained under ignored `analysis/work`. Important claims have original addresses;
no original game, DLL, Windows fixture or Proton execution occurred.

The final byte map validates 1,378 instruction/data anchors, 1,882 direct calls
and 690 resource references. Independent scans of both CODE sections match all
nineteen helper, three prompt-helper and two append-boundary reference sets.
The temporary catalog audit checks all original exported instruction bytes,
expression closure/semantics, IDs, helper arguments, control kinds, CFG edges,
captions, style cross-checks, and native model/material/font hashes. Deliberately
damaged map/catalog cases are rejected. Detailed counts and logs are retained in
ignored `analysis/work/menu-contract-verification-20261031`; temporary audit
scripts are removed after verification.

Existing native checks, all 158 tooling tests, related frontend/widget evidence
maps, frozen function-progress and whitespace checks pass. Three strict Windows
targets are up to date; no fresh Windows execution or desktop test is claimed.
There is no C++ change requiring another sanitizer fixture; prior widget
sanitizer acceptance remains applicable to its unchanged implementation.

Built and staged application SHA256 remains
`5137074b01e40adb695b66dcf11b3f8bed0e2245bd8f03a27800a9d404e553eb`.
Ledger unchanged:369 partial (364 client/five engine), zero complete, frozen 18,783.
All continuation changes, including the prior census, remain uncommitted at
the user's instruction.
