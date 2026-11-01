# Single-player pre-mission reconstruction

Status: integrated source, Main Single Player enabled; team-selection checkpoint
user desktop-accepted2026-11-01. Loadout screens implemented but completely
untested until accessible; full-pipeline acceptance remains pending.
The user explicitly requested the full pre-mission pipeline, including hero,
team, equipment and briefing. Early research below records pre-implementation
state; the latest checkpoint and desktop follow-up supersede it.

The [2026-11-01 function audit](singleplayer-function-audit.md) credits90
previously omitted partial bodies and extends four already-counted entries.
The current plan separates front-end preparation/request acceptance from the
later real game receiver, gameplay-save and post-mission stages. Equipment
screens remain completely untested until accessible.

Evidence: unpacked client SHA256
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
Existing [family contracts](menu-family-delivery.md#single-player),
[route census](menu-route-census.md) and control programs remain authoritative for
screen identities, IDs, captions and original callback bindings. Proprietary
exports and extracted data remain ignored under `analysis/work/`.

## Completion boundary

Deliver the actual mode/mission routes, hero editing, squad selection, equipment
selection/validation, descriptor-driven movie/briefing playback, Back/cancellation
and an explicit game-controller handoff carrying the selected mission and real
squad/equipment state. The user clarified that this checkbox ends at a validated mission/squad/
equipment handoff and must report actual game entry as unavailable. Gameplay
and gameplay-save loading remain outside the checkbox. Do not fabricate
successful world loading; retain separate real-receiver delivery gates.

Controller `006C20F0` status2 advances, status3 reverses. Both currently throw in
the native controller. `006C0EF0` dispatches kind6 subtype0 to hero, subtype1/2
to team, subtype3/4 to equipment; kind5 uses ordinary video and kind7 game entry.
The native application currently rejects non-null descriptors.

## Confirmed route behavior

- `0063B590/0063B740`: screen3 offers Campaign, Lone Wolf, Carnage and the two
  standalone variants. Three additional registered buttons stay hidden. Focus
  updates the explanation and the side-image selection. Main's Single Player
  action must remain unavailable until its actual receiver exists.
- `00638D30`: campaign starts reset preparation and descriptor traversal when
  profile progress is absent; otherwise they show the available mission list.
  Standalone uses `0063A950`. Selecting later campaign progress can require real
  save loading through `006C3340`; that receiver remains a separate boundary.
- `006B8A20` resets traversal to the campaign/mission container. `006B8AA0`
  advances and skips containers. `006B8B20` reverses to eligible kind6 setup
  descriptors, skipping movies/game entries. Lone Wolf skips subtype1 in the
  controller. Preserve the descriptor flags governing reverse traversal.
- `006B9680` builds standalone routes from selected GameData, including cloned
  metadata and eligible movies/setup nodes. It appends subtype2 team selection
  and subtype3 equipment (except flattened mission ordinal10), game entry and
  post-mission subtype5. It copies only kind5 movies with flag18 set. `006B93C0` aggregates 16-byte kind6
  equipment entries from the selected and preceding missions. Do not substitute
  a fixed list of setup screens.
- `006B1340` queries actual per-root/per-mode profile progress. Campaign,
  Lone Wolf and Carnage have distinct offsets; negative progress means absent.

A temporary native ASan/UBSan audit exercised the existing GameData reader on the
baseline `others.DTA` member `GameData/Gamedata00.gdt` (22,161 bytes, SHA256
`f92ab7e8796148af7854438ffca4df34b55cca8e83336878d7f569cdcd3319b1`).
It loaded successfully without sanitizer findings. In that baseline tree,
Tutorial has a campaign-level subtype2 team selector followed by game entry;
Arctic has campaign intro/team, mission briefing, equipment, game entry and
post-mission subtype5. Arctic4 adds subtype1 team selection. Thus briefing can
precede equipment. This is baseline archive evidence, not proof that Patch.dta
or loose overrides are absent. Production must use the real startup resolver.

## Runtime owners and equipment dependencies

Existing `GameData` and `HumanStore` are data readers, not the live squad:

| Original owner | Meaning | Native status |
| --- | --- | --- |
| application+04 / `008AE900` | GameData/application data manager | Tree readers and selected queries exist; cursor/preparation absent |
| application+10 / `008AE90C` | HumanStore candidate records and portraits | Reader exists |
| application+14 / `008AE910` | Live squad (polymorphic pointer-vector owner) | Not reconstructed |
| application+18 / `008AE914` | Store quantity vector | Not reconstructed |
| application+1C / `008AE918` | Second store quantity vector | Not reconstructed |
| `0098B030` | 500-slot item catalogue | Not reconstructed |

The global application object starts at `008AE8FC`; thus `008AE910` is
application+14, not+18. Constructor `006B4A60` installs the polymorphic owner
`006B8020` at+14. `006A5C60` initializes the two plain store-vector owners
at+18/+1C; it is not a soldier constructor. `006739F0` allocates a0xAC-byte
live soldier. This corrects the earlier provisional owner-offset assumption.
The game mode owner is application+28 / `008AE924` (`006B4670`).
`00694520` removes certain unavailable HumanStore-backed soldiers on team entry
outside standalone mode. `006958A0` refreshes candidate eligibility using live
soldier HumanStore index+3C and active byte+44. `00694AE0` chooses an eligible
candidate using original indices, capped at 30; no synthetic roster.

`007DC4E0` lazily loads the catalogue once, guarded by its initialized byte;
it does not clear store quantities. `006A5C30` queries the actual store vector.
`007DC500` opens `Tables/items.sav`, requests up to 252,000 bytes and visits 500
slots. Each inactive slot consumes a four-byte flag; an active record occupies
508 bytes total (flag, class and payload). Classes0/1/2 construct via
`007E03F0/007E04E0/007E0660`; readers are `007E04B0/007E0620/007E0750`.
Unknown active class consumes only the flag/class pair and leaves a null slot.
The baseline file has 246 recognized active records, nine zero flags and
245 unrecognized class records (including 0xCDCDCDCD development fill); an audit
following the original walk consumed126,964 bytes of128,520. Do not reject all
non-0/1 flags or assume the catalogue consumes the complete file.
The common reader `007DFCF0` has three 20-byte resource names, scalar fields,
and up to two typed nested payloads. Ten nested payload types have separate
original serialization sizes; do not read all records as one C++ struct or
infer firearm/weight semantics from names. `007DFAB0` computes common size0xA8
plus nested payload sizes. Missing catalogue is an actual failure.

Baseline catalogue: 128,520 bytes, SHA256
`9e82e40543e3c5b65d6004f2d45fac656d1e4f5a8df5259a3a0db2520c5983b6`.
Other required baseline members: `GameData/presets.def` (28,608 bytes, SHA256
`30e8f95b21df7e198413b62cd17c922b3f580e24d0f71fe8792019a1f702bd3d`),
`GameData/miss_itm.def` (3,592 bytes, SHA256
`1a1ae4dd388f5779549e9cc0b2173f700477281300b0015c6f2b87306ef86453`).
These were extracted using the project's own ISD0 reader, never by executing
original code. The original `Patch.dta` is ISD1; do not ignore its production
overlay when integrating.

`0067E200/0067E320/0067E350` own and load mission equipment policies. Each
mission state has ten triplet slots and a vector of required (item,count) pairs.
Equipment acceptance `00673DD0` checks, in order:

1. `00678AC0`: required equipment. Kind15 and item0xE7 have squad-size clamping;
   `00679060` counts actual equipment slots and carried vectors, including
   per-holder restrictions. Failure uses a real localized modal.
2. `00673D60` / `006A2330`: capacity ratio must be at most1. Disassembly at `00673DA0` passes
   soldier+0x8C to `006A2330`: capacity is at+8C, load at+90; the helper
   returns the absolute load/capacity ratio.
3. `006743F0`: active soldiers must have a real firearm in equipped/carried
   entries; otherwise a Yes/No confirmation precedes acceptance eventC6.

No implementation or function credit is assigned for these exported bodies.

## Additional confirmed data/control contracts

- Presets `0067D660`: header(version,count); record sizes0x1B0/0x1B4/0x1B8
  for versions0/1/other. Baseline version2 has65 records and consumes28,608
  bytes exactly. `0067D280` initializes absent equipment IDs to-1 and counts0.
  Carried counts at+34/+A8 fit14/32 fixed pairs. Optional current-profile
  `user_presets_orig.def` overlays at most24 records into four groups of six;
  missing overlay invokes `0067D930` defaults, not a fabricated load success.
- Mission policies `0067E350`: version1 skips a60-byte header table (other
  versions200), then reads slot count, active markers, mode, up to ten triplets
  and required item/count pairs. The baseline has60 slots,24 active, all mode4;
  the decoded stream consumes3,592 bytes exactly.
- `007DF950` computes permitted equipment slots from actual item class/kind/
  category, not filename heuristics. Slot masks are1 uniform,2 headgear,
  4 primary weapon,8 backpack,0x10 secondary weapon,0x20 holsters and0x40 pack.
  Soldier fixed entries are at+4C/+52/+64/+58/+5E respectively, each a six-byte
  (position,item,count) record; item IDs are two bytes into those entries.
- `00671200` checks the slot mask and existing occupancy before transfer;
  `00671110` / `006A3080` perform the transfer and return the unaccepted/swapped
  record. Store withdrawals (`0067BDD0`) and returns (`0067BDA0`) must balance;
  the original explicit force flag may replenish a shortage for auto-equipping.
- Hero kind25 `00650350`: base+delta values, base-setting message72, absolute
  value-setting73 and query79. `0064FF80` caps incremented values at100;
  `0064FEA0` prevents reducing delta below0. Both consult the linked control
  through messages71/70. Hero rows link to control04300000, the20-point
  counter, so adjustments debit/refund the actual shared counter.
- Hero `00697A50` bases at00842028 are
  80,75,60,50,75,30,80,40,75,70,80,50,80,30,70,40,50,60.
  Rows are Health, Might, Reactions, Inteligence, Endurance, Stealth,
  Light Weapons, Heavy Weapons, Shooting Rifle, Throwing, Morale, Driving,
  Swimming, Traps, First Aid, Pick Lock, Pilot, German (original spellings).
  `00696DC0` acceptance writes nine fields of actual HumanStore record0:
  rows1/2/5/6/7/10/11/14/15 to offsets48/4C/50/60/58/5C/54/68/64.
  Preserve this exact mapping; do not reinterpret the unused rows as new stats.

## Briefing and game entry

Kind5 uses vtable `00815BFC`: initializer `00688040`, shutdown `00688460`,
fixed update `0067EC60`, update `00688C10`, render `00688F50`, status `0067EA00`.
It can reuse the recovered ordinary movie/subtitle backend. Its descriptor
filename/subtitle/conditional flag must be preserved; retained Cinematics status13
is a different route. The next vtable is a different owner: `006C1F90` must not
be mistaken for a briefing callback.

Kind7 constructs the game owner through `00698510`. `006B9190` supplies the
current mission node, or the synthetic mission node in standalone mode; it does
not pass the kind7 marker itself. The constructor allocates real world/game
owners (`0049A420`, `00490740`, `00523AB0`) and an engine scene, so it cannot be
replaced by a menu-screen registration. Its receiving contract ends at a validated request for this checkbox, per the
user clarification. World loading stays explicitly unavailable.

## Current source work and verification

Unintegrated native owners now exist in `recovered/frontend/pre_mission_data.h`,
`pre_mission_route.h`, `pre_mission_inventory.h`, `pre_mission_squad.h` and the
static `single_player_definition.h`. They are dependencies of the full delivery,
not a completed flow. Main still has no enabled Single Player receiver. No
function-ledger credit has been assigned yet.

Temporary native ASan/UBSan audits passed:

- Catalogue246 recognized records,65 presets,24 active policies/60 slots,
  fourteen required-item references, all preset equipment references and twelve
  truncated-file rejections against actual extracted baseline data.
- All24 standalone mission routes through game marker; team/equipment reverse
  and forward traversal; actual Tutorial campaign team/game sequence.
- Hero20-point accounting, original nine-field acceptance, unavailable-candidate
  rejection, five-best/random auto-selection, unique squad roles and retained
  equipment across roster reconciliation.
- Equip/remove, incompatible-slot and carrying-capacity rejection, pack removal
  with contents, required-item and every-active-soldier firearm checks.

Native strict warning builds passed. Windows i386 header compilation is being
checked separately. No Windows executable, original DLL or Proton was run.
These audits do not establish desktop rendering, UI interactions, archive overlay
behavior or complete pre-mission delivery. Temporary audit programs remain in
`/tmp` during implementation and will be removed before completion.

Important machine-code corrections: `006A2330` reads a signed integer carrying
capacity at soldier+8C and a float load at+90. `006A3080` volume units are twice
a positive item category, otherwise1. Backpack payload type8 supplies float
weight/volume limits at serialized offsets12/16. The fixed-slot jump table at
006A3914 maps masks1/2/4/8/16 to soldier entries+4C/+52/+64/+58/+5E.
Auto-team score for fourth slot is `(Might - Shooting *0.1)*10` (006955DB,
constants0080F550/0080FA14); first/third use Shooting, second First Aid.

## Remaining work for the complete checkbox

User implementation priority: finish and wire the entire flow before further
tests/audits, then provide a complete interactive build for parallel desktop
testing. Earlier audit results above apply only to their stated source versions.
Newer implementation work below has not yet been checked.

- Complete original screen/control binding, descriptions, pointer/grid transfers,
  shared prompts, profile preset persistence and Back/cancellation ownership.
- Connect real startup resource resolution and mission-progress gates, typed
  controller traversal, ordinary movie/briefing and the validated unavailable
  game-entry endpoint. No successful placeholder world receiver.
- Run relevant existing native/tooling checks and strict Windows targets, audit
  the integrated pipeline with sanitizers, stage for user desktop validation.
- Update the frozen ledger only for implemented inventory bodies; keep the plan
  unchecked until full desktop acceptance. Do not stop at these source owners.

## Current integration work (not delivered)

`pre_mission_flow.h` now composes descriptor entry, hero acceptance, team
reconciliation, stock transfers, preset capture/application/writing, required
equipment/capacity/firearm checks, movie completion, reverse traversal and an
owned game request. `StartupSession` owns it lazily and supplies the real
archive/loose resolver, current-profile path, HumanStore reload and progress
words. No extracted-data path is used by these production services.

`BriefingApplicationState` keeps ordinary descriptor movie completion as status2;
its type is7 (00815BFC+1C→006C0E50, `MOV EAX,7; RET`).
opening/retained movie status13 remains separate. `PreMissionApplicationState`
supplies hero9/team3/equipment4 lifecycle adapters for their screen runtime.
These owners are awaiting host screen/controller construction and registration.
The actual00688040 subtitle loader derives its path from descriptor+10 (movie
filename); descriptor+14 is retained without inventing a separate subtitle path.
Flag18 selects music target.05 instead of the ordinary float3C343958.

The host side-image selector and hero-stat control now use the recovered message
contracts. Hero kind is **decimal25 / hex19**. Original0064FC56 binds the arrow
named `barrow up*` to decrement71;0064FD2F binds `barrow down*` to increment70.
The value uses style22: SMALL,.019,colorA0000000,center/center (00641E30).
Programmatic72/73 assignments do not debit/refund the linked point counter.
Synchronous70/71 queries use `MenuEventQueue::query_control`, retaining mutations
to the caller's event; `dispatch_immediate` intentionally owns a copy.

The shared derived list adapter accepts actual page/spacing settings and active
row IDs, rather than the Options binding page's fixed nine-row layout. Derived
text planes can bind their own names beneath a different control root.

Recruitment006908F0 uses table00841CB0: twelve10-word rows, with indices terminated
by-1. The populated rows are2=[7,9,11,16,17],3=[2,5,8,12,19,22],
4=[4,10,23,24,28,29],5=[3,6,13,21,25],6=[14,15,18,20],7=[0,1,26,27].
Other rows are empty. Bytes after row11 are strings, not extra recruit rows.
Eligible records gain item6; campaign subtype2 returns live equipment to stock.
Profile progress006B13A0 uses offset74+root*16 for standalone unlocking;
006B13C0/006B1340 use78/7C/80 for Campaign/Lone Wolf/Carnage.

Preset IDs at20/24/28/2C/30 mean uniform/primary/secondary/headgear/backpack;
00677D00 confirms the slot masks in its00677620 calls. Native capture/application
preserves this serialization order and balances old/new stock. Missing user
preset file writes the initially empty vector (count0); on an existing-file load,
0067DBF0 first creates24 defaults and0067D660 overlays decoded entries.

The setup descriptor's `items` vector is a separate profile-discovery dependency:
0066B0D7 reads application+20/008AE91C,006AE510 chooses its root-specific94/B8
record,006A7820 appends the ID unless that record's first byte is set. This is
not a HumanStore roster flag. Read-only exports are retained under ignored
`single-pipeline-entry-flags-20261031`. Its profile persistence integration is
still pending along with the remaining host screen work.

Hero content augmentation is now in `host/pre_mission_hero_menu.h`: original
root IDs041..047, GRSkillSpec/GRSkills, cloned18 named skill rows with spacing.02
(00697DA2 immediate3CA3D70A), counter initialization, stat queries and original
nine-field acceptance. Counter initialization runs once per owner, preserving
the budget when a confirmation is cancelled. It is awaiting runtime registration.

Image control00650730/006507E0 now has a host adapter for material-bitmap binding,
portrait pointer38, restore39, actual IGraph texture load3A and named bitmap9A.
It follows the actual visual+1E0→first material→5C bitmap chain; replacement calls
the selected bitmap interface+1C. The real HumanStore portrait resource can be
used directly. Exports including team augmentation helpers are retained in
`single-pipeline-image-team-20261031` for the next content implementation.
`MenuScene` can load the derived owner's actual primary model; `MenuText` can
start with no Main definitions, preserving owner-scoped overlapping IDs.

## Integration continuation — 2026-11-01 (unverified)

The retained entry owner now builds mode screen3 and mission screen5 from actual
GameData and profile progress, and supplies fresh route requests. Later campaign
save loading remains explicitly unavailable. Derived content now includes the
team portrait strip, four selected slots, stats/biography, awarded medals,
equipment categories, item/ammunition rows, carried slots, presets, transfers,
validation prompts and the conditional four-photo selection model. Controller
status2/3 dispatch now replaces actual descriptor states, including ordinary
briefings, reverse traversal and the user-approved unavailable-game endpoint.
Main Single Player is still disabled while implementation is being completed.

Important additional evidence:

- `00696740` hides both medal variants before placing awarded HumanStore.items
  IDs on `medal_%02d` anchors; when more than six exist, earlier awards use the
  compact b variant. Unawarded medals are not displayed.
- `00694640` toggles Panel02 statistics and Panel03 biography, labels A69/A6A
  and A6B/A6C. It does not open a numbered biography modal.
- `00691840`/`006919D0`/`00691020` implement the team selection modal: choose
  a team slot, swap its full slot record with slot0, reconcile, then advance.
  Equipment `00675F30`/`00673DD0` uses the same actual selection model to choose
  the sole active actor for root0 mission ordinals2/15 when the squad has more
  than one actor. Cancel returns without changing that choice.
- `007DFA10` store grouping: kind8→0; class0→4; kinds15/16→6;
  kinds0..2→1;3/4→2;5/6/7/12→3;9/10/11/21→5; remaining→7.
  The six store lists expose groups1/2/3/5/6/7; ammunition is associated with
  its weapon row. `0067C860` uses page4 and spacing float3DC28F5C.
- Item-row factory case `0065DAD1` installs vtable00814B18. Its visual/message
  bodies `00652740`/`00652AA0` bind main image, `ammo*`, `text_info*`, select
  `X_SQUARE.BMP`, and retain independent item/ammunition IDs. `007DF650` loads
  the actual second catalogue resource name plus `.bmp` through IGraph.
- Per-image material isolation follows the A0/material-list18 operations in
  `00693270`, `006763B0` and `0067C860`. Native frame clones retain their
  complete child trees; retaining just the root allocation is insufficient.
- Original combo constructor `00658770` binds `header*`, `cbdown*`, `listbox*`;
  `00659160` owns item append/clear, header81, selection76, query0200002F,
  expansion2D, commit2E and notification0400E000. The preparation adapter uses
  those actual model parts. `00678CE0` reserves first item500 for Custom;
  subsequent items address the mask5 preset view.
- Style1 is SMALL .02, justified/top, with wrapping; style23 is SMALL .02,
  right/center; style6 SMALL .019,right/bottom (`00641E30` catalogue).

No builds, tests or sanitizer audits have been run on this continuation, in
accordance with the user's implementation-first instruction. Function credit
and desktop-delivery status have not changed. Remaining work includes completing
original automatic-equipment supplementation/required-item preservation,
description overlays, profile discovery and username-save lifecycle, then
implementation review, builds/audits and staging the full pipeline.

## Handoff checkpoint — 2026-11-01

This checkpoint supersedes the earlier “unverified” continuation above. The
implementation is now wired end to end and Main Single Player is enabled.
No desktop acceptance of this pipeline has been recorded.

Additional source completed:

- `pre_mission_flow.h` applies original equipment bundle assignment0067A0F0,
  required-item preservation00679490 and supplementation006777A0 (initial
  level0; explicit Auto Equip also levels1/2). Catalogue/policy/preset data stay
  real, stock/capacity checks apply to manual and preset transfers. Inventory
  drag transfers have rollback when placement fails. Custom clears optional
  equipment, including Assign to All; no preset editor was invented.
- Equipment now populates required-item text006782B0 (AA9, count plus item
  captions and original punctuation), selected soldier statistics00674A60,
  and missing-item message00678AC0 using localized804's `%s` substitution.
- Nonnumbered description owner binds actual `description` art/text for items
  and `description01`/`text01`/`text02` for medals. Original00669B90 derives
  medal captions2774+2*index and2775+2*index. Item tooltip currently contains
  name/weight only; full original item-detail fidelity remains to review.
  Team and equipment drag operations currently have no floating dragged visual.
- Username writer006A13B0 uses original029A/index/23B4 chunks and runs on team
  deactivation00696C00. It opens lazily, preserving the original all-empty-name
  behavior. Native roundtrip passed. Discovery006AE510/006A7820 is represented
  by two transient root-specific lists;006B6530 clears these on resource reload.
  Original profile writer006B09A0 does not serialize them; no new save chunks
  were invented. Save-load suppression of appending belongs to excluded receiver
  006C3340, which toggles the record's first byte around load.
- Main factory availability now includes the real Single Player receiver.
  Controller status2/3 follows descriptors and ordinary briefings; kind7 owns
  and logs a validated request, then explicitly reports game entry unavailable.

### Latest user desktop failure and fix

The first staged full build failed during Main construction:
`Static text cannot receive a button callback`. Global image availability had
also bound Cinematics' decorative screenshot through generic MenuControl's
button-only pick registration. Generic static/image binding now writes the
actual frame pick word without calling ButtonVisualPair, and generic decorative
controls are ineligible for input. Derived PreMissionImage retains its own
eligibility and callbacks. Exceptions now include the offending frame name.
The corrected build has been staged, but the user has not reported its result.

Text review also found and fixed support for right/center style23 and inline
style15 required-item records. The latter are merged into a paragraph before
wrapping; the required-item paragraph passed the latest temporary native audit.

### Verification actually performed

- All three strict i386 Windows targets compiled before the latest shared
  image/text fixes: application-launch, engine-window, startup-probe. The full
  application target was rebuilt successfully after those fixes.
- Existing `make -C recovered test` passed and158 Python tooling tests passed
  before the latest image/text fixes. Recheck affected shared code next; do not
  describe those older suite results as covering the final fixes.
- Temporary native g++ ASan/UBSan audit **after** the latest fixes passed49
  real-data routes:23 standalone missions in modes0/3 and campaign opening in
  modes1/2/3;44 equipment screens and46 movies. It exercised actual GameData,
  catalogue, presets, policies and baseline HumanStore data (portrait calls
  replaced only inside the audit), both auto-equipment passes, acceptance and
  owned game-request lifetime after cancellation. Also checked username save/
  reload and the required-item paragraph. No sanitizer errors; temporary audit
  source/binary and roster extract removed. No permanent tests added.
- Latest staged/build application SHA256:
  `b8bcd31ac738034121dab1a5e394e11b7cf4f72480e3821651ed5c6245ca83ed`.
  Stage-only command was run; no original client/DLL/Proton was executed by agent.
- Function ledger unchanged428 partial,0 complete,18,783 frozen. No new credit,
  no commit, checkbox remains unchecked. README launch notes are not yet updated
  for the new pipeline. User explicitly requested handoff now due usage limits.

## Desktop follow-up -- authored screen transforms, 2026-11-01

The user launched the corrected image-binding build and reported Single Player
artwork beginning at the window center, with the remainder clipped offscreen.
The latest runtime log reaches Main and constructs both single-player screens;
the prior static-text callback exception is no longer the reported blocker.

Root cause: `ApplicationMenu::add_single_player_screen` applied the confirmation
position `(0.5,-0.375,-0.5)` to both authored full-screen models. At800x600, X/Y
alone translate the artwork400 pixels right and300 down, matching the screenshot.
Original registered construction **00627060** applies that position only to
family`10000000` (confirmation), not`0C000000` (modes) or`14000000` (missions).
The recovered shared `construct_registered_screens` already preserves this gate.

Removed that extra translation. The same error was present in
`PreMissionMenu::initialize` and unconditional `add_screen` positioning; ordinary
hero/team/equipment and four-photo screens now retain their loaded transforms,
while actual confirmations remain centered. Hero construction **006971B0**
does not translate the primary model; its confirmation builder **00697510**
explicitly does. Hero primary builder **00697600**, team four-photo builder
**00691840** and equipment four-photo builder **00675F30** do not apply the
confirmation translation. No global loader/camera/pointer/text changes were made.
Original exports remain ignored under `analysis/work/`.

Verification after this correction:

- All three strict i386 targets built: application-launch, engine-window,
  startup-probe. No Windows fixture/original client/DLL/Proton executed by agent.
- Existing native suite and158 Python tooling tests passed, now including the
  previously un-rechecked shared image/text changes.
- Existing native menu-text geometry checks passed under temporary ASan/UBSan;
  temporary binary removed. No permanent tests added.
- Function-progress render/check and whitespace checks passed; no new credit.
- Stage-only preparation completed. Build and staged application SHA256:
  `e1063e405548b6e16747244c82e8792f97e7d76e981b83a87a2f592f6fe34743`.

The correction is source/build-verified, **not desktop-accepted**. Next user check:
Main Single Player must display the full mode screen, mode hover explanations/
side images and Back; then exercise mission list and descriptor-dependent
hero/team/equipment/briefing screens, including four-photo and confirmation
dialogs. The agreed full-pipeline checkbox remains open; fidelity/ownership
limitations listed in the handoff still apply. No commit or push.

## Desktop follow-up -- frame binding, 2026-11-01

The next user launch reaches Single Player mode hover, Campaign activation and
the real `team members selection` model (137 frames). It then fails during
content construction with `Missing original menu frame: Panel01`. The runtime
hierarchy explicitly contains lowercase`panel01`, `panel02`, `panel03`.
This result does not establish full desktop acceptance of the previous layout
correction, but the failure is now in team construction rather than entry.
`AGENTS.md` was read before this follow-up.

The host lookup was incorrectly case-sensitive. Read-only disassembly of the
fingerprinted original engine confirms:

- Dummy frame vtable`1009BBA8`, slot`48`, resolves to **1001DED0** (FindFrame).
- Frame comparison **1001DF00** uses default case-insensitive comparison at
  `1001DFED -> 10098740`; flag`20000` instead takes the byte-exact branch.
- **10098740**, instructions`1009876A..10098784`, folds ASCII A-Z by adding20h.
  Original team builder **00691E10** searches lowercase`panel01` but declares
  `Panel01`; **00693030/006761A0** likewise declare`Panel02`.

`MenuScene::find_named` now uses a shared ASCII-folding name helper for exact
names and the host's existing trailing-star prefix patterns. Ambiguity and
hierarchy bounds still throw. No global space/underscore normalization, silent
fallback, locale-dependent folding or additional wildcard language was added.

Related preparation bindings were checked against original control-program
string sources and already extracted model name bytes. Fixed team first-aid
and team-member caption anchors (spaces, not underscores); equipment capacity
is`name- capacity`, and first-aid, soldier-stats, mission-essentials and
military-storages anchors also retain their actual spaces. Original builders:
**00693030,00693270,006761A0,006763B0,006755F0**.
Equipment builder **006763B0** null-guards optional authored photos/tables before
hiding them. The loaded baseline lacks`photo07`; restored this guard for its
prototype-hiding loop only. Required controls still fail on missing frames.
Generated store caption`text_info` remains intentionally renamed from
`text_info00`, as the existing native construction requires.

Verification:

- All three strict i386 targets build; existing native suite and158 Python tests
  pass. No Windows/original client/DLL/Proton execution by agent.
- Temporary ASan/UBSan audit passes65,536 byte-pair comparisons, all three panel
  regressions, mixed-case prefix patterns, empty/short names and space-versus-
  underscore rejection. Temporary audit source/binary removed.
- Authored-anchor literal audit passes137 names:53 team,68 equipment,9 hero,
  7 four-photo. This checks actual name bytes, not live engine construction.
  Explicit exclusions: the model key, null-guarded absent`photo07`, generated
  renamed`text_info`. Dynamic cloning/input/rendering still need desktop checks.
- Frozen function-progress and whitespace checks pass. No new function credit.
- Stage-only preparation completed; build/staged application SHA256:
  `08496f2d3a6d7ed88dd65bd52adc54d5aedc2679709d55f92b58791266690daa`.

Next desktop check: repeat Main → Single Player → Campaign, then candidate/team
selection, Back/continue, equipment and relevant dialogs/briefings. Source/build
verification is not desktop acceptance. Full-pipeline checkbox remains open.
No commit or push.

## Desktop follow-up -- portrait strip anchor, 2026-11-01

User explicitly confirms the prior frame-binding failure is fixed. The new
screenshot reaches Team Member Selection but shows all five candidate portraits
shifted right/down relative to the authored empty photo slots.

Root cause: the native team builder captured`photo03`'s local position while it
was still relative to translated`panel01`, then applied that local anchor to
clones under world-origin`GB_Soldiers`. These coordinate spaces are not equal.
Original **00691E10** links`photo03` to the generated strip with world-transform
preservation (mode1) at **006920E8**, updates it at **00692102**, then copies its
new local+`B0` anchor at **0069233A..00692353** for`0100006B`. Shared list layout
**00655950** writes local coordinates, so capture order matters.
Restored that sequence before cloning and anchor construction. No artwork,
camera, list spacing, shared clone semantics or individual portrait offsets
were changed.

Bounded inspection of the already extracted baseline model records confirms
identity quaternion/unit scale for`panel01` and`photo03..07`. Panel translation
is`(-0.0993696451,0.0466828644,-2.04057216e-9)`. At800x600, the old anchor
therefore produces **+79.495716 pixels X / +37.346292 pixels Y**, matching the
screenshot. First authored portrait anchor is`(74.941349,140.659547)` pixels.
Original horizontal spacing remains float bits`3DB367A1`; the art's five slots
are not perfectly uniformly spaced, so the original list stride differs from
their anchors by at most1.904 pixels, not a new rendering error.

Verification after this correction:

- Strict i386 application-launch/startup-probe targets rebuilt. Engine-window
  does not consume this team header; its previous shared-fix build is unchanged.
- Existing native suite passes.158 Python tests passed at the prior binding
  checkpoint; they were not rerun for this team-only transform change.
- Temporary native ASan/UBSan audit uses the actual five model records, a
  translation-only engine-link fixture (identity rotations/scales verified),
  and the real recovered`clone_menu_frame`. It reproduces the old79.5/37.3 pixel
  error, checks corrected first-anchor/depth preservation, and checks all five
  frame anchors within2 pixels X /0.001 pixel Y of the authored anchors under
  original stride; rendered bitmap bounds are not measured by this audit.
  This is native affine/clone verification, not live engine/desktop verification.
  Temporary source/binary removed; no permanent tests added.
- Frozen function-progress and whitespace checks pass; no new function credit.
- Stage-only preparation completed. Build/staged application SHA256:
  `9d0fb80d597bb018537c7a6314898044ab731fa3674c704ab28209046297f28e`.

Next user check: portraits should sit in the upper-left five photo slots, above
the scrollbar. Check scrolling, candidate hover/selection, selected-team slots,
Back/continue, then equipment and relevant briefing/dialog routes. The full
pipeline remains unaccepted; no commit/push and no agent-side Windows execution.

## Desktop follow-up — team interactions and text, 2026-11-01

The user reached team selection and reported cursor recentering, no initial
candidate selection, invisible portrait dragging, incorrect squad names/ranks,
forced four-person squads, Jan Kratochvíl text failure, Campaign/Carnage soldier
confirmation failure and scrolling reset after transfers. This follow-up
supersedes the earlier team drag/text/acceptance limitations; equipment floating
drag visuals remain separate. No desktop result for this new build yet.

Evidence and corrections:

- Shared cursor acquisition **0066A6E0** retains the **0066A4E0** initial cursor;
  new preparation frontend owners must not create another `(400,300)` position.
  `StartupSession` retains the actual integrated float coordinates when Main or
  a preparation owner shuts down, and seeds its successor from those coordinates.
  Initial application entry remains centered. Windows input/desktop behavior
  remains for the user to verify.
- Team activation **00691280** ends with **006958A0 → 00694AE0(0)**; the latter
  invokes selection **006948D0** for the first candidate outside the squad.
  Initial `shown()` now selects that candidate and positions **00695A40**'s
  selector. This highlights a candidate without assigning another squad member.
- **00690DE0** sets the squad name using **006752D0**'s user/full-name choice
  and style12 (`BIG`, `.04`), followed by a style2 rank at HumanStore+24.
  The removed second row was incorrectly the soldier's shortened name.
  Static initializer **0069F6E0/0069F790** establishes full rank IDs
  `2792..2799` and abbreviated IDs `27A6..27AD`. The exact installed base code
  at **00690EC6** uses the abbreviated member at `008AE854`; **the user's
  screenshot explicitly requests the full rank**, so this build uses
  `2792 + field24` (e.g. `Corporal`). This is a requested presentation choice,
  not a claim that the installed binary uses the full form. Name glyphs now
  use the original large bold atlas rather than the small font shown in the
  user's reconstruction screenshot. Original text-plane bounds are retained.
- **00693590**'s authored model names are misleading: `baccept and continue`
  has caption2654 **AUTOMATIC SELECTION** and calls **00695450**;
  `bauto assign` has caption2653 **CONTINUE**, calling **00695B30** or opening
  four-photo selection. Existing action-to-caption mapping was correct.
  Removed auto-fill from `accept_team()` and from opening the four-photo modal.
  Explicit Automatic Selection still uses the original policy; one through
  four manually assigned soldiers pass through unchanged. Empty squads keep
  Continue disabled; Lone Wolf retains its one-soldier limit.
- Four-photo Yes emits mission reply `0200006A` and then pop `06001005`.
  Team receiver **00691020** reconciles and queues `02000008`. The native
  `accept_team()` also advances the descriptor immediately; the queued pop
  reactivates the old team screen before `08` removes it. Its old `shown()`
  called `team()` after the flow had already destroyed that owner, producing
  the reported descriptor exception. The team screen now retains its borrowed
  descriptor identity and declines this stale refresh. Real ownership checks
  remain enforced in the flow; the queued pop/advance order is retained.
- Original **006937F0** makes a shallow portrait visual copy, positions it at
  the source world translation plus float `0080F550 = +.1` in Z, hides the
  source, and **00693B60** applies cursor movement. **00693C10** releases the
  copy on drop. Implemented these paths through the checked engine factory/
  copy/link/position methods, with native RAII and an unbound clone pick ID.
  Drag start selects its source, and registered `04008000` updates move the
  portrait. Drop restores the source and performs the existing transfer.
  New static export: `analysis/work/team-fidelity-20261101/006937f0.{c,json}`
  (`decompiled=true`). Other cited exports were already preserved; no original
  executable or DLL was run. Live depth/material/picking remains a desktop gate.
- **00693C10/00694C40** remove/include individual list entries (`02000084/83`)
  instead of clearing the list (`85`). `refresh()` now applies membership
  differences. `PreMissionList` reinserts restored candidates in original
  roster order, retaining its offset except the normal last-page clamp.
- **00648860** accepts legacy bytes, or decoded UTF-8 in wide mode, and the
  bitmap font consumes 256-entry metrics (**006404C0**). The host ASCII-only
  rejection caused Jan's crash. Rendering now converts valid Latin-1 UTF-8
  names to atlas bytes and also accepts original legacy bytes. Both raw ED and
  C3 AD resolve to the installed `í` glyph, including measurement, edit labels,
  squad names and paragraphs. The currently staged ANSI renderer remains
  bounded: characters outside Latin-1 use `?`; full original CE/other Unicode
  font ownership is not newly claimed.

Verification for this follow-up:

- All three strict i386 targets build. Engine-window includes the shared text/
  startup fixes; application/startup-probe were rebuilt after final team changes.
  No Windows executable/original game/DLL/Proton execution by the agent.
- Existing native suite and158 Python tests pass. Whitespace and frozen
  function-progress checks pass; no new function credit. Ledger remains428
  partial (423 client/five engine), zero complete,18,783 frozen.
- Temporary native ASan/UBSan audit loaded the actual baseline HumanStore,
  GameData/catalogue/policies/presets and ANSI Set2 metrics. It passed16 supported
  team routes (standalone modes0/3, Campaign/Carnage1/3, each with squads1..4),
  eight actual queue reply/pop/advance sequences using a fixture adapter with
  the same descriptor-identity guard, and auto-selection/Lone Wolf limits.
  It checked195 name fields/65 rank indices, Jan legacy/UTF-8 vertex equality,
  both font glyphs and16,384 high-byte pairs. Portrait loading was mocked;
  default-preset writes stayed in fixture memory. This is native verification,
  not execution of the Windows team screen or its engine methods.
- A second temporary ASan/UBSan audit executed the actual `PreMissionList`
  class with native frame/queue fixtures:500 scrolled remove/return transfers,
  natural reinsertion order and last-page clamping passed. Rendering and input
  callbacks are not desktop-tested by this fixture.
- Baseline HumanStore member9170 bytes, SHA256
  `e070e9b0f8f1489ce0ff40316726916afed5bfe5cb58e2e4dcfe06ace073cb5f`.
  Temporary audit source/binaries/fixtures and extracted HumanStore were removed;
  no permanent tests added. Final staged hash and desktop checklist are in
  [HANDOFF](../HANDOFF.md). Full pipeline remains unchecked, no commit/push.

## Desktop follow-up — name inset, drag depth and drop boundary, 2026-11-01

User says “Good, remaining issues” after build04f37c29…20a6a and identifies
three fidelity problems: the name above stats is offset right, dragged portraits
are under borders, and adding a dragged candidate requires moving too far right.
This is positive user feedback for the preceding fixes; it does not establish
exhaustive full-pipeline acceptance.

- **Name above stats:** the exact `bnamexx` builder **00691CB0** sends
  `02000073,1` after setting style12. This override was missing from the native
  definition. `ProfileEdit` defaults to inset10; at800 pixels the text padding
  formula truncates10×800/1024 to7 pixels, while inset1 truncates to0.
  Restored the original event on this team edit only. Original text bounds,
  typeface and the accepted Profile editor remain unchanged.
- **Dragged portrait depth:** the previous follow-up recovered drag-begin
  **006937F0**, including sourceZ+.1, but incorrectly kept that depth during
  movement. **00693B60** adds cursor displacement to X/Y **and replaces Z**
  with the third component returned by **0066A150**. Instruction **0066A168**
  explicitly writes `BF800000` (**-1**). `move_drag()` now uses the existing
  `cursor_frame_position` projection, which matches0066A150's normalized
  displacement and -1 Z. The first native drag placement uses that foreground
  depth immediately, preventing an under-border first frame as well. The
  initial sourceZ+.1 is not claimed as the movement depth. With the retained
  camera atZ=-15.5, this places the moving copy in front of border/source planes.
- **Drop area:** **00693C10** compares normalized cursor X with float
  `0080F518=.5`; candidate drops atX>=.5 transfer even with no squad control
  under the cursor. A control pick identifies a specific existing squad slot;
  **00690BD0** recognizes the slot's image/name/rank IDs. The native code was
  incorrectly requiring a squad-family pick for any candidate assignment.
  Restored candidate transfer forX>=400 at800x600, and the matching squad
  removal boundaryX<=400. Existing slot swaps remain in place. No guessed
  expanded rectangle, manual pixel shift or model translation was introduced.

Evidence: previously preserved original builder/drop/movement exports, plus
`analysis/work/team-drop-depth-20261101/{cursor-delta.asm,drag-move.asm,verification.json}`.
The new instruction anchors were read from the fingerprinted client without
executing original code. The user report contradicts the earlier movement-depth
assumption; this correction supersedes that part of the previous follow-up.

Verification:

- Strict i386 application-launch and startup-probe rebuilt; engine-window does
  not consume the changed team header, so its previous verified build is unchanged.
- Existing native regression suite and158 Python tests pass.
- Temporary ASan/UBSan audit executes the exact production `drag()`/`move_drag()`
  method bodies with native engine/control fixtures and the actual recovered
  `PreMissionTeam`:108 candidate drop/pick combinations across both sides and
  the exact midpoint,10 squad detach/swap cases,1000 foreground movement updates
  and source restoration/clone release pass. It checks the real team definition
  contains the inset override and uses the production text projection to verify
  the7-pixel correction. This does not execute the Windows class/engine methods
  or prove desktop appearance. Temporary source/header/binary removed; no
  permanent tests added.
- Stage-only preparation completed. Final build/staged SHA256 is in
  [HANDOFF](../HANDOFF.md). No new function credit (428 partial, zero complete;
  frozen18,783), no agent-side Windows/original DLL/Proton execution, no commit/push.

Next user check: name starts at the authored left edge above stats, dragged
portrait stays above borders, and a candidate can be dropped in blank space
immediately right of the screen midpoint. Confirm reverse drag, slot swaps and
scroll retention. Full equipment/briefing/handoff acceptance remains pending.

## Desktop acceptance — team-selection checkpoint, 2026-11-01

User confirmation: “Alright; all fixed, I see no more issues now. Record it,
commit and continue.” Accepted application build/staged SHA256:
`6fe0fd2b46361e3955b1582f16dd00aa540f627d0c261c4cefa00177b0192c4e`.
This accepts the observed team-selection checkpoint and the reported corrections,
including name-above-stats alignment, dragged portrait depth and the drop area.
Earlier team fixes remain part of that checkpoint. No additional case-by-case
execution is inferred from the broad acceptance statement, and the full
single-player pipeline is not marked complete.

The user explicitly requested: **the loadout screens are made but completely
untested until we can access them**. This is the current loadout-screen status.
Earlier native catalogue/squad/equipment/queue audits and successful compile/link
checks validate their stated source/data contracts only; they are not loadout
screen tests, desktop acceptance, or proof that the production route is accessible.
Equipment fidelity issues and any later construction/interaction failures remain
open for that future access and test session.

The user authorized committing the current integrated pipeline/team work,
including loadout code with this explicit untested status, and then selected
**Multiplayer menus** as the next work while loadout testing is deferred.
Keep the section3 single-player checkbox and equipment/further flow gates open.
No gameplay/save receiver, fabricated unlock, or successful world-loading service
was added to make an inaccessible screen appear delivered. No push authorized.

## Portrait click and drag-outline follow-up — 2026-11-01

After the accepted team checkpoint, user review clarified that member portraits
must select silently. Candidate IDs04101010+n*10 and squad image IDs04803000+
n*3000 now preserve their normal selection events but return7 from the native
image click adapter, avoiding0066B990's accepted-result1 call to0066B110. Other
image users retain their existing click policy.

Original006937F0 invokes006948D0 to select its drag source, creates a shallow
portrait copy and hides the source.00695A40 parents the selector to that source
at local(0,0,-.1). The source's hidden hierarchy therefore hides the outline
during dragging without copying it into the ghost.00693C10 restores the source
on canceled drop. The native drop handler incorrectly hid the selector on every
drop; it now retains it for canceled/rejected roster drops.

The user additionally confirmed that a successful roster-to-squad drop leaves
the outline at the soldier's former roster position. Before refresh removes the
roster entry, the host reparents the selector to the team model with world
transform preservation and keeps it visible. Canceled drops keep the original
source parent. Successful squad reorder retains the existing hide policy.
This user-observed position contract supplements the preserved static exports;
no original executable or DLL was run. Actual rendering remains a desktop gate.

Strict i386 application-launch/engine-window/startup-probe and existing native
regressions pass. Temporary ASan/UBSan audit uses the exact production image
click and drag-end bodies with native state/scene fixtures. It checks silent
click activation,20 combinations of side/boundary, source side and squad capacity,
plus squad reorder. Successful assignment preserves the selector by reparenting;
canceled/full-squad drops retain its source, and transfer out clears it. Log:
`analysis/work/team-drop-depth-20261101/outline-click-audit.log`. Temporary audit
source/executable removed. No new frozen-function credit (559 partial unchanged),
desktop acceptance, commit or push. Re-test click sound, hidden drag outline,
canceled-drop restoration and the stationary outline after squad assignment.

## Portrait text layering and drop refresh — 2026-11-01

User review found moving portraits rendered beneath text and suspected a brief
source-portrait reappearance on assignment. Original006937F0/00693B60 still
govern shallow portrait copying and cursor translation; depth remains -1 as
previously recovered. The engine scene pass precedes the host bitmap-text pass,
so depth cannot occlude the later text submissions. Team content now supplies a
drag-mask rectangle from the source visual's projected bounds plus cursor delta.
The existing UV-preserving glyph exclusion masks background and description text
under that rectangle. It clears with the transient owner. Source bounds are
cached before drag, avoiding foreground-Z projection differences. Modal overlays
continue drawing above their retained background.

Source inspection found a concrete possible flash:0066BEA0's only queue drain
precedes pointer processing; drag-end restored the source immediately and queued
refresh membership/texture events for the next update. The resulting render
could show the restored old source before list removal. Drag-end now uses a
bounded immediate visual refresh: incremental roster membership and all four
squad names/ranks, image identities and textures route before the next render.
Other refresh callers keep their existing queued behavior; the global event
drain order is unchanged. List insertion/removal still uses02000083/84 as in
00693C10/00694C40, preserving the scroll policy and stationary source outline.
This identifies and closes a native ordering gap; no original or reconstructed
desktop flash reproduction is claimed.

Strict i386 builds, existing native regressions and function-ledger check pass.
Temporary ASan/UBSan runs production raw/refresh and drag-rectangle methods with
native state fixtures: immediate/deferred included/empty squad cases route all
four portrait bindings coherently;441 cursor positions produce the expected
translated mask and exclude overlapping bitmap glyphs. Log:
`analysis/work/team-drop-depth-20261101/layer-refresh-audit.log`. Temporary audit
source/executable removed. No permanent tests, new function credit (559 partial
unchanged), desktop acceptance, commit or push. Staged controller awaits user
checks of text layering, canceled/successful drops and the retained outline.


## Desktop acceptance — Team Selection follow-ups, 2026-11-01

The user's “Good, I can find no more issues” accepts the later silent portrait,
outline retention, drag text-layering and drop-refresh corrections alongside the
multiplayer frontend checkpoint. The accepted executable and scope are recorded
in [the shared acceptance record](multiplayer-desktop-acceptance-20261101.json).
No individual desktop cases are inferred. The earlier accepted13863a7 checkpoint
remains preserved. **Loadout screens are still completely untested until accessible.**
