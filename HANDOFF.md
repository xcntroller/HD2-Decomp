# Session handoff - 2026-10-30

## Resume here: archive registration/key exchange recovered; full wrapper still open

Source checkpoint follows `7f35909`. User permits sensible tested commits.
No live host changes or desktop test required. New io/startup_archives.h and
host/startup_archives.h recover the archive region of 006DB6E0, final archive
cleanup in 006DC3A0 and 007E5F33 random arithmetic. See
[archive scope](analysis/startup-archives.md) and checked bounded plan entry.

Creation: three client rand calls BEFORE language/name formatting; opaque
protection call 006DB8C7->[00A59000] and state effects occur BEFORE archive region.
That operation is EXCLUDED, not implemented by a success stub. Separate native
begin-mask/name/create/configure operations do not claim a full wrapper.
Random state is caller-borrowed shared CRT state; no private seed/TLS ownership.

Sixteen slots: 0..4 null reserved; 5 Lang<language>.dta; 6 LangEnglish.dta ONLY
if 5 creation fails; 7 Patch, 8 Models, 9 Missions, 10 Maps, 11 Sounds, 12 Scripts,
13 Others, 14 Maps_C, 15 Maps_U. Calls register directly through original
_dtaCreate@4. Do not invent dtaAddDir/mount calls or include expansion archives.
All keys source-verified against original immediate writes. For every present
slot: mask=(mask<<16)|(rand&ffff), +0C(key^mask), full64 +10 reply, +0C(reply^key).
Do not shortcut handshake or test setter return (original ignores it).
Missing diagnostics include empty reserved slots and intentionally unused
English fallback. Actual DLL first call mixes internal seed, second recovers
compiled key/index; degenerate zero reply is preserved, not normalized.

Shared rw_data lookup: dtaOpen LOW BYTE flags and DLL override 100478DC both
zero => loose first, archive fallback on -1; otherwise archives first. Archives
scan registration order from zero. Language then Patch precede base assets.
Use the SAME module for archives, engine imports and DtaTextFiles. Concrete
adapter checks factory RVA1424, thiscall release+08->1181, set+0C->1073 and
reply+10->1384 (EDX:EAX). Module lifetime borrowed; indexes/codecs/registry
remain delegated. Diagnostic transport is native OutputDebugStringA, not dbgPrintf.

Archive cleanup releases forward, nulls slot AFTER callback. Original phase
runs AFTER 3D/sound/graphics close, before CoUninitialize; native owner handles
only archive refs. Caller must preserve surrounding teardown. Native RAII and
repeat-create guard are explicit differences, with partial-success cleanup.

Audits: all 2,048 availability patterns, fallback/order, random consumption,
false setters, independent two-word mask reference, zero reply, visible slots,
partial create exceptions and repeat-close pass ASan/UBSan. Strict i386 owner,
adapter and actual localized-text-loader composition compiles/links ONLY, never
executed. All 32 immediate key words independently decoded. Keys validate
53,612 ISD0 index/member records in seven base archives; LangEnglish/Patch
ISD1 header/bounds/checksums match (4,560/1,059 entries). Maps_C absent.
Existing native suite, 158 Python tests, host build and evidence checks pass.
Temporary audit sources/executables removed afterward; no permanent tests added.

NEXT: original directory/cache registration, then remaining protection/application/
state services and concrete graphics input services before controller wiring.
006DBFC0 calls archive setup, then 006DBE60, then 006DBFB0->006DB620 localization.
Directory root calls: Maps/Maps_C/Maps_U category0, Models1, Sounds2, Models3/4/5.
Read-only client exports in ignored resource-search-directories include
006ED570/006ED6B0/006ED760 and full cleanup 006DC3A0. Recursion lowercases
composite directory names, filters directory attribute10 and . / ..; root case
is preserved. IMPORTANT: same method 006D7590 uses DIFFERENT owners for category1
(008AEC90) vs5 (008AEBA0); do not collapse their storage based on decompiler.
Other owners/methods: category2 008AEC54/006D8550, 3 008AEC18/006D9640,
4 008AEBDC/006D6B80. Category0 graphics +110->1006FF80->1006FD60. Their native
cache/list implementations remain unrecovered; no caller or service stubs added.

Evidence: python3 -B tools/validate_startup_archives.py verifies three hashes,
24 anchors, six slots, 47 calls, 22 resources; eight negative mutations rejected.
rw_data research imported into NEW /tmp/hd2-rw-data-startup-20261030/RWDataStartup
project (workspace path with '&' is rejected by Ghidra). Subsequent exports were
read-only; original saved projects/binaries untouched. Ignored exports:
rw-data-startup-exports, rw-data-search-exports, rw-data-lookup-exports.
Three new partial client entries: 149 total (144 client/five engine), zero
complete, frozen 18,783. rw_data stays outside census. No agent game execution,
original binary edits or push.

## Earlier checkpoint: joystick list and concrete IJoy boundary recovered

Checkpoint follows `7becea0` input setup/localization. User permits sensible
tested commits. Source-only slice; no live host changes or desktop test needed.
See [joystick scope](analysis/joystick-devices.md) and its new checked plan entry.

New input/joystick_devices.h recovers 006DC640/006DC5E0: fresh GetIGraph/window,
factory(out,instance,window,null,0), signed count snapshot, forward creation,
raw-pointer append without retain. FALSE create result calls +30(1,2) and still
appends a valid output. Actual IJoy failure nulls output, so the original client
would dereference null; native guard throws, never silently skips the device.
Close releases FORWARD with list length visible, clears length, releases base,
then nulls base. Manager owns separate retained refs and releases REVERSE.
Native destructor closes; original raw-vector exit destructor only frees storage.
Repeat initialization requires close; append exceptions balance the new ref.

List poll recovers active 006DCEB0 reverse +58(1) calls BEFORE producer(false).
Do not double poll using producer(true). Existing graphics setup view aliases
the actual list view. host/joystick_backend.h resolves checked GetIGraph and
_CreateJoyBase@20; graphics stdcall(self), IJoy thiscall(ECX self), checked slots
for count/create/base release/device refs/config/poll/current/secondary. Both
modules borrowed; IJoy DirectInput internals explicitly delegated. Adapter is
joystick services only, not complete graphics/keyboard/mouse manager services.

IMPORTANT NEXT PRIORITY: original archive/search initialization and remaining
application/state startup services, plus concrete graphics input services.
Do not spend the next slice on deeper IJoy internals or invent an enumeration
startup call. Complete section searches found NO direct E8 calls or literal
pointers to either lifecycle helper; base global 008AECCC is only referenced
inside them. List starts empty at 006D5D90; setup consumes it and frame polls it.
This suggests dormant creation, though computed targets are not excluded.
Lifecycle callers remain unresolved. Normal empty-list startup is valid.

Temporary ASan/UBSan audit passes ownership, duplicates, zero/negative counts,
factory/null/first/later-device failures, inverted config, allocation-failure
balance, repeat/reinitialize, callback-visible length/base, forward list release,
reverse polling and reverse manager release. Actual manager/producer composition
verifies independent refs and snapshots without double polling. Strict i386
fixture instantiates list and all concrete adapter calls and links ONLY, never
executed. Existing native suite, 158 Python tests and host build pass. Fixtures
removed after checking. Evidence rejects eight hash/byte/slot mutations.

Run python3 -B tools/validate_joystick_devices.py: three hashes, 21 anchors,
36 vtable slots, three calls. Read-only exports under ignored
analysis/work/joystick-list-lifetime; guarded DefineJoystickListFunctions.java.
NO new frozen function credit: client helpers absent from inventory; IJoy is
outside the two-program census. Still 146 partial (141 client/five engine), zero
complete, denominator 18,783. Original binaries and saved project untouched;
no agent Proton/game execution or push.

## Earlier checkpoint: graphics input setup and actual startup localization recovered

New source checkpoint follows committed `46359bd` manager lifetime. User permits
sensible tested commits. No new live behavior, renderer change or desktop test.
IMPORTANT correction: 006DB620's post-attachment services are LANGUAGE/TEXT,
not profile/control loading. See [startup localization scope](analysis/startup-localization.md).

New source: frontend/input_setup.h, startup_language.h, localized_text.h and
host/startup_localization.h. Setup preserves keyboard-init(flags0), manager clear,
joystick-view snapshot BEFORE second graphics query, attach ignoring results,
language resolution and text preload. Registry language cache uses HKLM exact
key/value with english fallback. Cache installs before close; nonempty cache
skips flag recomputation. Japan/Russian or actual font-config existence selects
wide input flag 008AE92C; Text directory's trailing slash produces the original
doubled separator for FontConfig.hxml. Normal self-query/default C-locale case
folding only. Empty recursive language/unterminated buffers fail explicitly.

