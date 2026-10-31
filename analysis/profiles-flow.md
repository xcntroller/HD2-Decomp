# Front-end profile management

Status2026-10-31: integrated, native-audited and **user desktop-accepted**. Options/runtime-dialog acceptance is committed as
`015f2c8`. Profile acceptance is recorded below. User explicitly scoped this
checkbox to profiles and persistence; gameplay saves remain with later
controller/in-game deliveries. [Evidence map](profiles-flow-map.json).

## Completion boundary

Main → Change Profile → list/select, create, rename, delete confirmation and Back.
Use the real startup profile owner, loaded settings/bindings and `profile.sav`
serialization. Create requires one of four difficulty choices before returning to
ordinary browsing. Duplicate/general failures use the original shared runtime
prompts and retained editor context. Profile selection invalidates Options'
first-entry synchronization and updates Main's actual profile-name field.
Main's remaining version/save/date eligibility is a separate plan item.

Persistence runs only against the isolated runtime's writable copies. Staging
seeds `PlayersProfiles` once, including empty directories, and writes
`.hd2-profiles-seeded`; later staging preserves creations, edits, renames and
removals instead of resurrecting installed profiles. Existing marked runtime
workspaces migrate without reimport. Interrupted initial seeding can resume.
Installed game files remain unchanged. English physical-directory editing is
implemented; wide-language editing and gameplay-save loading are excluded.

## Original evidence and implementation

Original client SHA256
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`;
LS3DF SHA256
`12c61eed2aec0c45700ad0a5ddfd7cbb3cbf0ea15ec67de155790bb24e7756ee`.
Read-only decompilation exports/disassembly are retained locally under
`analysis/work/profiles-*20261031`. No original executable, DLL or Proton was
executed by the agent.

| Original entry | Reconstructed behavior |
| --- | --- |
| `006211B0`, `006210B0` | Rescan real profile directories; append retained names with style19. |
| `00623780`, `00622E70` | Entry/refresh: current name, list selection, difficulty and enabled controls. |
| `00621470` | Browse/create/rename/new-difficulty modes; trailing ANSI whitespace trim; original validation prompts. |
| `006211E0` | Delete reply, C3 edit retry and Back behavior; native caller implementation, no new absent-function credit. |
| `00623AE0`, `00622A90` | Select actual profile, remember it, refresh settings/Options synchronization and return. |
| `00622AF0` | Accept difficulty1..4 only during new-profile setup; save real payload. |
| `006AE630`, `006AE990`, `006AEC90` | Actual directory create/rename/delete; duplicate versus general errors; recursive file removal. |
| `00657AB0`, `006580B0` | Composite list: internal text endpoint `root|8`, slider `root|4`, low-bit routing and linked scroll messages. |
| `0065BD20`, `0065BE40`, `00643980` | Back-facing text plane, independent front-facing selector, selected-row center and visibility. |
| `0065AD80`, `0065AE90`, `006621B0` | Profile edit visuals/buffer and real BF/C0 character owner; native object ABI. |

The static screen is family2, identifier`08000000`, model`change_profile`.
The [map](profiles-flow-map.json) preserves all17 controls and original initializers.
Edit`08100000` has capacity17 and excludes `./\*?|<>:"`; list`08200000` emits
selection`04001000` and double-click`04013000`. The current-name field uses the
original no-profile label2002. `08D00000` date/time remains empty in this flow.
The original definition spells delete frame`Bdelete profile`, while the actual
asset spells it`bdelete profile`; only this definition uses the corrected alias.

New-profile creation selects the real defaults and writes `profile.sav`, then
holds list/actions/Back disabled until difficulty is chosen. Ordinary difficulty
buttons display the stored choice but remain disabled. Create duplicate/general
failures use labels9500/9501; rename uses9510/9511, shared mode2 and replyC3.
Delete uses mode1, label2004, Yes`02000017`, No0. A retained name survives the
modal context; success rescans and selects the first remaining real profile or
empty state. Empty rename cancels; Escape restores ordinary browsing.

`00622A90` reuses recovered `006B6670` selection/remembering and actual startup
settings/input/audio adapters. Original `006AF1D0` ignores load results; that
behavior does not establish successful malformed-profile loading. Filesystem and
serialization operations remain real and their failure results are preserved.
No successful gameplay receiver or fabricated profile is introduced.

## Desktop feedback and corrected selection

User reported that the rest of profile management worked, but supplied screenshots
showing unwanted date/time, no selected-row background and ineffective row clicks.
This is partial feedback, not full acceptance.

The date enqueue to`08D00000` has been removed. Style19 row names stay black when unhovered, including the selected name.
A follow-up desktop run confirmed the prior corrections and reported one remaining
difference: hovering a profile must turn its name white. The renderer now applies
white only to the hovered row; selection alone retains black text and the original
background. Disabled rows remain gray. A temporary ASan/UBSan audit checks every
selected/hovered pair, including scrolled absolute indices and pointer leave;
`analysis/work/profiles-verification-20261031/hover.log` records the result. The stored difficulty's explicit button selection
color remains white while disabled, matching the original screenshot.