Text path fully reconstructed through native cache: enumerate executable-root
text/language/*.txt, skip dot/dotdot/dialogs.txt case-insensitively, read relative
DTA paths, ignore file failures, preserve first duplicate ID. Parser uses CR/LF
lines, first/LAST quote, signed decimal prefix, raw bytes, no multiline/escape
processing. Reject ID -1. File gates/UTF16LE rejection and two diagnostics,
UTF8 BOM skip without decoding, and CLOSE ONLY HANDLE ZERO on normal return
are preserved. Native tiny-file/overflow guards and exception search cleanup
are documented. Do not assume original handle semantics or silently fix them.

Concrete Win32 registry/module, CRT file probe/find and original rw_data exports
compile/link for i386 (advapi32), but aren't desktop-executed. Borrowed rw_data
module/archive context remains delegated; original logger is not reconstructed,
native diagnostics use OutputDebugStringA. No successful production stubs.

Temporary sanitizer/native/i386 audit passes all 65,536 byte pairs, 10,000 record
variants, registry cache/fallback/wide rules, path/cache behavior, BOM/tiny-file/
seek/read/size/error/close cases, filters/order/duplicates, and actual manager
setup with changed second graphics pointer and ignored init failure. Independent
C-string/sscanf reference matches 2,162 TEXTY.txt records, 428 TEXTY_DD.txt,
zero license.txt. Concrete platform executable was compiled/linked ONLY, never
run. Existing native checks, all 158 tooling tests, host build (up to date),
evidence/accounting/diff checks pass. Temporary audit artifacts removed afterward.

NEXT: concrete device/list provider and original joystick factory caller at
006DC661 -> 007F8F30 -> IJoy.dll _CreateJoyBase@20 (assembly located, body not
recovered). Determine rw_data's actual search/archive initialization rather than
inventing mount steps. Remaining application Gamedata/HumanStore/profile services,
state owners and full startup still precede controller integration. Profile
persistence, renderer/text consumers and focus/navigation callers remain open.
One bounded source-only input-setup/localization checklist entry is closed.

Map: both hashes, 15 anchors, one graphics slot, 80 calls, 16 resource references;
rw_data import/export names independently checked and its hash recorded, outside
frozen census. Twelve new partial client entries: 146 total (141 client/five
engine), zero complete, frozen 18,783. Read-only exports in ignored
input-setup-application-services and startup-localization-* directories. No
original binary/project edits, agent-side Proton launch or push.

## Earlier checkpoint: native input-manager lifetime and attachment recovered

The new source checkpoint follows committed `5871f3c` installation/defaults.
User authorizes sensible tested commits. No new live behavior or desktop test.
`input/input_manager.h` recovers 006C6A20/6AB0/6B60/6CC0/6C90/6FF0 as a native
owner composing directly with recovered defaults, installer and producer.
`JoystickInput` now stores its retained device; i386 assertions verify record
size 8C and snapshot offsets 04/48. See [manager evidence](analysis/input-manager.md).

Ownership: graphics borrowed, keyboard/mouse creation/close outside manager;
only joystick refs retained at +04 and released at +08. Validate all devices
before mutation; retain forward and append zero snapshots; duplicate occurrences
own refs. Reattach appends and preserves bindings/old refs, replaces borrowed
graphics, sets key gate all ones, limits 400/400/1 and b4 flag zero. Constructor
uses 20/20/20 in normal zero-static context. Binding clear and attachment clear
ONLY four mouse-count bytes, preserving +A4/+A5. Binding clear does not release
devices or publish the scan map. Detach clears outputs before reverse release,
then clears joystick length, preserving tables/counts/gates/graphics/limits.
Destructor frees second, first, binding, joystick allocations in that order.

Temporary sanitizer/native and strict i386 audits pass through original defaults,
owner/snapshot/producer, controller views and frontend Escape/back with held-key
suppression. Audit covers validation, duplicates, reattach, getter/poll ordering,
callback-visible empty outputs, repeated detach, fifth-count preservation and
no implicit map publication. Separate forced first/second append allocation
failures verify balanced refs; tracked frees verify allocation-release order.
Existing native checks, all 158 tooling tests, host build (up to date), byte map,
frozen accounting and diff checks pass. Temporary audits removed after checking.
Native allocation failure balances the just-retained ref unlike original; native
malformed-call guards and vector/object ABI differences are documented. Reference
callbacks must not throw or structurally mutate ownership storage. No successful
no-op backend bodies force the controller live.

NEXT: source-only setup boundary 006DB620 is exported in ignored
input-manager-startup-boundary. It calls graphics +90(flags0), clears/attaches
manager to global joystick list, then resolves a string via 006B67B0 and passes
it to 006B1090. These last services need real recovery; do not replace them with
empty-string/success stubs. Concrete graphics/joystick backend and enumeration,
application Gamedata/HumanStore/profile services, state owners and full startup
still precede controller integration. Profile persistence and focus/navigation
callers stay open. One bounded source-only manager checklist entry is closed.

Evidence: both hashes, 19 anchors, 31 graphics slots, three calls. Six new partial
client functions: total 134 (129 client/five engine), zero complete, frozen
18,783. Read-only exports in ignored input-manager-lifetime, input-manager-release
and input-manager-startup-boundary. No original binaries/project changes,
agent-side Proton launch or push.

## Earlier checkpoint: binding installation, configuration and both defaults recovered

Current source slice follows committed `143f518` producer work and `4e0d923`
confirmed pacing/keyboard/preprocessing. User authorizes sensible tested commits.
No live interaction changes and no new desktop test is required for this slice.

New headers: `input/binding_installation.h` (006C7480/006C7440),
`binding_configuration.h` (006A8600/8630/8690/86C0/8740), and
`binding_defaults.h` (006A8BE0/8C00/006AA090). Both real compiled layouts have
92 records each; selector one chooses alternate, all others primary. Source
preserves reverse mode/dirty application, ignored failure/dirty clearing,
byte-wrapping counts, map callback order and nontransactional install failure.
Old outputs/gates survive binding changes. Default reset clears settings only,
not the input manager or mode. Mode 2 freshly yields 63 output words, mode 3 64.
See [full scope](analysis/binding-installation.md).

All 184 compiled native records match an independent bounded assembly decoder
and exported literal assignments, including register-clobber checks. Temporary
ASan/UBSan audit and strict Windows i386 compilation pass. Actual original
defaults -> native installer/map/snapshot/producer -> controller views -> frontend
Escape/back path works, with held-key suppression. Failure, reentry, mode and
count-boundary cases are audited. Existing native/tooling/build/evidence/accounting
checks pass (all 158 tooling tests; host build up to date). Temporary audit
artifacts removed after verification.
Original record padding and upper EAX result bytes are not recreated. Replacement/
active setting storage must survive callbacks and growth; no successful no-op
production services enable the controller.

One source-only checklist entry closes binding evaluation/installation/defaults.
The broader launch/controller and shared keyboard/navigation entries stay open.
NEXT: original manager construction/device ownership and graphics attachment:
006D5D50 -> 006C6A20, 006D5D70 -> 006C6AB0; graphics init calls 006C6FF0 at
006DB64B and 006C6CC0 at 006DB675. These references were inspected in assembly;
these bodies are NOT recovered yet. Profile chunk/version persistence 006A8900,
application Gamedata/HumanStore/profile prerequisites, text targets, and focus/
navigation callers remain open. Keep following startup order toward actual
controller integration; avoid disposable successful service stubs.

Evidence map: both hashes, 21 anchors (including complete default constructors),
two engine slots and 191 calls. Ten new partial CLIENT entries: 128 total
(123 client/five engine), zero complete, frozen denominator 18,783. Read-only
exports in ignored input-binding-installation, input-profile-binding-installer,
input-binding-configuration, input-binding-context, input-binding-default-helpers
and input-binding-layout-choice. No original binaries/project changes, agent-side
Proton launch or push.

## Earlier checkpoint: binding input producer source recovered and audited

The user authorizes sensible tested commits. `4e0d923` commits the confirmed
pacing/keyboard device integration and source-only preprocessing/text FIFO.
This subsequent source checkpoint recovers client 006C7870 in
`input/binding_producer.h`: reverse joystick current/secondary snapshots,
optional graphics/mouse/keyboard polling, getters, normalized mouse motion,
persistent source gates and reverse keyboard/mouse/joystick binding evaluation.
`input/keyboard.h` also recovers 1006F4B0 configured map installation and
1006F500 packed snapshot. Actual original bindings/defaults are NOT guessed.
See [producer evidence](analysis/binding-producer.md).

All kinds are represented: keyboard; five mouse buttons and three axes;
eight joystick axes, button bits and four POVs. Preserve ignored poll results,
byte-wrapping shared-source counters, reverse evaluation and original stale
output branches. Analog joystick uses CURRENT snapshot in both output buffers;
buttons/POV use separate snapshots. Native POV/buffer bounds throw rather than
original out-of-bounds access. Exact x87 environment/signaling NaNs are outside
scope. State vectors replace original allocator/class ABI.

No new live behavior. Existing device keyboard path remains desktop-confirmed.
The producer is source-only pending original manager creation/device ownership,
binding installation/defaults and concrete snapshot services. Original main loop
006DCEE4 passes polling flag zero after external mouse/joystick polling. Do not
add duplicate polls or no-op service bodies to force controller integration.

Next coherent dependency: 006C7480 binding replacement/installation (maintains
scan slots, key/mouse counts, +B4 map updates and joystick mode); then the real
startup binding/default installer and device ownership. Its assembly was read,
not reconstructed. Original application startup 006B65D0 still needs real
Gamedata/HumanStore/profile services. Text targets and focus/navigation callers
also remain open. Keep following original startup order toward controller wiring.

Temporary ASan/UBSan and strict Windows i386 audits pass: all packed positions/
state bytes, map guards/aliasing, poll/getter order, 256 count values, source
sharing, mouse/joystick axes/buttons, POVs and original stale cases. POV matches
original x87 status-word branches over 100,000 angles, boundaries/adjacent floats,
infinities and NaN. Actual producer -> controller views -> frontend Escape/back
works, holding suppresses repetition and release/repress rearms it. Existing
native suite, all 158 tooling checks, strict host build and evidence/accounting
pass. Temporary audit files are removed after verification.
Map: both fingerprints, 41 anchors, two engine slots, three calls. New partials
006C7870 and 1006F4B0: total 118 (113 client/five engine), zero complete, frozen
denominator 18,783. 1006F500 is absent from frozen inventory, no new credit.
Read-only exports: ignored startup-input-prerequisites and engine-keyboard-snapshot.
No original project/binary changes, agent-side Proton launch or push.

## Earlier checkpoint: device confirmed; client preprocessing and text FIFO source recovered

The user desktop-confirms keyboard interaction and focus loss/return. The
pacing/keyboard slice remains uncommitted; last commit 8eef91d. Current request
is continuation, not a new commit. No new live interaction in this follow-up.

`frontend/keyboard_preprocessing.h` recovers 0066B3E0 and 0066B380: blocked gate,
prehook/availability order, scan before character callbacks, legacy versus ACP
UTF16 dispatch, all-units consumption latch, modifier shortcut/fallback,
secondary-buffer +B8 Escape and existing fresh-screen/drag/back tail.
`input/text_queue.h` recovers separate engine +A8/+AC FIFO and WM_CHAR insertion
branch. +A8 is NOT +A4 scan translation. Queue slot zero is last result; pending
values use 1..31 and getter shifts before reading. See
[scope and evidence](analysis/keyboard-preprocessing.md).

Source-only: actual application input production, character target, ACP conversion
and diagnostic services are required before integration. Do not wire successful
no-op services. Original has uninitialized upper key DWORD bytes; native forwards
zero-extended meaningful byte. Unterminated 1023-byte source throws rather than
original stack overread; wide storage is initialized. Focus/navigation callers
remain open in plan.md. Next: original 006C7870 input-buffer production and
focus/navigation callers, following original call order toward controller wiring.

Temporary sanitizer and strict Windows i386 audits cover queue capacity/order,
all scan bytes, modifier sides/priority, shortcut, character consumption, UTF16
latch, failed/empty conversion, callback mutation and NaN/signed zero. Map checks
both hashes, 26 anchors, 12 graphics slots, one client slot and three calls.
Existing native checks and all 158 tooling checks pass; live host build is
up to date. Temporary audits removed after verification.
One new partial 0066B380: 116 total (112 client/four engine), zero complete,
frozen denominator 18,783. Other indirect entries absent, no new credit.
Read-only exports: ignored analysis/work/engine-character-fifo-defined and
frontend-key-preprocessing; seed script verifies new slots before transient
function creation. No original project/binary changes, Proton launch or push.

## Earlier checkpoint: keyboard device integration built; desktop test pending

The user confirms recovered frame pacing works. Latest log records the new
10/100 ms clock, all four movie opens and clean 3D/sound/graphics teardown
after 899 frames. New keyboard work is not desktop-confirmed and remains
uncommitted with the pacing slice. Last commit is 8eef91d.

`input/keyboard.h` now also recovers separate left/right modifier flags and
the original fixed scan-table translation. `host/engine_keyboard.h` owns an
original LS3DF DirectInput keyboard: verified +90 initialize/+94 enabled/+98
close, foreground nonexclusive host-selected flags zero/no repeat. It copies
the immutable 63-entry plus sentinel table from fingerprinted LS3DF; native
source handles poll/reacquire, scan/repeat, modifiers and character lookup.
Device creation/data format/cooperative setup and release remain delegated.
It snapshots held keys at initialization and closes partial initialization.

The existing --use-engine-mouse path now enables the keyboard owner for movies
and menu. Duplicate Win32 Enter/Space/Escape activation is bypassed; fallback
paths without engine mouse, including F9 diagnostics, retain message transport.
Client input-manager/preprocessing still open; the complete recovered controller
is not live. Native debug-toggle state does not affect engine debug rendering.

Desktop command remains:
`python3 -B tools/run_engine_window.py --menu-confirm --use-confirm-picking --use-engine-mouse --opening-movies --frames 0`.
Check Escape skips one movie; then menu Enter/Space/Escape and Exit/No/Yes.
Holding keys should not activate successive screens. Check focus loss/return
and normal close. Keypad Enter is audited; user has TKL, no hardware check.
Expected log: 'Recovered keyboard polling enabled', key scan/character records
and 'Keyboard closed' before sound/graphics close. Do not commit before testing.

Temporary sanitizer/i386 audit checks all 64 modifier combinations, 131,072
scan/modifier combinations against original table, duplicate/sentinel lookup,
actual poll/scan-to-front-end dispatch, held keys and read/reacquire failures.
Strict host build, existing native suite, 158 tooling checks, byte map,
frozen accounting and diff formatting checks pass.
Combined frame/input map now validates 29 anchors, 52 graphics slots and timer
call against both fingerprints. Function counts unchanged: 115 partial
(111 client/four engine), zero complete, frozen denominator 18,783. Modifier/
translation functions are absent from frozen inventory; delegated initialization
does not earn credit. Temporary audits removed after checks. Read-only original
exports are in ignored analysis/work/engine-keyboard-services; guarded seed
script now includes verified modifier/translation/ownership slots.
See analysis/frame-input.md and plan.md for remaining startup dependencies.

## Resume here: checkpoint committed; original frame pacing desktop test pending

User requested commit and continuation. **8eef91d** commits the confirmed four
movies → menu path, audited controller and prior common-service closure.
New uncommitted slice: `frontend/frame_clock.h` recovers 006DC950, including
unsigned low-32-bit micros, maximum-before-minimum, fresh baseline per branch
and ignored timer/wait results. `host/frame_clock.h` supplies original Win32
manual-reset event, multimedia event-pulse timer and 100 ms wait. Both loops
now use minimum 10/maximum 100. Sleep(1)/Sleep(16) and mouse-local timer removed;
mouse polling gets the same frame delta. Links winmm. Desktop test pending.

`input/keyboard.h` recovers 1006F190 poll/reacquire, 1006F320 message/scan/repeat
and 1006F470 character consumption. Source-only: actual device lifecycle,
character/modifier and message producer remain services; live keys unchanged.
Important: retry success bypasses debug toggles; retry failure clears keys but
returns success; repeat check is inside scan loop, strict >350/>34 signed
wrapping milliseconds; it can beat higher new scans. Character repeats do not
retranslate. All three engine entries absent from frozen inventory, no credit.

Temporary sanitizer/native/i386 audits pass; strict host build and 158 tooling
checks pass. `frame-input-map.json` validates both fingerprints, 20 anchors,
52 graphics slots and timer call. One new partial client 006DC950: total 115
(111 client/four engine), zero complete, frozen denominator 18,783. See
`analysis/frame-input.md`. Temporary audits removed after final checks.

Desktop: `python3 -B tools/run_engine_window.py --menu-confirm --use-confirm-picking --use-engine-mouse --opening-movies --frames 0`.
Check video/audio pacing, Escape skip, menu pointer/keyboard/Exit/No/Yes and
window close. Do not commit this interactive slice before desktop confirmation.
No agent-side Proton/game run or push. Read-only original exports in ignored
`analysis/work/engine-keyboard-loop` and `startup-input-prerequisites`.
Next: concrete keyboard/input producer services. 006C7870 includes bindings,
axes and joystick logic requiring assembly review; 006B65D0 startup also needs
game-data/HumanStore loaders and profile reset. Do not stub these as success.

## Resume here: controller source audited; opening playback desktop-confirmed

The user confirms the opening-movies path works as intended. Their runtime log
records all four IShow opens (status zero/opened one), menu entry and clean
3D/sound/graphics teardown after 1,259 frames. That live slice is confirmed;
the new controller slice below is source-only and needs no repeat desktop test.
The user requested the current tested/audited checkpoint be committed locally.

`frontend/application_controller.h` now reconstructs 006C0820 startup/reinitialize
gates and prerequisite order, normal 006C0CA0 selection, null-descriptor 006C0EF0
owning replacement, shared 006C20F0 frame/input/update/post-update order and
normal video completion, 006C3100's single accumulated 100 ms tick, 006C3190
render gating, 006C31D0 input forwarding, 006C72B0 non-consuming buffer views,
and both flag-gated finalizers. Current-state pointers/status are reread across
callbacks. Installation precedes initialization; its null-descriptor result is
ignored. Frame phase resets after the finalizer callback. Suspended updates
still consume a fixed tick and do not add a render gate.

This controller is NOT wired into the host. Application prerequisite/profile/
command-line, mouse, delay and frame-service bodies, actual input-buffer
production and concrete movie/menu state owners are required contracts. Other
descriptor/game/credits and retained/restart/exit movie routes explicitly fail.
Native ownership clears the original dangling released pointer before allocation;
empty/allocation failures throw. Full 006C0920 global/device cleanup is absent.
Do not supply successful no-op backends merely to enable controller integration.

Temporary ASan/UBSan and full Windows i386 audit compilation pass. Audits cover
all startup paths, four-video transition ordering, installation-before-init,
ignored false initialization result, shutdown/input/update callback reentry,
both input queries, render gates, suspended/large/wrapping time, delay expiry
and finalizer masks. All 158 existing tooling checks, frozen accounting and
`git diff --check` pass. `application-controller-map.json` validates 13 byte anchors,
26 state vtable slots and 14 calls against the original client fingerprint.
Ten new PARTIAL entries: total 114 (110 client/four engine), zero complete,
frozen denominator 18,783. Temporary audits removed after final checks.
Read `analysis/application-controller.md` for exact scope and the next backend
dependencies. The launch checklist entry remains open until integrated.

Read-only/no-analysis controller helper exports are in ignored
`analysis/work/launch-controller-helpers` and `launch-controller-finalizers`;
existing startup/controller exports remain usable. No agent-side Proton launch,
original asset/binary changes, baseline database edits or push.

## Confirmed checkpoint: opening movies into compiled menu

The user prioritizes original startup → movies → compiled menu toward a
decompiled game, with minimal disposable host glue. New `--opening-movies`
requires `--menu-confirm` and runs Logo1/Logo2/Logo3/Intro before constructing
the existing compiled menu. The user has now confirmed this live slice.

Correction to the preceding research: 004336C0/00899728 is a separate music/audio
service, not the video factory. Original 00688040 gets IShow through IGraph +38
→ engine 10069E80 → 10070D30, vtable 1009D9C0. It imports BINKW32 BinkOpen and
BinkSetSoundSystem/BinkOpenDirectSound. `host/engine_video.h` verifies its slots
and owns close/release. `frontend/opening_playback.h` recovers ordinary video
completion, status 2→13, Escape-only skip, sizing and sequence transitions.
The host transports Win32 keys, filters repeats, initializes sound first and
constructs the menu after playback. DLL codec/render/audio remain delegated.
Original startup/controller, subtitles, music coordination, language/preferences
and graphics keyboard polling remain open. The broad startup item is unchecked.

Desktop command:
`python3 -B tools/run_engine_window.py --menu-confirm --use-confirm-picking --use-engine-mouse --opening-movies --frames 0`
Check natural playback/sound, Escape skips one movie, Space leaves ordinary
logos playing, menu entry and Exit/No/Escape/Yes. A separate close during a
movie checks early cleanup. The first error and runtime host log are useful.

Strict i386 host build, existing native suite and 158 Python checks pass.
Temporary ASan/UBSan/i386 playback audit checks state ordering, aspect equality,
failure/paused/completion/skip and cleanup. Temporary staging fixtures check
repeat copies and modified/symlink destination rejection. Staging-only real
preparation copied four Bink files (about 87 MiB) into ignored run/GameMovie;
no agent-side Proton/game launch. `opening-video-map.json` validates both hashes,
17 anchors, 56 engine slots, eight client slots, six calls, four movie names.
At that checkpoint counts were 104 partial/zero complete. Temporary audits removed.

Guarded `DefineOpeningVideoFunctions.java` seeds verified indirect targets only
in read-only/no-analysis sessions. Exports live in ignored
`analysis/work/opening-engine-video-defined`, `opening-engine-video-draw`,
`opening-client-video-defined`, `opening-client-video-body` and earlier movie
directories. Unseeded exports stopped at missing functions; the seeded exports
succeeded. Never run two Ghidra sessions on the same project concurrently:
the attempted client export hit the project lock and was rerun successfully.
No original binaries/assets or saved research project were modified.

After desktop confirmation, continue subtitles/music dependencies and original
controller/lifecycle recovery at the menu boundary. Do not call this full
WinMain/startup recovery. See `analysis/opening-video.md` and `plan.md`.

## Resume here: shared screen entry closed; source-only common services uncommitted

User desktop-confirmed factory/dialog work (732 frames, clean teardown) and
requested commit. It is committed as **6782cf0**. New source-only slice closes
the shared native screen creation/binding/lifetime/stack/dispatch contract.
See `analysis/menu-screen-foundation.md` for the exact completion boundary.
The plan entry is checked with unused service branches explicitly source-only.

`frontend/common_event_services.h` and the real queue now recover all remaining
0066c950 common cases: resource release before saved-index erase, model/argument
gated focus notifications, LIFO clear/state 3/2, direct reset before state 10,
unique/nonzero registrations, capture-owner release/acquire and inverse
application/input mode order. Resource release callback can mutate the vector;
original rechecks current size but erases its saved index. The original
006dcad0 receives inverse mode in CL; cached decomp omitted this argument.
Capture events use typed retained adapters, not reinterpreted word_0c pointers.
Application/resource/input/picker service bodies remain supplied contracts;
unconnected host services throw, and current live flows emit none of these new
events. This slice enables no new desktop interaction and needs no repeat
desktop test. Source/backend/ABI limitations are documented in the closure note.

Temporary actual-queue ASan/UBSan audit and strict i386 audit compile pass,
including resource/capture callback reentry, retained owner lifetime, mode
changes during callbacks, direct versus pending ordering and saved receiver
after picker-induced stack mutation. Host build, native suite and 158 Python
checks pass. New map validates 23 anchors/11 calls. Counts unchanged at 104
partial, zero complete. Audit artifacts removed. New source/doc changes remain
uncommitted; no agent-side Proton launch or push.

Next substantive work: an entire keyboard preprocessing/navigation-caller slice,
or options internal visuals/settings services. The shared screen entry should
not be reopened merely to absorb per-screen builders, application service
bodies, original ABI, resource-cache or device/capture/timing work; these have
their own existing plan entries. Desktop commands for existing flows still work.

## Committed checkpoint: live shared factory and application-back router

User confirms the slot-zero diagnostic and keyboard fix now work. New slice:
`host/menu_screen_factory.h` connects actual main/confirmation engine model roots
to `construct_registered_screens`; products now have separate allocation/binding
and root-initialization stages. `OwnedScreens` retains both named products;
temporary factory definitions die before direct initial main selection. Queue
can start empty for factory startup. Host screen bindings are indexed by ID
family rather than three special pointers. The host retains static original
registry indices 1/4; native owner indices 0/1 represent successful construction
order and must not be confused with them. Unsupported models/builders throw.
The scene/resource bridge still supplies preloaded model roots and session text.

Slot zero now also uses its recovered constructor helper: singleton ownership
is installed before binding, registration follows successful binding, no root
initialization. No additional action is enabled. Both existing desktop commands
remain applicable (ordinary engine mouse and F9 diagnostic).

New `frontend/application_events.h` recovers 00626b20's five service priorities,
fresh current-ID back tests for families 2/14/10/4, generic back and explicit
credits/resume delegation. The real queue uses it. Screen-specific callback
bodies and profile cleanup/credits/resume implementations remain unavailable;
they do not become no-op successful flows. Unknown events still fail explicitly
in the bounded host. Remaining common dispatcher service cases are open.

Strict i386 host build, native suite and 158 tool Python checks pass. Temporary
ASan/UBSan plus strict i386 audit covers startup selection before pending work,
definition teardown, owning registration, runtime construction stages, callback
precedence, all 64 ID-family back cases and real deferred quit/cancel queue.
The new map validates 17 original byte anchors and six calls. 00626b20 is absent
from the frozen inventory, so no census count change: 104 partial, zero complete.
Temporary audits removed. User desktop-confirmed the complete slice and requested
a local commit. Latest log: 732 frames, keyboard confirmation and clean teardown.
No agent-side Proton launch. Earlier no-commit notes below are historical.

Next desktop checks: ordinary Exit -> No, Escape, keyboard confirmation and
Exit -> Yes; separately F9 -> No/Escape, reopen and keyboard Yes. Then continue
the shared entry's remaining common service routes and concrete next-screen
builders; do not mark the broad entry complete merely from this integration.

## Latest fix: keyboard transport independent of mouse mode

User confirms the prompt-style fix works, but keyboard activation regressed.
The diagnostic command excludes engine mouse, while ordinary keyboard adapter
construction and WM_KEYDOWN delivery were still gated by `use_engine_mouse`.
Both are now enabled whenever the registered screen keyboard adapter exists.
Runtime keys retain their subclass route; added handled/scan logging and stop
message processing immediately after a runtime affirmative quits. Strict i386
build and 158 Python checks pass. Desktop retest pending; no commit.
Use the same runtime diagnostic command below and check hovered Exit/Yes/No
Enter/Space, Escape, and F9 dialog activation.

## Latest fix: runtime prompt style support, awaiting desktop retest

User's slot-zero diagnostic reached F9/open successfully, then exited cleanly
after 155 frames with `Unsupported original text style layout`. Engine binding,
four-child construction and 02000096 push succeeded. The glyph style catalogue
omitted runtime prompt styles 24/25. Added their original 00641e30 values:
font 2, black, left/vertically centered, heights .04/.03. No general paragraph
wrapping is claimed. Temporary ASan/UBSan audit submits all four actual runtime
definition captions and style 25 through the real glyph helper; strict i386
build and existing native suite pass. Temporary audit removed. No commit or
agent-side Proton run. Retest with the runtime diagnostic command below.

## Current uncommitted slice: owning live screen products

The user desktop-tested the product ownership migration and reported it works.
Do not commit the current combined interactive slice until the new slot-zero
diagnostic is desktop-tested as well.

New opt-in `--probe-runtime-confirmation` with `--menu-confirm` constructs a
separate original `confirm` engine model, an owning screen product with four
children, copied definitions, attached bitmap text and the recovered runtime
protocol. It registers slot zero in the real queue/stack and routes mouse
clicks, focus, Enter/Space and Escape. F9 opens a localized Yes/No diagnostic;
Yes submits a quit reply, No/Escape dismiss. Pattern frame lookup supports
the original `byes*`/`bno*` definitions against the model's `byes?`/`bno?`
frames. It is gated separately from the confirmed Exit flow and currently
disallows engine-mouse/picking combinations. The application callback that
normally requests this dialog remains unrecovered. The broad screen checklist
entry stays open.

Strict i386 build and 158 tool Python checks pass. Staging-only launch
preparation completed with the extra OK label; no Proton launch. User desktop
command: `python3 -B tools/run_engine_window.py --menu-confirm --probe-runtime-confirmation --frames 0`.
Check F9, hover, Yes/No, keyboard Enter/Space on a focused button, Escape,
reopen after No/Escape, and clean shutdown. If the model binding fails, the
first error and host log are the useful evidence. No new function census credit
is claimed for this host integration.

The live `--menu-confirm` main/confirmation path now builds each screen through
`host/menu_screen_product.h`: one product retains copied definition strings,
controls, focus, frame and event endpoints in destruction-safe order. The
scene, text store and queue remain session services and outlive both products.
The `--menu-ui` path without confirmation still builds only its bounded preview
controls. Original `screen_construction.h` generalized factory is not yet the
live construction path; the new slot-zero diagnostic awaits desktop validation.

Strict i386 host build, existing native suite and 158 tool Python checks pass.
No agent-side Proton run. Earlier diagnostic command:
`python3 -B tools/run_engine_window.py --menu-confirm --use-confirm-picking --use-engine-mouse --probe-groups --frames 0`
Check Exit, No, Escape and Yes with pointer and keyboard and normal teardown.
The repository contains uncommitted host, launcher, staging and documentation
changes. The last committed checkpoint remains `44c2053`.

## Resume here: slot-zero modal protocol audited; engine product integration next

User desktop-confirmed the preceding screen construction/metadata slice; log
shows 380 frames and clean 3D/sound/graphics teardown. User subsequently requested
this session checkpoint be committed. Earlier no-commit notes below are historical.
Current checkpoint includes shared-state closure, screen construction/ownership
and the native-audited slot-zero protocol; slot-zero engine binding is unfinished.

New `frontend/runtime_confirmation.h` reconstructs slot-zero definition/state,
raw/localized opening, key and activation observer paths. Four modes: no buttons,
Yes/No, OK, and Yes/No with style 25. Visibility records precede caption, then
request state, then queued 02000096. Replies are full event codes (target/arg
zero), not callback pointers. Escape queues removal before the negative reply;
button ordinals 2/4 use affirmative, 3 negative, other senders still close.
Inactive/mode-zero consumes every key; unknown mode words are preserved.
Force-close stores inactive before queueing removal and preserves reply words;
normal button/Escape completion clears them. Typed observer contexts borrow
this stable protocol object; backend services and queue outlive it.

New slot-zero construction helper in `screen_construction.h` preserves load,
position, temporary definition, singleton installation BEFORE +8c, insertion
only on success, and definition cleanup. Root +70 is not called. Binding failure
leaves the installed product owned/unregistered. Concrete engine/model/product
services remain supplied contracts. Native allocation failure throws.

Real queue now handles 02000096/97 through shared ScreenStack. Push saves previous
and activates slot zero with focus zero; dismiss checks CURRENT ID FAMILY zero,
hides/removes/clears previous without reactivating underneath. The host still
binds only main/slot-four confirmation; slot-zero visuals/input remain disabled.
No new desktop interaction or original application callback was enabled.

Temporary integrated native ASan/UBSan audit and strict i386 compilation pass,
including actual screen/text/event/observer/queue code with fake visual/services,
all modes/edge cases, deferred replies and caption lifetime, ordering, constructor
success/failures and ID-family gating. Strict i386 host rebuild, existing native
suite, 9 frontend and 17 launcher checks pass. No permanent test or agent-side
Proton run. Temporary audits removed after checks.

See [modal notes](analysis/menu-runtime-confirmation.md) and
[map](analysis/menu-runtime-confirmation-map.json): 26 anchors, 5 slots, 44 calls,
4 resource names. Six new PARTIAL entries; key 006691b0 and observer 006693a0
are absent from the frozen inventory and gain no credit. Total **104 partial
(100 client/four engine), zero complete, frozen 18,783**.

New `DefineRuntimeConfirmationFunctions.java` checks vtable/forwarding/callback
bytes before transient definitions. All eight seeded exports succeeded under
read-only/no-analysis and live in `analysis/work/menu-runtime-confirmation-defined`.
Do not trust the first unseeded export's zero status: it stopped at missing
006691b0. No research project edits were saved.

Screen foundation checklist remains OPEN. Next: connect generalized products,
model/child ownership and slot-zero input to actual engine bindings, then recover
remaining application callback/routing services. Avoid claiming the eleven typed
builder hooks implement their actual application bodies. Existing desktop launch
command remains valid; normal Exit/No/Escape behavior is expected to be unchanged.
This checkpoint is recorded in the session commit (`git log -1`). Do not
recommit these same slices or push.

Next-session order:
1. Build a concrete owning screen product around the recovered construction
   helpers, retaining model/children/definition data for their required lifetimes.
2. Connect slot-zero model, text/button endpoints and key routing, then audit
   protocol/queue integration before requesting a desktop test of the new dialog.
3. Recover remaining application callback routes; keep the broad screen checklist
   open until its concrete integration works. Widget visuals/settings remain open.

### Previous slice: shared screen construction/ownership, desktop-confirmed

User requested another foundation checklist entry and explicitly **no commit**.
Current work advances **Recover screen creation, binding, lifetime, push/pop and
general event dispatch**; leave it unchecked. New `screen_construction.h` recovers
00627060's shared factory loop and 00668470's screen binding, including all eleven
typed builder routes, fresh callback-sensitive traversal/ID checks, confirmation
position, child binding before metadata, +8c/+70 ordering, successful insertion,
definition destruction before direct initial selection. Concrete model/resource
services, screen products and application builder bodies remain backend contracts.
No original application address is called from native source.

New `OwnedScreens` owns stable screen products and copied registration strings;
successful insertion order defines indices and does not rewrite IDs. ScreenStack
now supports this owner as well as the fixed host registry, storing each index
explicitly. Registry entries/previous pointers outlive the stack. Native ownership
is not original class/vector/SEH ABI; failed binding reclaims the native product
rather than leaking it. See [screen notes](analysis/menu-screens.md) and the new
[fingerprinted map](analysis/menu-screen-construction-map.json).

Live `DefinedScreenEvents` now owns ID/model metadata instead of borrowing the
factory definition. It accepts mutable initialization lists; const initialization
still copies. Screen definitions have typed root initialization lists, installed
by the host after all child endpoints are bound. Current lists are empty and the
host still uses its existing two-screen construction, not the general factory.
No additional action or original application observer has been enabled.

Temporary integrated ASan/UBSan audit and Windows i386 audit compilation pass;
all builder routes, list growth, changed identifiers, load/bind failures,
exception cleanup, retained callback/payload lifetime, metadata after definition
teardown and registry growth, plus owned-registry stack operations are covered.
Strict i386 host build, existing native suite and 9 frontend/17 launcher checks
pass. Temporary audit files removed after checks. No permanent test or agent-side
Proton run. New map: 10 anchors, 9 slots, 16 calls. Two new PARTIAL entries,
00627060/00668470; total **98 partial (94 client/four engine), zero complete,
frozen 18,783**. All current and previous state-foundation changes are uncommitted.

For desktop sanity, use the existing command and check Exit -> No, Escape/back,
keyboard confirmation and Exit -> Yes, then normal teardown:
`python3 -B tools/run_engine_window.py --menu-confirm --use-confirm-picking --use-engine-mouse --probe-groups --frames 0`
Do not run Proton from the agent or commit without the user's requested testing.
Next within this screen entry: slot-zero modal subclass/protocol and remaining
application routing/callback services, then integrate generalized construction
with real products. Widget visual backends remain another separate open entry.

### Previous slice: shared control-state foundation completed

The plan entry **Reconstruct shared control state, initialization/event messages
and eligibility** is now checked off. This is completion of the common native
contract, not the entire foundation or an original-ABI function completion.
See [closure evidence](analysis/menu-foundation-state.md) and the fingerprinted
[map](analysis/menu-foundation-state-map.json). All base selectors, guarded
virtual setters, frame/text/observer ordering, observer precedence, retained
local/queued events, mutable initialization and fresh child layout passed a
temporary ASan/UBSan audit; actual widget/container derived-state composition
and strict Windows i386 compilation passed too. Eligibility matched the isolated
23 original instructions for all 16,777,216 byte combinations. No Proton/game
launch, permanent test or new visible behavior. Live text/button paths retain
prior desktop confirmation; no repeat desktop test needed for this audit/comment
and documentation slice. Temporary audit files removed after checks. The current
audit/evidence/documentation changes are uncommitted.

Widget visuals, real preference services, general screen factories and keyboard/
capture/timing work retain separate open checklist entries. Next substantive
implementation remains widget internal-part construction, detailed below.
Original-function counts remain 96 partial, zero complete, frozen 18,783.

### Previous options-widget checkpoint

Recursive construction/input slice is user desktop-confirmed and committed as
**354ae55** (372 frames, five-control recursive probe passed, clean teardown).
The options-widget source slice is committed as **76e41da**, with native audit
coverage; engine widget visuals are not desktop-validated.
It adds no live menu action; no repeat desktop test is needed for these source
additions. This top section supersedes the historical checkpoints below.

New source files: `options_widget_base.h`, `slider_control.h`,
`selector_control.h`, `toggle_control.h` and `options_observers.h` under
`recovered/frontend`. They compose actual native state/text/observer helpers
with supplied visual/internal-part/input backends. They do NOT construct engine
widgets or implement real settings storage/apply services. MenuControlKind now
names original kinds 8/10/24; live constructors still explicitly reject them.

Slider 00665010 behavior includes signed/unsigned range limits and wrap, silent
position/step, direct thumb float queries, floor(+0.5) quantization, different
immediate/queued linked-update paths and zero-argument change notifications.
Focus/press/drag, ceil(abs(wheel)/120), parent/range enable gates and strict-<0
repeat timing (300/50) are reconstructed. Track world-bounds conversion and input
locking remain backend work. Selector 0064F550 includes owned items, query
payloads, wrap, removal's stale caption/index<=0 disable and programmatic change
while disabled. Toggle 006546F0 has notifying/silent fallthrough, rereads after
observers, state/half selection and target-nibble focus. Direct toggle activation
has no disabled gate; parent input eligibility is responsible. Original invalid
enabled-empty selector and nonfinite float conversion become native exceptions.

Typed observer helpers reconstruct 0061FCA0 (preset: seven silent values, seven
availability changes, then apply), 006202A0 (five float settings, query before
ID match, music threshold and sound-only click preview), 00620440 (three byte
settings; mouse-only apply) and 00620720 (voice byte and queued slider enable).
They call supplied native services; preference implementations, graphics feature
calculation, persistence and engine apply helpers are still open.

`tools/DefineOptionsWidgetFunctions.java` fingerprint/vtable/registration-checks
the indirect targets before defining them. Use with `-readOnly -noanalysis` and
ExportFunctions.java in the existing research project. Cached exports are under
`analysis/work/menu-options-widgets`, `menu-options-widget-helpers` and
`menu-options-observers`. Some early helper exports stopped at missing callback
metadata; the subsequent verified definition script successfully exported all
four observer functions. Do not repeat the failed command or infer missing code.

New `analysis/menu-options-widget-map.json`: 48 byte anchors, 29 vtable slots,
34 direct calls. Twenty-two new PARTIAL census entries; eleven additional
reconstructed method starts are absent from the frozen inventory and not counted.
Total **96 partial (92 client/four engine), zero complete, frozen 18,783**.
Native ownership/ABI composition receives no separate function credit. The
[options notes](analysis/menu-options.md) detail exact rules and remaining scope.

Temporary native ASan/UBSan and i386 audit checks all three runtimes, actual
state/text/tagged-event/observer code, retained string/query lifetimes, original
edge cases, focus/disable propagation, wheel/repeat and connected settings graph.
Graphics preset delivery changes seven toggles silently and causes no feedback;
apply follows all fourteen queued messages. Native visual/input services are
fake backends; actual engine visuals/settings files were not touched. No new
unit tests or agent-side Proton runs. Remove temporary audit files after checks.

Next substantial slice: recover actual widget internal-part/model construction
00664B90 (slider), 0064F1C0 (selector), 006541E0 (toggle), and their internal
arrow/thumb/half control classes. Pseudocode for these constructors loses stack
arguments; verify instructions before binding visual names, lookup flags and
parent/layout roots. Then connect real preference services (006B6740 and apply/
graphics-feature helpers), complete options definitions and nested availability,
and entry/apply/back contracts before enabling options. Existing desktop command
with --probe-groups stays valid, but current source additions do not change it.

Foundation scope is now explicit in plan.md: deliver readable native shared
behavior alongside real screens; complete original client class ABI is not a
precondition. Keep large slices, no untested live commits and no pushes.

## Previous checkpoint: recursive control factory and container input, desktop-confirmed

Previous group bridge is user desktop-confirmed and committed as **85ba432**
(425 frames, successful probe, clean teardown). Current recursive construction
slice is user desktop-confirmed: five-frame factory probe passed, 372 frames,
clean graphics/3D/sound teardown. Record this tested checkpoint before new work.
This top section is authoritative; earlier sections describe historical state.

New `frontend/defined_control_tree.h` owns kind-five groups and supplied leaf
products, using the existing original factory sequence for every node. It binds
group frame/ID before recursion, searches children under their parent's bound
frame, preserves the separate layout root, and compacts only successful child
owners/endpoints without changing definition IDs. Immutable definitions, backend,
frames and queue must outlive the tree. Native ownership is not original class,
vector, SEH or intrusive ABI. Only text/button leaves and kind-five groups are
supported by the host backend. Leaf layout still uses the existing bounded
renderer; passing the layout root is not a reconstruction of original layout.

New `container_input.h` implements group key 00656AA0 (full child eligibility,
fresh focused-index reads, no Enter/Space activation fallback), full 00656B20
entry (target 15 calls virtual key, maps handled to seven; ordinary target looked
up three times), activation 00656BD0 and exact virtual pointer focus 00656C30.
The native tree composes these with existing shared leave/press/release/drag/
click/wheel helpers, preserving full targets through nested levels. Normal flat
screen input policy is unchanged; nested screen availability/navigation policy
and other widgets are still prerequisites to a real additional flow. 00656B20
is absent from the frozen census, so receives no extra function credit.

Host `menu_control_tree.h` supplies real frame lookup/state and existing leaf
constructors. Existing normal text/button controls now pin their bound leaf
frame; bounds, direction, visibility and normal/active lookup stay in that
subtree, allowing names to repeat across groups. Existing main/confirmation
construction remains the tested flat collection; the new recursive owner is
exercised by the opt-in diagnostic and native audits.

`--probe-groups` now creates five engine dummy frames over three levels, with
duplicate `codex_shared_leaf` names in separate branches, and constructs their
control tree from definitions instead of manual bindings. Checks scoped lookup,
preserved layout root, IDs, initialization, targeted state isolation and nested
input forwarding. Empty end groups deliberately decline focus/key/press and
report the original no-focus click result. Diagnostic frames have no geometry
or user bindings; no extra menu action is enabled.

Desktop command:
`python3 -B tools/run_engine_window.py --menu-confirm --use-confirm-picking --use-engine-mouse --probe-groups --frames 0`
Check captions/hover, unavailable buttons and hidden OK, repeated Exit/No/Escape
and final Yes by mouse and keyboard. Expected startup log:
`Original recursive group factory: five controls, scoped duplicate names, three-level state and input passed`.
User confirmed this slice; the runtime log was inspected. No agent-side Proton run,
new unit tests, untested commits or pushes.

Strict i386 build, existing native suite, 17 launcher/nine frontend checks,
temporary ASan/UBSan mixed-tree audit and i386 compilation pass. Audit uses the
actual recursive tree/state/text/button-event helpers with fake frames/leaves:
three-level mixed children, duplicate names, distinct layout root, failed/missing
child compaction and cleanup, caption/query/layout, three focus lookups, target
15, fresh focused-index reads, leave veto, release filtering and disabled child
activation/key gates. Temporary audit source/binaries are removed after checks.
Factory evidence now 23 anchors/10 slots/two calls; engine evidence unchanged
400 anchors/43 slots. Ledger **74 partial (70 client/four engine), zero
complete, frozen 18,783**. No claim of live mixed nested leaf validation.

Next large dependency after testing: recover options widget kinds 8/10/24 and
their settings contracts, then complete actual options definitions, model edits,
nested input availability, callbacks and entry/apply/back behavior. Use
analysis/menu-options.md and local menu-options-subbuilders exports. Avoid
enabling a partial options page. Credits also needs its application-state/movie/
scroll/music controller, so is not a simple alternative screen push.

## Previous checkpoint: engine group probe, desktop-confirmed

The prior factory slice is user-tested and committed as **4fc2eac** (1003
frames, clean teardown). The new group bridge and diagnostic are now user
desktop-confirmed: the probe succeeded and teardown was clean after 425 frames. Earlier sections
are historical; this section is authoritative.

`MenuScene::create_group_frame` implements the bounded dummy-frame portion of
0066DBF0: driver factory kind 6, parent link mode 0, name, position, retained
owner. It checks the verified dummy vtable and operation results; the original
accepts other frame kinds, has a distinct string/vector ABI and ignores those
operation results. Group owners are released before models and driver. The
opt-in `--probe-groups` creates two named nested dummy frames under the main
model, verifies traversal and direct child relation, binds frame pick IDs via
the recovered container visual helper, and exercises targeted nested hide/show
and state query through `DefinedContainerEvents` using real engine frame
visibility/update calls. These invisible diagnostic groups have no geometry or
input binding. The probe is not the actual options definition or recursive
widget factory; it does not establish options behavior.

Desktop command:
`python3 -B tools/run_engine_window.py --menu-confirm --use-confirm-picking --use-engine-mouse --probe-groups --frames 0`
Confirm menu rendering and Exit/No/Escape/Yes by mouse and keyboard as before.
Startup should log `Original dummy group probe: linked two frames, bound IDs and
routed nested state/events`; absence or a preview failure is significant. After
user confirmation inspect the runtime log and commit. Never run Proton from the
agent, add unit tests, commit untested interaction changes or push.

The five options sub-builders are exported under ignored
`analysis/work/menu-options-subbuilders`; [options inventory](analysis/menu-options.md)
describes six settings groups (controls has two) and widgets/callbacks. Options
entry 0061BEA0 initializes settings before transition, and show observer 006204D0
updates the third caption. Full widget factory, recursive actual model bindings,
focus/input, settings and callback contracts remain open. Prioritize these in a
large cohesive slice after confirmation; avoid enabling incomplete options.

Strict i386 build, existing native suite and 17 launcher checks passed; temporary
launcher audit confirms flag forwarding/mode rejection. Client group call sites
and engine dummy vtable targets are fingerprint-validated. Ledger: **72 partial
(68 client/four engine), zero complete, frozen 18,783**. The new real-engine
diagnostic is desktop-confirmed for this bounded path.

## Previous checkpoint: factory and nested containers, desktop-tested

The previous targeted-tree slice is user-tested and committed as **ea0134f**.
The current factory and container slice is user desktop-confirmed. The log shows
1003 frames, mouse and keyboard transitions, and clean graphics/3D/sound teardown.
Earlier sections describe historical checkpoints; this section is authoritative.

All 17 main-menu and four confirmation leaf controls now share the native
definition factory and initialize once in visual, definition-data, event, then
insertion order. Native host constructors currently cover text and buttons only.
Their visual frames receive original control pick IDs; availability still limits
input to Exit/Yes/No. Static text initialization now runs through this same path.
The host's frame lookup uses its existing verified traversal, not a complete
implementation of original FindFrame flags or error handling. Native allocation
failure throws rather than reproducing original potential null dereferences.

Recovered kind-five UI container initialization, hierarchical identifier
arithmetic and child-first event dispatch have source implementations and
fake-backend audits, but are **not connected to a live engine container backend**.
Container dispatch tries children even for tagged or zero-target events and
retains the child result, unlike screen dispatch. The options helper 0066DBF0
creates an engine dummy frame of engine type 6; that is distinct from UI control
kind 5. Recursive initialization preserves the separate child-search frame and
layout root, rereads the definition child count, and ignores individual child
creation failures. Original widget ABI, full factory and callbacks remain open.

Strict i386 host build, existing native suite, 17 launcher and nine frontend
checks passed. A temporary native ASan/UBSan audit and i386 compilation exercise
the factory's kinds, failure and cleanup ordering, all 21 leaf initializations,
ID wrap/OR behavior, recursive visuals and three-level container event delivery.
The new fingerprinted factory evidence map has 17 anchors, four client vtable
slots and two direct calls. Ledger: **71 partial (67 client/four engine), zero
complete, frozen 18,783**. The integrated leaf path was also desktop-confirmed.

Desktop command:
`python3 -B tools/run_engine_window.py --menu-confirm --use-confirm-picking --use-engine-mouse --frames 0`
Check captions and hover, Exit/No/Escape/reopen and final Yes using mouse and
keyboard. Check unavailable buttons remain unavailable and hidden OK stays
hidden: more frames now receive pick IDs. Expected log lines include
`Definition controls ready` with 17/17 main and 4/4 confirmation children and
`Original static initialization`. Only Exit/Yes/No are enabled; keyboard button
silence remains original behavior. No agent-side Proton run or new unit tests.

Next, recover the engine dummy-frame creation/link/
position bridge and connect a model-backed kind-five container factory and its
recursive controls. Trace input/focus and options settings callbacks before
enabling a complete additional flow. 0066DBF0 creates the frame via driver +54,
links it to its parent via +38 mode 0, names it via +34, positions via +08 and
retains it in a frontend-owned list. Inspect 0061CF60, 0061DA10 and 0065D150.
Profile, last-save and version callbacks remain unresolved. Preserve large slices,
do not commit before user testing, and do not push.

## Previous checkpoint: tested targeted trees

Previous registered-stack slice is user desktop-confirmed as matching the game,
and committed as **335c519** (3320 frames, clean sound/graphics teardown).
The targeted-tree slice is **user desktop-confirmed and included in this committed
checkpoint**. Latest direct log: 507 frames, keyboard confirmation activation,
normal 3D/sound/graphics teardown. User ended work for today; there is no pending
implementation or desktop-test request from this session. Do not restart testing
or recommit the same slice. Earlier checkpoints below describe historical state;
this top section is authoritative.

Next session, continue in a large cohesive slice:
1. Read plan.md and analysis/menu-screens.md, then inspect current git status.
2. Recover nested group/heterogeneous construction needed by another real flow.
   Start with cached options builder 0061cf60 and its group helper 0066dbf0,
   alongside factory 0065d150; do not assume every group is a screen subclass.
3. Trace required original observer/data callbacks before enabling that flow.
   Main show callback 00627520 is cached but NOT installed. Profile/save data
   and version formatting still require their actual contracts.
4. Build/audit a complete dependency group, update plan/evidence, then request
   desktop validation before committing new interactive changes.

Preserve large-slice preference; no agent-side Proton run, no new unit tests,
no commit before user testing, and no push. No gameplay or engine-wide scope
expansion solely to improve function counts.

New frontend/screen_event.h reconstructs 006684f0. Zero target OR tagged code
routes to screen base, even when the tagged event names a child. Untagged
02000016 FIRST calls leave-focus with the CURRENT FOCUSED INDEX (cast to
uint32), then enter-focus with event target only if leave result is nonzero.
This happens BEFORE lookup, including missing/unavailable targets. Lookup runs
twice on a valid target; missing returns false; child result is ignored and
handler returns true. A nonzero screen-family target with child bits zero is
not equivalent to target zero. No inferred focus/default/navigation key bindings.

New dispatch_copied_control_event in control_events.h reconstructs 0064dc30/+74:
retained copy -> child +6c -> return result -> release copy. Direct +6c queries
mutate caller fields; +74 queries mutate only the retained copy. Base layout
0200002c propagates FRESH zeroed local events through +74 for every child.

New frontend/defined_screen_events.h owns native screen properties, observer/
tagged bindings and fixed per-definition message endpoints. It composes actual
ControlStateRuntime (screen hide overridden by 00668440), lifecycle helpers,
hierarchical lookup and emitter-local delivery. Screen initialization accepts
native typed records and rejects raw executable callback address transport via
existing observer_request validation. Lifecycle notifications now traverse actual
native observer/tagged bindings; they are no longer merely diagnostic callbacks.
HOWEVER original main/profile/dynamic screen observer lists are still UNATTACHED.
Do not claim profile/date/save/version data is initialized or original callbacks
executed. Fixed native child slots do not reconstruct mutable original child ABI.

New host/menu_screen_events.h binds all 17 main/four confirmation children:
static labels, blank fields/deferred version, input-unavailable buttons and hidden
OK all receive messages. DefinedControls::message_control_at bypasses host input
availability ONLY for event delivery. Input remains restricted to Exit/Yes/No.
ButtonVisualPair::receive_event preserves the child as local-event emitter.
Static text uses its existing emitter destination. Owners outlive borrowed bindings.

MenuEventQueue now routes nonzero targets via target >>26 into ANY bound screen,
independent of current stack top, after deferred/reply-bit handling. Screen result
ignored, so missing child declines without an application error. Unsupported
registered screen IDs throw before unsafe access. Only one/four runtime-bound.
HostMenuEventActions uses these event-tree runtimes for activation/deactivation;
main/confirmation focus and model bridges share them. New model enable/update
methods separate visibility-only screen lifecycle from base show/update ordering.
Original 00668400/440 themselves have no explicit frame update call. Existing
modal render/update path unchanged. Main input focus is now constructed in all
confirmation modes, while keyboard/engine input stays opt-in as before.

Checks: strict i386 host build; temporary native ASan/UBSan audit + i386 audit
compilation; existing native suite, 17 launcher/nine frontend checks. Audit uses
ACTUAL event tree/state/text/focus/key/control-event/queue helpers with fake
visual backends: all 21 endpoints, mutable text count queries, payload ownership,
input-unbound disabled-text mirroring, root base state/screen hide, fresh copied
layout, root-family/missing targets, tagged/zero base bypass, focus veto/order,
second lookup and ignored child result. Root observer false does not veto tagged
updates. Targeted captions reach the menu behind a modal; deferred targeted text
releases after Yes/pop and is discarded/released on No/Escape. Original Exit/
Yes/No definition/key queue flow included. Temporary audits removed after checks.

Screen evidence map now 24 anchors, five client vtable slots, 178 direct calls,
142 resource references; UI map unchanged 400 anchors/40 engine slots. Existing
frontend evidence and frozen accounting pass. Two new PARTIAL client entries:
006684f0 and 0064dc30. Total **67 partial (63 client/four engine), zero complete,
frozen 18,783**. Generic context/ownership glue earns no extra original credit.
No original class/event ABI, intrusive refcount/SEH, nested/general factory,
slot-zero dialogs, original dynamic callback lists or new flows completed.

Desktop command unchanged:
`python3 -B tools/run_engine_window.py --menu-confirm --use-confirm-picking --use-engine-mouse --frames 0`
User confirmed this slice works. Latest runtime log includes:
`Registered screen event trees ready: 17 main / 4 confirmation children`.
Only Exit/confirmation enabled. Original keyboard silence preserved. Most new
targeted/update/observer paths are native-audit verified until real dynamic lists
are recovered; normal desktop flow exercises shared lifecycle through the trees.
Checkpoint includes the tested source, evidence and docs. Next dependency: nested group/
heterogeneous factory and actual screen observer/data callbacks required for a
complete additional flow. Read analysis/menu-screens.md before choosing; credits
uses application state 12, options uses settings/groups, Cinematics selects seven.

## Previous checkpoint: registered screen stack/lifecycle and dependency inventory

Previous text slice is user desktop-confirmed and committed as **1410782**
(506 frames, clean sound/graphics teardown). Current screen-stack slice is
**desktop-confirmed and committed as 335c519** (3320 frames, clean teardown).
User requests large slices. Do not commit interactive changes until user testing.
No agent-side Proton launches (no window driver); no new unit tests; never push.

New frontend/screen_registry.h records original 14 named definitions, in order,
plus separate runtime dialog slot zero. All raw name/model strings (including
spaces) and IDs checked against original binary and compiled registry dump.
Five/seven are BOTH named SINGLE MISSION 2 and share a model, but 00627060
sends them to DIFFERENT builders: 006391e0 versus 00614140. Cinematics selects
seven. Preserve both. Runtime slot zero is separately created by 0066c1d0;
it is not named confirmation four despite sharing the model.

New analysis/menu-screens.json / menu-screens.md inventory all 14 registrations,
53 static children and 100 initializer call sites in 00623dd0. Includes exact
child kind/name/ID, caption/binding/observer metadata and original code addresses.
Eleven identifier-specific augmentation builders exported read-only and direct
resource references/callees inventoried. These are NOT complete nested control
lists or reconstructed builder implementations. Profile japan check conditionally
adds max-length event; inventory must not be blindly executed as flat data.
Complete reachable/in-game/nested-screen census is still open.

New frontend/screen_stack.h replaces MenuEventQueue's screen_depth counter with
borrowed registered identities. Regular 02000001: hide current, push selected
registered screen, activate focus zero. Modal 02000002: preserve old visibility,
SAVE current to previous (+f8), push, activate focus 10200000. IMPORTANT regular
selection jumps directly to 0066d561, bypassing +f8 store: it preserves previous.
02000005 pop hides before removing, CLEARS previous, then resumes remaining
current screen with zero focus. General 02000003 back pop preserves previous.
Clear helper hides each stacked occurrence in LIFO order; source/audit only,
not wired to application transitions. Empty pops no-op. Native unsupported
bindings preflight before live state changes (deliberate guard vs original ABI).
Only slots one/four bound by host; no additional menu actions enabled.

New frontend/screen_lifecycle.h reconstructs 00668400/00668440 model-enabled
gates and exact ordering: enter focus -> show model -> emit 0400d000; hide model
-> leave focus zero -> emit 0400c000. Results ignored. HostMenuEventActions now
uses these helpers with actual model +e0 flag, existing visibility/update bridge
and shared focus handlers. Old separate show/hide-confirmation action callbacks
removed. Original modal activation requests Yes focus (10200000); the pointer
can change it immediately. Screen observer lists/dynamic callbacks remain
UNATTACHED: lifecycle emitted tags are diagnostic only, not dynamic data updates.
Main model seeded visible at host startup; full original factory/lifecycle ABI,
registry allocation/error handling, targeted event routes, slot-zero dialogs,
application-state transitions and general rendering remain incomplete.

Validation after VSCode crash: strict i386 host build, temporary native
ASan/UBSan audit and i386 instantiated audit compilation pass. Checks visibility
gating/order, full registry traversal, regular/modal/back/pop/resume/clear,
previous-pointer asymmetry, empty operations, unavailable binding preflight,
and actual definitions/DefinedScreen/screen-key/control-events/queue Exit/No/Yes
with automatic modal Yes focus. All 14 compiled names/model keys/IDs match
inventory. Existing native suite, 17 launcher and nine frontend checks pass.
New menu-screen-map.json verifies exact client fingerprint, 13 instruction
anchors, two vtable slots, 178 calls and 142 resource references. Existing UI
map remains 400 anchors/40 engine slots. Two new PARTIAL client entries:
00668400/00668440. Total **65 partial (61 client/four engine), zero complete,
frozen 18,783**. Inventory/generic containers earn no function credit.
Temporary audits removed after checks; generated decompilation stays ignored.

Desktop command unchanged:
`python3 -B tools/run_engine_window.py --menu-confirm --use-confirm-picking --use-engine-mouse --frames 0`
Check repeated Exit -> No and Exit -> Escape -> reopen; final Yes closes.
Try Enter/Space and mouse, moving between Yes/No, holding a button during
transitions, and focus loss/reentry. Watch for stuck highlights or hidden dialog.
The modal push now requests original Yes focus; current pointer routing may
immediately select/clear another target. Keyboard ordinary-button silence stays.
New log includes `Original screen lifecycle: ... tag=0400d000/0400c000`.
After user confirms, inspect runtime log and commit. Do not treat the whole
screen-creation/dispatch plan item as complete. Next substantial dependency:
original screen/targeted observer dispatch and nested group construction, then
choose a flow whose callbacks/data boundary can be completed. Credits changes
application state 12; it is not a simple registered screen. Options queues slot
six through 0061bea0 plus settings-backed messages. See inventory before choosing.

## Previous checkpoint: attached text state and definition-driven captions

Previous shared control-state/event slice was desktop-confirmed and committed
as **8062165** (399 frames, clean teardown). Current larger text slice is
**desktop-confirmed and committed as 1410782** (506 frames, clean teardown).
User requests large cohesive slices. Do not commit interactive changes before
confirmation; do not agent-launch Proton (no window driver). No new unit tests.

New frontend/text_object.h and text_control_runtime.h reconstruct native owned
text records and connect all text message selectors to storage/layout adapters.
Indexed set replaces valid signed indices, otherwise appends; styles truncate
to a byte; mutation marks dirty. Localized set style 40 needs version formatter;
localized append has no such special case. Remove marks dirty even on invalid
indices; valid removal returns TRUE including index zero. IMPORTANT assembly
trap: 006448d3 overwrites the argument stack slot with the record pointer, so
00644938 does NOT test the original index. Clear removes all records; it is not
refresh/relocalization. Typed ControlEvent.text preserves shared string lifetime
without converting original executable addresses into native pointers.

All 17 main/four confirmation definitions carry original caption/style events
before activation bindings. Empty localization 2002 initializes three blank
fields. Version style 40/ID 2006 is explicitly deferred; no guessed version.
Bounded original style catalog covers 10/11/12/13/40. Actual rendering supports
one nonempty printable ASCII record in supported title/body alignment only.

New stable MenuTextObject instances are owned by MenuText and outlive controls.
Static labels initialize once there; button captions initialize once through
ButtonVisualPair's full derived runtime. Disabled state mirrors into the same
text records used by rendering. Glyph submission reads records/font/height/
inset/alignment from attached state, replacing the live hardcoded label table.
Old label table remains only for regression geometry audit. Existing engine
bounds/device/glyph bridges, hover/unavailable color policy and fonts retained.
General paragraph wrapping, original +94 color override, full font API/class
ABI/refcount/SEH, dynamic profile/date/save/version callbacks remain unresolved.
Only Exit/Yes/No are input-bound; other actions still unfinished.

Stager/decoder permit original empty ID 2002 with opt-in include_control_text
(flag --include-control-text). Launcher automatically stages it. Same five
owned assets/manifest format; 13/16-label bundles upgrade to 14/17 as required.
Temporary real-asset audit verified font identity, idempotence and rejection of
a modified owned label file before any writes. Original game/runtime untouched.

Validation: strict i386 host build; temporary native ASan/UBSan audit and i386
compilation cover storage ownership/index/style/dirty semantics, version callback
boundary, every text selector, all 21 definitions, empty/deferred records and
48 bit-for-bit geometry comparisons with original fonts/labels against old
renderer (normal/hover/unavailable). Binary audit verifies 20 original caption/
style pairs. Existing native suite, 17 launcher, 12 text and nine frontend
checks pass. UI map: 400 anchors/40 engine slots; frontend evidence passes.
Nine new PARTIAL client entries (006440f0/00644590/00644200/006443c0/00644510/
00644450/00644890/00644960/00641e30): **63 partial (59 client/four engine),
zero complete, frozen 18,783**. Dispatcher/definition entries extended.
No original full class/font reconstruction claimed. Temporary audits removed.

Desktop command:
`python3 -B tools/run_engine_window.py --menu-confirm --use-confirm-picking --use-engine-mouse --frames 0`
Check caption appearance/placement, hover, Exit/No/Yes, Escape and Enter/Space,
mouse sound, blank date/profile/save fields and hidden OK. Keyboard silence is
original ordinary-button behavior. New log: `Original attached text objects
ready: 21; captions come from definition events`, deferred version formatter,
and original per-button initialization with caption records included.
After user confirms, inspect log then commit. Next large slice: expand complete
screen/dependency inventory and reconstruct screen creation/dispatch needed for
another useful flow; finish remaining factory/dynamic callbacks as dependencies
require. Keep plan.md honest; shared foundation is still broader than this slice.

## Previous checkpoint: shared control state/event layer

Shared-state/event slice is user desktop-confirmed and committed as 8062165
(399 frames, clean teardown). The notes below describe that historical boundary;
attached text has since been implemented as described above. The previous
shared-definition/screens slice is desktop-confirmed and committed as ce61b0e
(log: 800 frames and clean teardown). User requests the largest practical
cohesive slices. Keep plan.md updated and avoid event-sized iterations.

Implemented shared ControlStateRuntime, observer registration/removal, show/hide,
disabled-text mirror sequencing and emitter-owned local delivery. Every created
button now runs definition initialization regardless of input availability.
Hidden OK runs its original 01000004 through shared state -> hide root frame ->
update -> emit 0400c000. Removed MenuScene::hide_confirmation_ok special path.
Other main actions remain input-unbound; original keyboard silence preserved.
Root frame lookup is independent of visual-kind lookup. Dummy vtable 1009BBA8
+24/+30 now anchored to 1001E610/1001CFE0; root/normal/active remain borrowed.

IMPORTANT correction to older notes: 01000008 calls REMOVAL 0065C9A0, not
register_override. Registration 0065CAC0 handles special override tag 04021000.
Ordinary duplicate tags append. Removal scans backward; nonzero callback/tag/
context comparisons OVERWRITE the match in that order (context has highest
priority). It is not AND filtering. All-zero filter removes none. Override tag
clears override first, then still scans ordinary records. Source/tests preserve
this behavior. 01000007 registers; 01000008 unregisters in control_state.h.

ControlEvent now carries a separate typed ControlObserverRequest pointer beside
its string/payload field, preserving native pointer width. Original raw +0C/+10
address words are rejected as callable pointers. Context remains borrowed;
portable event layout is not the original 24-byte ABI. EmittingControlDestination
routes ffffffff local events through the emitting control; other events go to a
supplied upstream queue. Focus/init still cannot invent a missing front-end queue.
Observer false does not veto subsequent tagged events, matching original code.

New text_control_event.h reconstructs COMPLETE selector/argument sequencing of
00661940 and 006551D0, with helper/storage backends delegated. Text 01000099
uses POSITIVE z=1 direction before layout (distinct from static initializer's
negative-z direction). Button overrides check attached text; 0200002C lays out
on normal visual and 02000076 stores 0/ffffffff into text +94. Host still has
no attached client text objects; MenuText renders through its existing adapter.
Known unsupported text cases are explicitly rejected in host. Text-present
mirroring and full text backend dispatch are audited, not integrated rendering.
Do not claim complete original font/localization/layout helper reconstruction.

Checks passed before crash: strict i386 host build; temporary native + ASan/UBSan
and i386 compilation audit using ACTUAL runtime/classes/helpers. Covers all
observer filter-presence combinations, invalid/duplicate/override registration,
typed request lifetime and raw address rejection, all shared state cases,
repeated show/hide with state-before-frame/update-before-notify order, disabled
text present/absent, mutable properties, fresh child propagation, local-event
continuation despite false observer, copied tagged observer registration, ALL
derived text selectors and nullable button overrides, actual definition/queue
Exit/No/Yes and hidden OK initialization. Existing native suite, 17 launcher/
nine frontend checks, frontend evidence pass. Temporary audits removed; no new
unit tests or agent-side Proton run. UI evidence now 371 anchors/40 engine slots.
Seven new PARTIAL client entries: 0065CAC0, 0065C9A0, 0065C760, 0065C790,
00661CC0, 00661940, 006551D0. Totals 54 partial (50 client/four engine), zero
complete, frozen 18,783. Generic context glue earns no extra count.

Desktop command unchanged:
`python3 -B tools/run_engine_window.py --menu-confirm --use-confirm-picking --use-engine-mouse --frames 0`
Check startup/hover, Exit/No/Yes, Escape and Enter/Space, original pointer sound,
other inactive buttons and hidden OK staying hidden. New log should include
`Original control initialization: frame=bok records=1 visible=0 disabled=0`.
The 12 button objects now all initialize through definitions, including inactive
main controls. Read user confirmation and log before committing this slice.

Next cohesive slice: attached client text state/font/layout backend and caption/
style initialization, then first additional screen/dependency inventory. The
current full text dispatcher has portable callback sequencing only; don't mark
font/backend work complete. Preserve larger-slice preference across crashes.

## Previous checkpoint: reusable definition-backed screens

User asks for larger cohesive slices, ideally an entire plan item per turn.
This turn implements the entire "Replace hardcoded Exit/confirmation adapters
with reusable controls and definitions" item for the current two-screen flow.
**User desktop-confirmed: check the plan item and commit this tested slice.** Prior initialization/state slice was desktop-confirmed (435 frames,
clean teardown) and committed first as d152b0c.

New frontend/menu_definition.h: bounded original 00623DD0 data for 17 main-menu
children (eight text, nine buttons) and four confirmation children (text, Yes,
No, hidden OK). Exact IDs/order/kinds/frame names; all nine main activation
bindings, Yes/No reply+pop order and OK hide record. Caption/style/observer lists
are NOT reconstructed here; absent from bounded descriptors does not mean
absent from original game. Existing bitmap text pipeline still renders labels.
Semantic roles are host metadata; availability remains Exit/Yes/No only.

New frontend/defined_controls.h constructs stable owned controls through the
reconstructed factory tail, supplied host visual creator, button +78 no-op,
supported initialization and insertion. Shared collection owns nine main button
pairs and three confirmation pairs; only supported controls bind to screens or
picking. OK still hidden via existing MenuScene helper; untagged derived init
not integrated. Factory/model lookup/kind selection/error ABI remain incomplete.
Host visual errors throw; native provider can report null creation safely.

New frontend/defined_screen.h supplies ALL focus/key/pointer/wheel/target
callbacks for both screens. Same generic context for main and confirmation;
old host/main_menu_focus.h and inline ConfirmationFocus removed. Hierarchical
lookup now accepts both arrays and vectors, with unchanged mask/first-match
semantics. bind_screen validates exact definition identity. Stable unique_ptr
controls outlive screen contexts; unsupported children keep original indices
but are unbound. No new navigation bindings, screen actions or engine ABI calls.
Legacy rectangle/message paths and noninteractive preview use same definitions.

Current checks: strict i386 host build; temporary native + i386 compilation and
ASan/UBSan audit of actual generic classes plus reconstructed helpers/queue.
Covers construction/cleanup/failure, 17/four slots, nine/three button objects,
stable targets, unsupported children/OK, every pointer tag, wheel, focus and
navigation wrapping, full-word/consumed/focus-mutating keys, Exit/No/Yes/Escape,
wrong/duplicate binding rejection and cached legacy hover. No new unit tests.
Temporary compiled-definition audit matches all 17 recorded names/IDs and all
nine event/argument pairs against original binary push instructions. Existing
native suite, 17 launcher/nine frontend checks, UI 351 anchors/38 engine slots,
frontend evidence and frozen accounting pass. Temporary audits removed.

One new PARTIAL 00623DD0 (bounded definition data); extend existing 0065D150
integration coverage. Generic adapters earn no separate reconstruction credit.
Totals: 47 partial (43 client/four engine), zero complete, frozen 18,783.
Read plan.md before next work; do not break larger work into event-sized turns.

Desktop command unchanged:
`python3 -B tools/run_engine_window.py --menu-confirm --use-confirm-picking --use-engine-mouse --frames 0`
Verify Exit hover/click, No/Escape cancellation, keyboard Enter/Space activation,
Yes exit, inactive other buttons, click sound and held transitions. Expect
`Definition controls ready: screen=main-menu children=17 button-pairs=9` and
`screen=confirmation children=4 button-pairs=3`, plus normal clean teardown.
Optional legacy mode checks can omit --use-engine-mouse / use --menu-ui.
Do not launch Proton in the agent environment. Keep current changes uncommitted
until user confirms. No visual differences or new enabled actions expected.

## Previous checkpoint: live button initialization and control state

User desktop-confirms original initialization-list sequencing for Exit/Yes/No
and original state defaults in their eligibility/disabled checks. This tested
slice is ready to commit. Last checkpoint f483551 preceded this continuation.

frontend/control_initialization.h reconstructs 0065CF00: actual mutable record,
live list size/storage each iteration, ignore handler return and continue.
ButtonVisualPair owns portable records and initializes them before marking
bindings ready. Current lists are tagged activation events only and route the
existing original tagged branch. Full untagged caption/layout/observer
initialization is NOT integrated; preserve existing text adapter and hidden OK.
Original button +78 is no-op; no new engine ABI calls or assets introduced.

ControlProperties in control_state.h reconstructs only five 0064DCC0 fields:
disabled=0, visible=1, focusable=1, +58=0, +5C=ffffffff. ButtonVisualPair and
MainMenuFocus/ConfirmationFocus now use actual eligible/disabled state rather
than constant answers. Other main controls remain disabled. Expected visible
behavior is unchanged; no new navigation keys or sounds.

initialize_created_control reconstructs successful-allocation 0065D150 tail:
+2C visual gate, +78 definition +20, +70 definition +10, append. Failed visual
initialization destroys via +80(1), skips insertion, still returns true. Helper
is source-only; kind allocation/model lookup/full factory remain unresolved.

Validation: strict i386 host build; temporary native/i386 audit for live-growing/
shrinking/empty lists, mutable records, ignored false returns, payload lifetime,
factory success/failure order and real button/event-queue Exit/No/Yes flow.
Original state defaults and eligibility/disabled behavior audited. Temporary
files removed; no new unit tests. Existing native suite, 17 launcher/nine frontend
checks, 342 UI anchors/38 engine slots, frontend evidence and frozen accounting
pass. Two new PARTIAL entries (0064DCC0 state subset, 0065D150 tail); 0065CF00
absent from frozen inventory. Totals 46 partial / zero complete / frozen 18,783.

Desktop command:
`python3 -B tools/run_engine_window.py --menu-confirm --use-confirm-picking --use-engine-mouse --frames 0`
Check Exit opens confirmation, No/Escape cancel, Yes exits; repeat using mouse
and focused Enter/Space. Hover, held transitions and mouse click sound should
remain as before; ordinary keyboard activation remains silent. Agent must not
launch Proton. Read user report/log before committing. No new screen enabled.

Next: integrate derived/text definition initialization and observer payload
contracts, then replace more hardcoded screen/control construction using the
factory tail. Keep plan.md current; all partial delivery boxes remain open.

## Previous checkpoint: menu plan and shared control state

User requested a tracker for the ENTIRE menu. Read [plan.md](plan.md) first for
scope, screen families, current status and delivery order. Keep it updated each
slice. Entire base-game menu includes reachable in-game menu variants; gameplay
and wholesale engine replacement are separate milestones. Current priority is
reusable control/definition construction, then one low-dependency complete flow.

Keyboard silence is correct for ordinary Exit/Yes/No in the recovered original
path: concrete front-end +44 = no-op 0067EA80, screen 00668670 calls button +38 /
006552F0 directly. Sound helper 0066B110 is invoked separately after accepted
pointer clicks, not keyboard activation. Direct E8 candidates also include
00620348 in a separate options-control branch; do not generalize to all widgets.
Static binary verification, not an original-game runtime comparison. Preserve
current keyboard silence. Evidence in menu-ui-map.json and RESEARCH.md.

Continued decomp in frontend/control_state.h: source-only base 0065CD40 handler
now handles tagged registration first, ordinary state/property messages,
observer/override delegation, conditional emit and fresh per-child 0200002C
propagation with live child bounds. 0064D9F0 eligibility preserves exact byte
bitwise semantics. Does not alter host behavior yet. Virtual/helper bodies,
class ABI and definition construction remain adapters. Next: compose reusable
control state/initialization with existing bindings and screen adapters; trace
definition factory and screen inventory before enabling more main-menu actions.
Do not invent Tab/arrow bindings while their original callers remain unresolved.

Validation: temporary native audit covers all 16,777,216 eligibility byte triples,
all twelve switch cases, disable guards, masked/unknown/tagged messages, property
roundtrips, fresh child events, shrinking collection and retained payload.
Strict i386 compilation of instantiated handler passes. Audit removed; no new
unit tests. Existing native suite, 17 launcher/nine frontend checks, UI evidence
333 anchors/38 engine slots, frontend evidence and frozen accounting pass.
0065CD40 extends an ALREADY counted partial; 0064D9F0 absent from baseline.
Counts remain 44 partial / zero complete / frozen 18,783. No new desktop test
needed for this source-only slice. No agent-side Proton launch.

Desktop-confirmed queued Escape slice committed as a879727. The user reports
all behavior works except silent keyboard activation, which matches evidence.
Latest log: main-menu Escape/0200000a, keyboard Enter on Exit/No/Yes, mouse
Exit/No, 689 frames, clean teardown. No keypad hardware claim.

## Previous checkpoint: queued Escape

Committed as a879727: original queued Escape/back flow, engine-input mode only.
User desktop-confirmed this slice on 2026-10-29. Keyboard silence is resolved
in the current checkpoint above. Main-menu Escape now
opens quit confirmation; confirmation Escape cancels. `frontend/frontend_key.h`
reconstructs 0066B3E0 tail: offer full key to current screen, if unconsumed scan
01, end active drag with +54(0), clear drag flag, resolve current screen again,
enqueue target zero/code 0200000A/argument screen identifier. Host context uses
shared dispatcher state and both existing screen adapters. First-keydown/repeat/
foreground guards remain host policy. Enter/Space use same helper. Original
key preprocessing, outer gates, application interceptors and object ABI remain
incomplete; no Tab/arrow/default-focus additions.

`MenuEventQueue` adds bounded derived 00626B20 handling for 0200000A against
CURRENT stack depth, not its possibly stale event argument. Confirmation emits
14001200 argument one then 06001005; main menu emits 02000003 argument zero.
Existing deferred quit flow handles the rest. Other preview modes unchanged.
Special screens 2/14/10 excluded. Neither 0066B3E0 nor 00626B20 appears in frozen
inventory, so no count credit: 44 partial / zero complete / 18,783 baseline.
Temporary native/i386 audit of actual helper+queue passed queued open/cancel,
stale identifier/live stack, deferred-quit discard, Yes, consumed keys, full-word
delivery, drag-end/clear/fresh-screen order. Removed; no new unit tests.
Strict i386 host build, existing native suite, 17 launcher/nine frontend
checks, 320 UI anchors/38 engine slots, frontend evidence and frozen accounting
pass. Desktop behavior is user-confirmed; no agent-side Proton launch.

Desktop command unchanged:
`python3 -B tools/run_engine_window.py --menu-confirm --use-confirm-picking --use-engine-mouse --frames 0`
Try Escape on main menu, Escape again to cancel, hold Escape through transition,
held mouse/drag + Escape, mouse/keyboard Exit/No/Yes and audible clicks. Log
should show keyboard scan 01 and event 0200000a, then original push/cancel chain.
User confirms everything else works; latest log records main-menu Escape/back,
keyboard Enter on Exit/No/Yes, mouse Exit/No, 689 frames and clean teardown.
No agent-side Proton launch.

The audible sound slice is committed as 062842e. User confirms works; direct
log shows initialized sound, loaded buffer, buffered mouse Exit/No/Yes callbacks,
Escape, 451 frames and clean 3D/audio/graphics teardown. Codec and staging
validation remains documented below; no sound resources/codecs changed this slice.

Previous checkpoint 062842e: decoded original button.wav and engine-buffer
loading for audible accepted POINTER clicks. User confirms it works. Direct log
records initialized audio, loaded buffer, buffered callbacks for Exit/No/Yes,
Escape, 451 frames and clean 3D/audio/graphics teardown. This tested slice may
now be committed. Codec-32 PCM16 mono/stereo is now independently reconstructed
in `tools/sound_dpcm.py`; extractor supports it with bounds/final WAV-size checks.
Other qualities/widths/header layouts/non-PCM remain unsupported. Decoder uses
fresh block seeds, sign bit seven, magnitude floor(i*i*32767/16129) and wrapping
16-bit stores. Original rw_data 100097F7/table 10037370 are fingerprint-anchored
in `analysis/sound-codec-map.json`; validator checks all 128 magnitudes.

Actual SOUNDS\button.wav equals original decoder byte-for-byte: 5638 bytes,
PCM16 mono/22050 Hz/2797 frames, SHA-256
ced364c301e041d9b7e8c3e78672cb2b967b79c9f2542888fb8c7bf38416500a.
Temporary native i386/GDB differential audit: real WAV plus 18 vectors (78224
bytes), all codes, mono/stereo, first/continuation, extrema/wrap/max blocks.
Eight malformed cases rejected. No DLL init/game/Proton launch. Direct ELF
attempt hit i386 syscall restriction; debugger-stopped pure computation succeeded.
Temporary copied code/vectors/oracle outputs removed; ignored disassembly cached.
No new unit tests. rw_data is outside frozen ledger: counts remain 44 partial /
zero complete / frozen 18,783; do not grant client/LS3DF credit for this codec.

Stage resources now opts into include_click_sound / --include-click-sound,
manifest owns Sounds/button.wav and retains it on default restaging. Engine-input
launcher enables it. Temporary real-archive audit passed 22→23 upgrade, preserved
prior provenance, all hashes, identical default restaging and modified-wave
rejection before writes. Original game/desktop runtime untouched by audit.
Only when sound driver initializes does MenuScene call sound +74 (100545A0)
with path Sounds\button.wav and flags zero; it checks status/buffer +13C.
Three stdcall args, not model-open ABI. Original resource cache 0044A1C0 is still
bypassed; actual engine file/buffer/query/play bodies remain delegated. Existing
frame/scene/driver ownership handles release. Keyboard/hover/wheel unchanged;
driver-init failure retains silent fallback. Audio slice is desktop-confirmed.

Checks passed: strict i386 build, existing native suite, 90 existing Python
checks (52 mission/archive, 12 resources, 17 launcher, nine frontend), UI evidence
312 anchors/38 engine slots, codec evidence eight anchors/all 128 magnitudes,
frontend evidence and frozen accounting. Desktop command unchanged:
`python3 -B tools/run_engine_window.py --menu-confirm --use-confirm-picking --use-engine-mouse --frames 0`
Try mouse Exit → No, Escape/keyboard activation, then Yes. Audible clicks now
expected for accepted pointer activation (assess Exit/No; quitting may cut off
Yes). Logs: `Original button.wav loaded` and `Original accepted-click sound
callback: buffered`. Read direct log and user hearing report before committing.

Previous checkpoint 8f28c2f: original ISnd driver startup/teardown, only
under `--use-engine-mouse`. User confirms it works. Direct log: startup
status zero/initialized one, flags 0402, mouse Exit/No/Yes, 166 frames,
3D driver close then sound initialized zero then graphics close. This tested
slice may now be committed.
`frontend/sound_startup.h` reconstructs 006DB3B0 flag composition: option bytes
06/08 clear, quality index zero gives 0402. These are bounded host defaults,
not recovered user preferences. Original disabled-setting gate/global storage
and obfuscated wrapper remain absent. Parameters verified from assembly are
HWND from graphics +48, rate 22050, channels two, bits sixteen, flags.

`host/engine_sound.h` verifies ISndGetDriver at 10068FA0 and sound vtable
1009DDF0 +04=1007B560, +08=1007A850, +0C=1007A840. Six stdcall words for init;
close/query clean self. Opens before MenuScene so original scene audio setup
can run; declaration order keeps it alive until after scene/frame/3D-driver
teardown, then closes before graphics. Rejects pre-owned initialized singleton.
Failed or partial startup closes before silent fallback; original engine owns
DirectSound/COM internals. Logs `Original sound driver startup` and `Original
sound driver closed`, with initialized state. Frame/buffer remains SILENT.

Read-only archive investigation found SOUNDS\button.wav, 5638 bytes, one
encrypted block, in Sounds.dta (ISD0, 4649 members). Key words low/high are
4FE85106/8D2965CA, inferred from header count/table-size equation and validated
end-index metadata layout. Existing native extractor rejects codec type 32.
No decoded wave or staged sound asset, no game files changed, no checksum or
payload correctness claim. Exact archive hash/metadata is in UI map. Next trace
codec 32 in rw_data.dll/original reader plus loader 0044A1C0 before buffer loading.

Checks pass: strict i386 host build, existing native suite, 17 launcher/nine
frontend checks, UI evidence (309 anchors/37 engine slots), frontend evidence
and frozen accounting. Temporary native/i386 audit passed all 1024 flag
combinations, call arguments, success ownership and failure/partial cleanup;
removed. No new unit tests or agent-side Proton launch. Counts 44 partial /
zero complete / frozen 18,783. Desktop command unchanged:
`python3 -B tools/run_engine_window.py --menu-confirm --use-confirm-picking --use-engine-mouse --frames 0`
Check startup, mouse Exit → No/Escape, keyboard activation, mouse Yes and clean
teardown. Still no audible sound expected. Inspect initialized/status log;
latest run confirms actual audio initialization, not just silent fallback.

Previous tested slice (committed 4bff28a): original click-sound FRAME and accepted-click
helper, only in `--use-engine-mouse`. This is deliberately still silent:
sound-driver initialization and `button.wav` archive/codec/buffer loading are
not implemented; no new sound asset is staged. User confirms it works. Direct
log: sound callback after mouse Exit/No, Escape, keyboard Enter on Exit/Yes,
365 frames and clean teardown. This tested slice may now be committed. Original sound vtable 1009BCB8 +30 is 1001BB10 (not visual
1001CFE0); it forwards self/enable/zero to +AC 10054F30. +78 100548D0 returns
false when buffer +13C is null. All used sound slots and sector +7C 1004DCB0
are fingerprint-anchored. Sector +78 1004D040 removes LIGHTS; do not confuse it
with sound insertion at +7C.

`MenuScene` creates factory kind four, sets type three/name sound_click/disabled/
volume one, adds it to scene and primary-sector sound list, and retains an owning
reference until normal teardown. `frontend/menu_click_sound.h` reconstructs
0066B110: nullable binding, active query with zero, re-read binding, enable with
one only when inactive. `MenuFrameInput` now invokes that helper only after a
screen pointer click returns one, before queue pumping. Keyboard direct
activation stays separate. Counts: 43 partial / zero complete / 18,783 baseline.
Strict i386 host build, existing native suite, 17 launcher/nine frontend checks,
301 instruction anchors/34 engine slots, frontend evidence and frozen accounting
pass. Temporary native/i386 audit passed null/playing/stopped sound, binding
replacement during query, accepted-only dispatch and callback order; removed.
No new unit tests or agent-side Proton launch.

Desktop command unchanged:
`python3 -B tools/run_engine_window.py --menu-confirm --use-confirm-picking --use-engine-mouse --frames 0`
Check mouse Exit → No, Exit → Escape, keyboard Enter/Space, then mouse Exit → Yes.
No new audible sound is expected. Log should include `Original sound_click frame
bound` and `Original accepted-click sound callback`, plus clean teardown. Read
that log before committing. Next trace sound-driver
setup and loader 0044A1C0/008AEC54 before attempting actual button.wav playback.
Cached startup sound setup: `analysis/work/menu-resource-setup/006db3b0.c`;
verify its obfuscated arguments against assembly before choosing parameters.

Previous checkpoint 2ade2d7 reconstructs previous/next/index focus traversal in
`frontend/screen_navigation.h`: 00668580, 006685f0, 006686f0. Original two-pass
wrap order, subordinate-ID mask 000ff000, candidate eligibility, leave veto
(reported handled), and enter argument 15/result ignored are preserved.
No original live binding identified: section-mapped client scan found no E8/E9
sites or absolute-address references for the two traversal entry points. This
does not prove dead code; computed/inlined/runtime paths remain unresolved.
Do not invent Tab/arrow mappings or automatic default focus. These standalone
primitives are not connected to the host; desktop behavior is unchanged.
Temporary native audit with actual main/confirmation ID order passed empty,
all-disabled, wraparound, subordinate IDs, leave veto, ignored enter result and
callback arguments. Strict native/i386 Windows compilation passed; temporary
files removed. No new unit tests or agent-side Proton launch.
Accounting: 42 partial / zero complete / frozen 18,783 baseline. UI map adds
three complete-body instruction anchors (291 total), not completion claims.

The focused Enter/Space activation slice is committed as d87fea4. User confirms
it works; keypad Enter cannot be hardware-tested on their TKL keyboard. Direct
log: main-menu Space (39), confirmation Enter (1c) dismissing No, mouse Exit/Yes,
676 frames and clean teardown. No keypad (9c) or explicit repeat samples logged.
Keyboard transport, navigation/default focus and original object ABI remain
incomplete. Existing engine-input command is unchanged; no new desktop test is
needed for this standalone traversal source.

The preceding shared-input slice is committed as `a49df3a`. User tried every
requested case and confirms intended behavior, including held-button Escape/
drag-off. Direct log retains wheel -480 across screen changes, records No,
Escape and Yes, 861 frames and clean teardown. Raw masks are absent; held-button
outcome is user-reported. Original singleton/refcount/texture lifetime remains
incomplete. No new unit tests or agent-side Proton launch.

The main-menu engine-input slice is desktop-confirmed and checkpointed in the
local commit titled `Route main-menu Exit through rebuilt engine input`.
User instruction: **do not commit until tested and working**. For interactive
changes, wait for the user's successful desktop result before a local commit.
Do not push without explicit approval. Preserve all worktree changes.

- User confirmed main-menu input works. Latest direct log: four main-menu Exit
  clicks at target `05000000` / focus 15, No twice, Escape, Yes, 364 frames and
  clean driver/graphics teardown. The log lacks raw button masks and exhaustive
  drag/focus evidence. Earlier cursor binding and wheel observations still apply.
- Tested slice moves main-menu Exit onto the same rebuilt engine
  polling / frame-update / dispatcher path used by confirmation, only under
  `--use-engine-mouse`. Main-menu model picking tags Exit's normal/active
  visuals with original ID `05000000`. Definition order: seven static labels,
  nine buttons, then version; Exit is child 16 / vector index 15 on screen
  `04000000`. All 17 definition-name pushes and ID arithmetic are anchored.
- `host/main_menu_focus.h` adapts screen focus/click/press/release/drag/wheel
  routing for Exit only. `MenuFrameInput<Screen>` is shared by main and confirm
  adapters; they remain separate state instances, not original shared input ABI.
  Engine mode bypasses message-driven mouse activation for both screens.
  Other modes and disabled controls are unchanged. Inactive-screen adapters
  clear on transitions; outside/poll failure/cancel clears both; reentry requires
  all buttons up. Original absolute pointer tracking, host pre-focus, queue pump
  timing, departure cancellation and disabled double-click remain deliberate
  limits. Main-menu engine picking/input has bounded desktop confirmation.
- Latest desktop cursor log: owned model vtable `1009BA50`, root `CURZOR`,
  child `CURZOR.Rectangle02`, two frames, position/update
  `(0.610000,-0.685000,-1)`, No, Escape, Yes, 553 frames and clean teardown.
  User reports it works. This validates the prior cursor slice, not the new
  main-menu engine picking. Use the same engine-input command below and check
  left/right Exit, No, Escape, Yes, hover/departure and wheel without activation.
- Cursor desktop attempt stopped at ABI guard `slot 0x34, expected RVA
  0x1d330`, before rendering (0 frames, clean driver/graphics teardown).
  Corrected model-specific name setter: vtable `1009BA50` +34 is `100346A0`,
  which calls base `1001D330` then model propagation `10034630`. Do not bypass
  that override. All model slots used by this path are now anchored from the
  model vtable; other expected method targets were correct. Guard diagnostics
  now include actual RVA. Strict build/checks passed; user subsequently confirmed the corrected
  cursor slice works as described above.
- Latest continuation stages original `MODELS\kurzor_menu.4ds` only when
  `--use-engine-mouse` is requested (manifest ownership persists later).
  The model is 330 bytes, SHA-256
  `d29a19d542f7155e14b82e592a642caf974b068926d6bd30cae59e098fd409f2`,
  with existing `X_KURSOR.TGA`; no new texture. Real-archive staging audit
  upgrades 21 to 22 assets, preserves prior provenance, verifies all hashes,
  retains identical files/manifest on default restaging and rejects a modified
  cursor model before writes. All audit files were temporary.
- Engine-input host now loads model kind nine, names its root `CURZOR`, links
  primary sector and hides it. Original scene +7C lookup with flags FFFF must
  return that owned model; +E8 name and setter/update ABIs are checked.
  `frontend/cursor_frame.h` reconstructs the 0066A220 tail projection and
  +08 position / +24 world-update order. The confirmation adapter calls it
  on host-seeded arm/reentry and after processed input, before screen callbacks.
  Visible bitmap cursor, hit/pick ray and other modes are unchanged.
  Cursor binding/update has the bounded desktop confirmation above. Look for `Original CURZOR model bound`
  and `Original CURZOR position/update` logs using the same engine-input command.
- Latest continuation reconstructs wheel getter/predicate, int64 truncation
  and low-DWORD argument passing, focused-screen +4C forwarding, button
  `04011000` with zero argument, and post-callback current-screen repick/+40.
  `--use-engine-mouse` now consumes polled Z. Drag early return still skips
  wheel; double-click activation remains disabled. Repick uses the same frame
  cursor point and the actual confirmation model. User confirmed scrolling
  produces no visible effect. Latest log records Yes target 10200000 wheel
  -120/-240/+120, result 7, event 04011000 with zero argument, 432 frames and
  clean teardown. No No/outside wheel samples are established by this log. Three new
  partial entries bring totals to 38; no complete functions claimed.
- Current goal is a functional, decompiled/rebuilt menu. User authorized
  prioritizing the engine boundary that helps that goal. New next desktop step:

  ```sh
  python3 -B tools/run_engine_window.py --menu-confirm --use-confirm-picking --use-engine-mouse --frames 0
  ```

  User desktop-confirmed it works well (2026-10-29), including right-click
  activation. This matches recovered mask-three screen transitions and
  any-button click tracking; no left-only restriction was added. Direct log
  records No three times, Escape, Yes, one poll failure and recovery, 807
  frames and clean teardown. It lacks raw button masks, so right-button
  identity is user-reported. Focus-loss/drag-off outcomes remain unverified. Omit it to restore established message-driven
  confirmation. No agent-side Proton launch or new unit tests.
- `recovered/input/mouse_poll.h` reconstructs LS3DF `1006F8A0` device
  read/Acquire/retry/clear, capability-based wheel suppression and swapped
  buttons, plus `1006FB30` first-five-button mask packing. Original data format
  is 20 bytes (signed XYZ, eight button bytes). `host/engine_mouse.h` uses
  original +DC setup with zero for foreground nonexclusive flags six, borrows
  the original device for rebuilt polling and calls +E4 for engine-owned close.
  Portable snapshot storage replaces engine globals. Unused nonexclusive Win32
  metric queries and exclusive-mode accumulation are excluded; original device
  creation/configuration/teardown remain in LS3DF.dll.
- New functional flag feeds polled engine buttons into reconstructed
  confirmation pointer update/dispatch/screen callbacks once per frame. Main
  Exit remains message-driven. Position/motion and hover remain host supplied;
  one sampled point supplies hover and click picking. Frame delta uses bounded
  QPC intervals, without original wait/scaling. Button press/release/drag event
  emissions are reconstructed and route through the existing tagged bindings.
  Queue pumps after input. Live double-click dispatch stays disabled; wheel is now reconstructed as above.
  Departure cancellation is retained; focus loss/outside/poll failure reset
  frame input; reentry first requires all buttons up. No activation from stale
  focus fallback. Original captured screen-control objects/helper are absent.
- Frozen inventory lacks the two engine entries and the tiny pointer-event
  button callbacks, so they receive no ledger credit; totals are now 38 partial (three wheel functions added).
- Traced the original timer through cached exports and section-mapped assembly:
  `006DC950 -> 006DCAE0 -> 006C20F0 -> state +14 / 0066BEA0 -> 0066B990 ->
  0066A220`. Source is QueryPerformanceCounter/Frequency converted to integer
  microseconds, a wrapping unsigned low-word difference divided by 1000, then
  bounded to 10..100 ms by the main-loop caller. An optional state time scale
  and remainder carry precede dispatch; its provider semantics remain unresolved.
  The 400 threshold is accumulated update milliseconds, potentially scaled.
- Original queue processing precedes input. Picking precedes input update and
  relative cursor integration; bound screen updates follow input. Do not swap
  this ordering based on the host's current event loop.
- `recovered/frontend/pointer_update.h` now includes idle bookkeeping and
  `pointer_cursor.h` accumulation/clamping. XY sums round to float before
  clipping; reported deltas correct overflow using the original subtraction
  order and long-double intermediates. Wheel accumulates without clipping.
  Original FPU control-word behavior remains absent. Engine frame updates are
  now connected only in the confirmation adapter as described above.
  The observational shadow seeds cursor/ranges from the first available host
  point per entry/reentry and uses host position differences; message timing
  still approximates frame polling. Dispatch reads corrected XY motion.
  Suppression and inhibition stay clear and are not desktop-tested.
- Six screen helpers (`00656CA0/CD0/D10/D40/D70/E00`) in
  `recovered/frontend/screen_input.h` reconstruct press/release/drag/end/begin/
  double-click routing. Invalid focus returns zero for the first five, seven
  for double-click. Release additionally requires hit index to match focus.
  All are partial: portable accessors/callbacks replace original ABI/objects.
  The shadow now maintains its own simulated Yes/No focus and routes through
  these helpers; enter/leave succeed and child callbacks return seven.
  Logs include focus, result, cursor and corrected motion. No live events or
  activation run, and new logging has not been desktop-verified.
- New `recovered/frontend/pointer_dispatch.h` reconstructs
  `0066B990` sequencing including wheel: gate, pick/update, target-change gate, alternate +2D
  branch, mask-3 press/release, captured-control veto, double-before-single
  click, drag begin/end/move and early return before wheel. Context must
  resolve the current screen for every callback. Wheel handling/repick order
  is now reconstructed; accepted-click helper remains delegated. `recovered/host/pointer_dispatch_probe.h`
  runs portable screen routing against simulated controls; it never invokes
  actual screen/control objects or activation. Capture state is absent; wheel is not sampled.
  Consecutive identical focus/drag logs are suppressed. New logging has not
  been desktop-tested. Interactive behavior is unchanged.
- Original mouse poll graphics +E8 is `1006F8A0`: device +24 reads 20 bytes
  at `100D1EC8`, acquisition and one retry follow failure, repeated failure
  clears five DWORDs. Graphics +EC/+F0/+F4 return the stored signed XYZ via
  `1006FA10/20/30`. Opt-in setup/shutdown and rebuilt device polling are now
  added as described above.
  Successful-path cursor warping, absolute accumulation and device/focus
  setup remain unreconstructed.
- Strict i386 host build, all existing native checks, 12 resource checks, 17 launcher checks,
  9 frontend checks, 27 accounting checks, 273 UI instruction anchors, 26 engine vtable slots, frontend evidence and
  frozen accounting pass. No new unit tests or agent-side Proton launches.
  Function totals are **0 complete / 18,783; 38 partial**.

Next session:

1. Continue from cached exports in `analysis/work/frontend-inspection/`,
   `analysis/work/menu-input-route/`, `analysis/work/engine-loop/` and
   `analysis/work/initialization/`. Read the new timing section in `RESEARCH.md`.
2. Main-menu engine input and shared session are committed (19f30e5, a49df3a).
   Focused keyboard activation is committed (d87fea4); keypad Enter is not
   hardware-tested. Continue locating live keyboard/navigation references,
   screen construction/default focus or engine input/resource lifecycle.
   The known source/units do not make the host's timestamps equivalent to it.
   Original captured-control objects, shared input object lifetime, engine
   device cursor positioning and screen/control ABI remain absent.
3. Keep the probe observational. Selector zero means **any** button, not button
   index zero. Do not remove pointer-departure cancellation until safe Win32
   capture and outside-window releases are implemented.
4. Preserve UX/mode gates. User desktop command:

   ```sh
   python3 -B tools/run_engine_window.py --menu-confirm --use-confirm-picking --probe-confirm-picking --frames 0
   ```

Latest desktop evidence is the main-menu input run: Exit target 05000000 /
focus 15, No twice, Escape, Yes, 364 frames and clean teardown. User confirmed
it works. Shared-session continuation was subsequently desktop-confirmed (861 frames,
wheel -480 retained across screens, No/Escape/Yes). Keyboard activation was subsequently user-confirmed: Space on Exit, Enter on
No, mouse Exit/Yes, 676 frames and clean teardown. Keypad Enter is untested. Original CURZOR binding and wheel no-op were confirmed previously;
no double-click dispatch claim is made. Logs are ignored and overwritten at the
next launch.

## Earlier milestones and detailed evidence

The latest desktop `--menu-ui` run **works**: original artwork, cursor and static
English bitmap text render, and the bounded Exit adapter closes the host.
The user confirmed today's horizontal-inset correction **works as expected**.
The user also confirmed original normal/actived button-visual selection:
"Yup, works well. Just as I remember it. Continue". Only Exit responds to hover; the remaining
buttons initialize in their normal state and stay disabled.
Do not claim the original menu dispatcher or complete game has been recovered.

Read [README](README.md), [the text/Exit notes](RESEARCH.md#static-text-and-exit-slice)
and [the UI evidence map](analysis/menu-ui-map.json) next.

## Today's layout correction

- Reused cached Ghidra exports; no fresh export or new unit tests.
- Original text-object constructor `00661B30` defaults byte `+90` to 10.
  Button initializer `006550E0` retains it; plain static-text initializer
  `0065C680` overrides it to 1. The main-menu definitions do not override it.
- Original `00644A80` projects X with an inset of
  `trunc(width * byte_90 / 1024)`: 7 pixels at each button edge at 800x600,
  zero for plain static labels. The host now applies that verified step.
- Text rectangles are separate from raw visual rectangles. Exit hover/click
  bounds, fonts, vertical geometry, artwork and other modes are unchanged.
  Binding logs now include both `bounds=` and `text=`.
- A real-asset C++ geometry audit against the cached rounded runtime bounds
  found all 13 labels fit horizontally (widest button text 204 pixels versus
  approximately 309.4 pixels available after inset).
- Validation: existing native geometry checks passed; strict i386 build passed;
  UI evidence validated 30 anchors, 3 engine and 2 client vtable slots.
  No agent-side Proton launch was attempted (known window-driver limitation).
- Desktop follow-up: user reported "Good work; works as expected. Continue".
  Direct log confirms corrected bounds, Exit activation, 971 frames and teardown.
  This is bounded confirmation, not a claim of pixel-perfect original layout.

## New button-visual slice (desktop-confirmed)

- [Button visual adapter](recovered/host/menu_button_visuals.h) reuses the bounded
  exact-name lookup for all nine normal/actived pairs. It is created only in
  `--menu-ui`; `--menu-model` remains an untouched artwork/cursor preview.
- Cached original code `006550E0` aligns active local XYZ to normal when flag
  `+E0 & 0x80` is clear, then hides/updates active. The adapter reproduces the
  same local-position/dirty-flag bridge; do not substitute a world-space setter.
- Original focus callbacks `00655250`/`006552A0` switch visibility and update
  both visuals. The engine visibility setter is stdcall `1001CFE0`, vtable `+30`.
  One hover sample now selects Exit artwork and caption color in the same frame.
- All unsupported controls remain normal/disabled; Exit hover/leave switches
  `normal07`/`actived07`. Focus loss or leaving the client restores normal.
  Text bounds and click bounds remain unchanged. No new assets are needed.
- Strict i386 build and existing native geometry checks pass. Expanded UI map
  validates 40 anchors, 5 engine and 4 client vtable slots. No new tests/exports.
- Latest desktop log: nine pairs initialized, repeated focused/normal Exit
  transitions, Exit activation, 575 frames and normal teardown. User confirmed
  the behavior matches their recollection. Not exhaustive pixel-perfect proof.
- This is a host-owned focus adapter, not the original dispatcher, focus sound,
  tagged-event routing or confirmation screen. Do not claim those are recovered.

## Current continuation: original quit confirmation

- [UI map](analysis/menu-ui-map.json) now records the original main-menu path
  and binary anchors. Screen 4 is `CONFIRM` / `confirm`; text style 12,
  localization 2004; Yes/No style 13, localizations 2001/2000; `bok` hidden.
  Exact definition names are `byes*` and `bno*`, not exact runtime frame names.
- At screen-stack depth one, Exit queues screen 4 then deferred `22000004`.
  Yes `14001100` releases deferred events after clearing bit `20000000`;
  No `14001200` discards them. Both then emit `06001005` to pop the screen.
  Released `02000004` reaches `0066D33B` and sets front-end state +4 to 4.
- Do not conflate this with `0066C1D0`'s separate common confirmation object at
  front-end+100: its initializer `00668790` uses style 24 and callbacks.
  Separate cached code `00627060` independently confirms screen 4's model
  placement `(0.5,-0.375,-0.5)` at `0062711F..00627147`.
- [Resource stager](tools/stage_menu_resources.py) now accepts
  `--include-confirm` for `MODELS\confirm.4ds` and all five original textures.
  Existing manifests upgrade safely; later default staging retains ownership
  of these optional assets. Only `X_NMNBTN.BMP` overlaps the main-menu assets.
  No automatic launcher or host behavior change; Exit still closes directly.
- New opt-in `--confirm-model` diagnostic stages the original model and opens
  it alone at the verified screen-4 position, logging the loaded hierarchy.
  It is artwork-only, not a reconstruction of modal composition or input.
  The diagnostic was subsequently desktop-checked; its original command was:

  ```sh
  python3 -B tools/run_engine_window.py --confirm-model --frames 0
  ```

  Close with Escape or the window close button. The direct host log is overwritten
  on each launch. It may show uninitialized overlapping visual states; do not
  interpret the artwork-only diagnostic as the original modal composition.
  Strict i386 build, all existing native checks, 29 existing resource/launcher
  checks, launch-mode isolation audit and 61 UI binary anchors passed.
  Do not attempt Proton in the agent environment.
- Desktop diagnostic loaded successfully. The user observed a blank confirm
  dialog with button boxes scattered around: this is the raw diagnostic model
  without text or initialized control visuals, not the normal menu or intended
  final dialog. The runtime hierarchy contains 13 frames: `bok` with
  `actived04`/`normal03`; `shadow01`; `confirm` with child `text`; `bno?` with
  `normal01`/`actived01`; and `byes?` with `actived03`/`normal02`. Literal `?`
  characters are part of the actual runtime names; definition patterns contain
  `*`. The log was still open when inspected, so no shutdown claim is made.
  Return to the regular menu with `--menu-ui`, not `--confirm-model`.
- The diagnostic now initializes all three original visual pairs through the
  same shared adapter as main-menu buttons, and logs projected `text`, `byes?`
  and `bno?` bounds. It still has no prompt, input or original modal
  composition; `bok` is not hidden merely because its control is disabled.
  The user confirmed the updated desktop diagnostic displays the popup and all
  three buttons. Logged bounds at 800x600: text `(288.3,248.6)-(511.2,347.9)`,
  Yes `(296.0,340.8)-(392.2,364.7)`, No `(406.9,340.8)-(503.2,364.7)`.
  The third button is the `bok` visual: it remains visible in this diagnostic,
  which does not implement original control disable/visibility semantics. Its
  appearance is not evidence that the final Yes/No popup should show it.
  The subsequent interactive confirmation was desktop-confirmed as described below.
- New experimental `--menu-confirm` mode builds on original model/labels and
  host-owned bounded mouse input. Exit opens confirmation, Yes exits, No or
  Escape cancels. Usual `--menu-ui` remains unchanged. Original control event
  `01000004` dispatches vtable `+14` to `0065C790`, which hides and updates the
  bound frame: the middle `bok` must be hidden in this Yes/No screen.
  Staged localized prompt/Yes/No use IDs 2004/2001/2000 and existing Set2
  font. Real-font audit found widths 98/23/19 pixels against available
  222.9/82.2/82.3, respectively.
- To run the opt-in experiment on the desktop:

  ```sh
  python3 -B tools/run_engine_window.py --menu-confirm --frames 0
  ```

  It is not the original dispatcher and its keyboard/focus behavior is bounded.
  Existing default mode can be restored using `--menu-ui`.
- Desktop test confirmed Exit opens the popup, No returns to the menu and Yes
  closes cleanly (178 frames). The user reported menu captions over the popup
  and a grey box behind the prompt.
- Original static-text initializer `0065C680` sets the text visual direction
  to `(0,0,-1)` through engine vtable `+0C` before calculating its bounds.
  The experimental confirmation now does the same, using ABI-checked
  `1001C8D0`; the user later confirmed the grey box is gone.
- Original front-end render `0066DE00` draws the previous screen first, then
  disables driver state 5 and draws the current screen in a second pass while
  switching model visibility. The experimental host now follows that ordering,
  drawing main text between the passes and confirmation text after the second;
  state 5 and both model visibilities are restored. No clipping approximation.
  The two-pass rendering and prompt-frame orientation are built and passed
  existing checks. **User desktop-confirmed (2026-10-27): all issues fixed,
  works well** — no caption overlap, no grey box, No/Yes correct.
- The user chose to pivot from extending host glue to reconstructing original
  front-end functions and gradually replacing the scaffolding. The first slice
  is [button focus callbacks](recovered/frontend/button_focus.h): `00655250`
  and `006552A0` sequence visibility, frame updates, tagged event emission and
  return values; entry checks the original `+64` gate. The host invokes these
  through an adapter and assumes that gate is clear. Original object layout
  remains unimplemented.
  Both functions are tracked as partial, not complete. The user confirmed the
  callback wiring still works; the desktop log shows Exit focus transitions,
  No and Yes, and normal teardown.
- The [control-event routing core](recovered/frontend/control_events.cpp)
  reconstructs the portable observer selection in `0065C900` and tagged-event
  filtering and local/queue routing in `0065CF50`. It preserves distinct
  receiver and stack-context values for callbacks, but uses portable containers,
  callback interfaces and shared payload ownership instead of original ABI,
  intrusive references and queue. These functions are also partial.
- Original `0065D150` initializes a control and sends its definition's `+10`
  list through `0065CF00`; tagged events reach `0065CD40`, which copies them
  into the control event vector. The host loads the original Exit/Yes/No
  activation records through that reconstructed tagged branch and filters
  their focus events through `0065CF50`. No focus record matches. The host still
  uses its own mouse handling. The confirmation hover adapter switches only
  the previously and newly focused controls rather than emitting extra leaves.
  The user reported it works as expected; the runtime log
  independently records Exit, focus transitions, Yes and normal teardown.
- The [menu event queue](recovered/frontend/menu_event_queue.cpp) and button
  activation callback now reconstruct the bounded Exit/confirmation sequence:
  activation emits `04001000`, matching records are queued, and the FIFO pump
  handles push, deferred quit, reply release/discard, pop and quit. On Yes the
  released quit is appended after the already queued pop. Escape queues the
  screen-four discard/pop pair. The host still owns mouse hit/press detection,
  actual model visibility, final quit and the screen object; unsupported event
  cases fail explicitly. `--menu-ui` retains direct Exit. The user reported the
  queue wiring works; that run's log independently showed Exit, No, Exit, Yes,
  pop before quit, 184 frames and clean teardown, but not Escape. `006552F0`
  and `00626B20` are absent from the frozen function inventory, so no ledger
  credit is claimed for those addresses despite their binary evidence.
- [PointerClick](recovered/frontend/pointer_click.h) reconstructs the target
  capture and equality/clear portion of `0066A220`; `--menu-confirm` now uses
  it for Exit and Yes/No. The host still does rectangle hit testing and cancels
  on pointer departure or focus loss, unlike the unmodified original captured
  target logic. The original timing, double-click, frame picking, per-update
  sequencing and object ABI remain unreconstructed. The user confirmed this
  change works. That run's host log independently showed No and Yes, pop before
  quit, 339 frames and clean teardown; that run did not show Escape or establish the
  drag-off result.
- [Screen focus routing](recovered/frontend/screen_focus.h) reconstructs the
  `00656B90` leave path and `00656C30` focus routing through portable adapters,
  plus the ordinary branch of `00656B20`. `--menu-confirm` now uses it for
  Yes/No hover changes. The host assumes both controls eligible; input-update
  timing and special target 15 remain unsupported. The
  user confirmed this integration works. That run's host log independently
  recorded No and Yes, pop before quit, 517 frames and clean teardown; it did
  not show Escape.
- [Screen target lookup](recovered/frontend/screen_target.h) reconstructs the
  original `0065C830` identifier-field selection and first-match search. The
  confirmation definition creates screen `10000000` and, in order, text
  `10100000`, Yes `10200000`, No `10300000` and OK `10400000`. The host maps
  rectangle hits to the two button IDs; the user confirmed this integration
  works. That run's host log independently recorded No twice, Yes, 588 frames
  and clean teardown, but not Escape.
- [Pointer picking](recovered/frontend/pointer_pick.h) now reconstructs the
  original cursor-to-ray and picked-frame identifier subset. A separately
  gated diagnostic calls the original scene picker and logs its ID alongside
  the rectangle result. The user confirmed it works; an earlier probe log
  showed matching Yes/No samples and a No-engine/empty-rectangle disagreement.
  A later probe showed matching Yes/No samples and a Yes-engine/empty-rectangle
  disagreement, without recording its position. It closed after 351 frames.
- A new opt-in `--use-confirm-picking` mode routes confirmation hover and
  press/release target selection through the original engine picker. It tags
  both Yes/No visual frames and preserves host cancellation and default modes;
  other controls, original input timing and screen/control objects remain
  incomplete. The user confirmed it works. The latest log confirms the active
  mode, records No, Escape and Yes, and shows 741 frames and clean teardown.
  Its edge samples do not record positions or establish exhaustive coverage.
- The opt-in mode now forwards sampled cursor motion through the original
  `00669FF0` movement predicate and the ordinary `0066B990` screen-motion
  branch. Motion within an already-focused button produces its `04005000`
  event; the host logs that event once per button to avoid flooding. Default
  rectangle-driven modes are unchanged. The host samples absolute positions,
  not original relative input, and does not reconstruct drag gating or full
  timing. The latest log records `04005000` for both Yes and No; the probe
  remains observational even when combined with the interactive mode.
- [Button click dispatch](recovered/frontend/button_input.h) now reconstructs
  `0064DBC0`'s `04012000` event followed by activation via `+38`, for the
  bounded Exit and confirmation buttons. The target parameter is omitted
  because the button activation callback ignores it. No registered event
  matches `04012000`, so queue behavior is unchanged. The user ran the desktop
  check; its log records `04012000` immediately followed by `04001000` for
  Exit, No and Yes and clean teardown. It does not record Escape.
- [Screen click routing](recovered/frontend/screen_input.h) now reconstructs
  `00656DD0`'s focused-index validation, focused-control callback and
  invalid-focus return 7. Confirmation clicks use it when the sampled focus
  matches the hit. If not, the host logs the mismatch and retains its previous
  hit-based selection, since the original input update order is not yet
  reconstructed and using stale focus could activate the wrong button. The
  user confirmed this works. The latest log shows focused-control routing for
  both No and Yes, and no logged fallback.
  `0064DBC0`, `00656B20` and `0065C830` are absent from the frozen function
  inventory, so no ledger credit is claimed for them. The strict i386 host
  build, existing native checks, 17 launcher checks, 151 binary anchors and
  function accounting pass. No new unit tests were added and Proton was not
  launched by the agent.
- Mouse-state research: selector zero means **any** button, not "button zero."
  `0066A050` implements this and nonzero bitmask selection; `0066A080`,
  `0066A0A0`, `0066A0C0` and `0066A0F0` implement previous/current tests and
  press/release transitions. `0066B990` uses selector 3 for screen button
  transitions. Engine `1006FB30` packs five button bytes into bits 1..10 hex.
  Constructor `0066A4B0` initializes captured/previous target and timer to zero.
  On release `0066A220` emits a double-click flag only
  for matching captured/current and previous/current targets with elapsed
  timer strictly below 400; every release resets the timer and capture.
  `0066B990` dispatches screen `+60` before ordinary click `+5C`. The focused
  button's double-click callback emits `04013000`; its click callback emits
  `04012000` and then calls activation. The new
  [button transition](recovered/frontend/pointer_buttons.h) and
  [click timing](recovered/frontend/pointer_click_timing.h) subsets are wired
  only into an observational `--probe-confirm-picking` shadow. It logs Win32
  button snapshots, picked identifiers, captured targets and click/double-click
  flags; message time in milliseconds is an approximation, not a verified
  original tick source. It resets on focus loss and between confirmation
  sessions. Original polling, timer units, update ordering and screen state
  remain incomplete; no double-click events are dispatched. Do not remove host
  cancellation on pointer departure without safe Win32 capture/outside release.
  The user reported it works; the latest log records matching left-button
  press/release captures and `click=1 double=0` for No and Yes, then Escape and
  clean teardown. It contains no right-button or `double=1` samples, so
  multi-button and timed double-click probe behavior is not desktop-verified.
- Next: expand original control and screen object construction, input and
  event routing beyond the bounded confirmation path. No new unit tests.
- Validation: 29 existing resource/launcher checks passed. Real-archive audit
  upgraded 16 assets to 21, preserved all prior records, checked every output
  hash, retained identical files/manifest through default restaging, and
  rejected a modified confirmation model before writes. Temporary assets were
  removed; desktop runtime and game stayed untouched. UI map validates 59
  anchors; front-end evidence and frozen function accounting also pass.

## Project scope and working agreement

- Reconstruct the base Steam Hidden & Dangerous 2 client and original LS3DF.dll.
  Sabre Squadron is excluded from current work and function counts.
- Prioritize the original menu over deeper ALPS1 mission research.
- Use our independently written i386 Windows host with the original engine and
  locally installed assets through Proton, not a lookalike replacement UI.
- Preserve original game files and the original Ghidra project. Generated assets,
  binaries, logs, toolchains and pseudocode remain ignored and local.
- Sensible local commits are authorized. Never push without explicit approval.
- Function baseline: 18,783 internal non-thunk functions; 29 partial, 0 complete.
  Integration glue and research do not receive reconstruction credit.

## Earlier text and hover desktop evidence

After the original text/Exit milestone, the user reported text crossing borders.
After restoring the original horizontal inset, the user confirmed:
"Good work; works as expected. Continue".

The earlier hover run followed the user's "Yup, works well" confirmation.
Its log contained nine initialized visual pairs, all 13 text bindings,
focused/normal transitions, `Exit control activated`, 575 frames and clean
teardown. That log was overwritten by later runs; the observation is preserved
in the UI map. Exact pixel-perfect visual fidelity is not established.

The first text attempt shut down cleanly at `Missing original menu frame:
text_screen`. Commit `54a7c67` corrected the frame-name pointer from auxiliary
`+0xF0` to name `+0xE8`. Native regression distinguishes both fields. Do not
reintroduce the wrong offset or normalize literal model-name spaces.
Earlier milestones: `6e559ce` static text/Exit; `5864005` original cursor;
`394a9e2` menu-model preview; `de944a2` camera/update diagnostics.

## Restart commands

Run from the workspace root:

```sh
make -C recovered engine-window \
  WINDOWS_CXX="$PWD/.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-clang++"
python3 -B tools/run_engine_window.py --menu-ui --frames 0
```

The launch must run from the user's desktop terminal: the agent environment has
no usable Proton window driver. Do not repeatedly retry that known limitation.
`--menu-model` preserves the artwork/cursor-only mode; omit both menu flags for
the blue graphics bootstrap. `--frames 0` waits for Exit, Escape or window close.
The launcher uses an isolated ignored prefix/runtime, not the user's game prefix.
Read the direct runtime log before interpreting unrelated Wine diagnostics;
an empty log can mean the desktop process has not finished yet.

Relevant checks (choose the smallest set covering the next change):

```sh
make -C recovered test
TMPDIR="$PWD/analysis/work" python3 -B -m unittest discover -s tools -p 'test_*.py'
python3 -B tools/validate_engine_host.py --map analysis/menu-ui-map.json
python3 -B tools/validate_frontend.py
python3 -B tools/function_progress.py --check
git diff --check
```

Before the successful desktop retry: 158 Python tests passed, native tests passed,
UI evidence validated 21 instruction anchors / 3 vtable slots, strict i386 build
passed. Graphics, scene and cursor also have separate host-validator maps.

## Code and evidence for the next investigation

- [Text geometry](recovered/host/menu_text_geometry.h): literal bindings,
  styles, model-bounds projection, glyph sizing/advance/alignment and Exit state.
- [Text renderer](recovered/host/menu_text.h): fonts, 13 labels and explicit
  D3D8 state bridge.
- [Scene integration](recovered/host/menu_scene.h): exact-name/subtree lookup,
  visual world bounds and hierarchy diagnostics.
- [Name reader](recovered/host/frame_name.h): the corrected `+E8` field.
- [Geometry tests](recovered/tests/menu_text_geometry_tests.cpp): alignment,
  rounding thresholds, bounds, name regression and bounded input.
- [Font parser](recovered/host/bitmap_font.cpp) and
  [stager](tools/stage_menu_text.py): original TBLs and selected English labels.
- [Original font research](analysis/menu-text-map.json),
  [UI ABI map](analysis/menu-ui-map.json) and
  [front-end map](analysis/frontend-map.json): original addresses and boundaries.

The verified missing horizontal inset is restored and desktop-confirmed.
Continue comparing actual original layout/style behavior against our geometry;
do not paper over future mismatches with guessed offsets, clipping or smaller
fonts. Current mode is fixed 800x600,
ANSI Set2. Button containers use their own `normal00` through `normal08` visual
children for bounds. Literal names include `bresume game`, `bchange profile`
and `text_last save00`. Hierarchy/bounds are now logged.

Important ABI/ownership pitfalls:

- Engine methods use stdcall with self on the stack, not C++ thiscall.
- Keep explicit camera/model updates: omitting them produced a green blank menu.
- Preserve graphics/driver/resource teardown order.
- The borrowed D3D8 device bridge is new integration glue, not an original font API.
  Do not Release the borrowed device or replace the explicit state snapshot with
  an all-state block: restoring FVF behind IGraph's cache desynchronizes drawing.
- Original text arithmetic includes promoted rounding-sensitive operations.
  Preserve threshold tests instead of simplifying float expressions.
- Use section-mapped PE reads for addresses; VA minus image base is not a general
  file offset. Verify Ghidra pseudocode against assembly/calling conventions.
- Research Ghidra project: `/home/control/Documents/Projects/HD2ResearchProject/HD2Research.gpr`.
  Original baseline: `/home/control/Documents/Projects/GhidraHD2Project/HD2Decomp.gpr`.
  Use read-only/no-analysis exports into new directories and verify each result;
  headless exit code zero does not prove every export succeeded.

## Deliberately deferred

In the main menu only Exit is enabled. Original confirmation,
tagged-event dispatch and focus behavior are only partially reconstructed in
the experimental `--menu-confirm` path; the default `--menu-ui` Exit still
closes directly. Original focus sound, other interactive screens and real
dynamic profile/date/save/version values are absent. Logos are original
`Gamemovie/Logo1.bik`, `Logo2.bik`, `Logo3.bik`; playback, audio, gameplay,
Sabre Squadron and deeper ALPS1 work remain deferred.