`0065BD20` requires finding`select*` under the record frame, setting direction
`(0,0,+1)`, assigning the record endpoint's pick ID, hiding it, then linking it to
the screen layout root with mode1 (preserve world transform) and updating it.
The former implementation omitted this setup, leaving the selector under the
back-facing text plane with no proper record pick ID. The corrected bridge keeps
world Z and places X/Y at the visible row center, as`0065BE40/00643980` do.
Clear, scroll and hide/show update selector visibility. Pointer clicks resolve the
current cursor row afresh; keyboard activation uses the keyboard-focused row.
This avoids a stale hovered row after keyboard navigation. Selection logs record
the chosen row/name for the next desktop run. Actual engine picking and appearance
still require user confirmation.

## Rapid clicks on different profiles

User clarified that quick clicks on different rows incorrectly returned to Main.
The input timer`0066A220` keys double-click candidates by the shared record endpoint,
so row identity must be resolved before notifying the profile owner.
Vtable`00814298` routes press(+48) to`0065C120`, click(+5C) to`0064DBC0` and
double-click(+60) to`0064DBE0`. Press changes the record's selected index without
emitting ordinary profile selection. Double-click emits`04013000` only; ordinary
click subsequently invokes`0065BF20` and emits`04001000`. Caller`00623AE0` compares
the pressed row name against the previously chosen profile before deciding Back.
Disassembly is retained in the verification directory as`record-press.asm` and
`double-click-dispatch.asm`; the caller's existing read-only export is under
`analysis/work/profiles-filesystem-20261031/00623ae0.c`.

Our former double-click handler called ordinary selection first, making that
comparison always match, even across different rows. The corrected handler sets
the pressed row without notifying the profile owner, emits double-click before
ordinary selection, and rejects invalid/disabled rows. Shared timing is unchanged.
A temporary ASan/UBSan audit composes the actual record methods and profile observer
with recovered pointer timing/dispatch across every pair of rows, quick alternating
rows, repeated same-row double-clicks, slow repeats, the400ms boundary and empty/
disabled rows. Exactly one ordinary selection fires per click. Results:
`analysis/work/profiles-verification-20261031/double-click.log`. Desktop confirmation
of this correction remains pending; no additional function credit or commit.

## Verification

Earlier temporary ASan/UBSan audits passed for the actual profile flow, host
filesystem body through native filesystem shims, actual profile serialization,
BF/C0 editing through the keyboard pipeline, modal reply ordering, duplicate and
general errors, cancel/new-difficulty gating, delete failure/empty list, path guards
and one-time staging. Logs are retained in
`analysis/work/profiles-verification-20261031/{audit,controls,staging-audit}.log`.

The correction adds temporary ASan/UBSan audits of the actual record/list bodies
through `DefinedScreen`, `MenuFrameInput` and the recovered pointer dispatcher:
first entry, same-target switching between Mark/Peter, actual observer callbacks,
keyboard activation with the cursor elsewhere, click after keyboard navigation,
every scroll offset, hide/show, disable and clear. Selector center/visibility and
selected/disabled text colors are asserted. A separate audit extracts the actual
engine bridge methods and verifies direction/pick/link-mode/visibility/update
ordering, retained world Z and absent-selector handling using native call shims.
These establish native contracts, not execution of LS3DF. Temporary audit sources
and binaries are removed after verification; result logs are retained.

Strict i386 application-launch, engine-window and startup-probe compile/link;
existing native suite and158 Python tooling tests pass. Only stage-only preparation
is performed by the agent. The handoff records the final staged executable hash.

Pending desktop acceptance: compare the selected-row background and difficulty
color with the original; select either profile using mouse and keyboard; scroll a
long list; create/difficulty/rename/cancel/delete and duplicate prompts; Back;
reopen/restart to confirm remembered profile and isolated persistence. Do not mark
the plan/family items complete or commit interactive changes until confirmed.

## Function accounting

Eleven newly implemented bodies present in the frozen inventory receive partial
credit; original ABI/general language/game dependencies remain excluded. Absent
callbacks/helpers receive no new credit. Total426 partial (421client/fiveengine),
zero complete; frozen denominator18,783 unchanged. Selector bridge recovery adds
no credit for absent bodies or borrowed engine calls.

## Desktop acceptance

User2026-10-31: “Good, works as expected. Record, commit and continue.”
Accepted application SHA256 `5e6882429e45bd8805e167ee72a8f243feb7d6ca6c8bbe576b028123cac5acf7`. This accepts the
English front-end profile flow after selection/background/hover/double-click
corrections. No separate execution of every suggested desktop case is inferred.
Gameplay saves, wide-language editing and Main's remaining labels/eligibility
remain separate work. Historical pending statements above describe intermediate
builds; this acceptance supersedes them.
