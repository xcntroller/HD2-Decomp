# Startup configuration and data owners

This source-only follow-up to [settings/profile selection](startup-settings.md)
recovers the actual application+08 configuration owner, its game.cfg file
operations and the select-and-remember callback. It also adds an owning
composition of the four recovered startup data components, and the sound
manager's shared movie-change/fade helpers. Live startup remains unchanged.
Full application/state/protection, sound/catalog/control lifetime and controller
integration remain open; none is replaced with a successful placeholder.

New source:

- [StartupConfiguration](../recovered/frontend/startup_configuration.h): three
  persisted strings and original loading/setter/save behavior.
- [Shared legacy text](../recovered/frontend/legacy_text.h): real raw read,
  lookup/update and write operations, also used by profile.pr.
- [ApplicationStartupData](../recovered/frontend/application_startup_data.h):
  owns the recovered GameData/config/profile/HumanStore subset in original order.
- [Sound movie helpers](../recovered/frontend/sound_movie.h): actual IShow change
  operation and primary fade scheduling, through expanded
  [checked DLL forwarding](../recovered/host/settings_services.h).

## Configuration and ordering

Application construction006B4A60 creates GameData(+04), allocates/zeros the
12-byte configuration(+08) and immediately calls006A60A0. Only afterward does
it construct profile/options(+20) and HumanStore(+10). Configuration loading
precedes normal resource/archive/text setup. It must not be deferred until
profile selection. Other application owners are not reconstructed here.

| Config original offset | Value | Exact read key | Exact write key |
| --- | --- | --- | --- |
| +00 | CD key | NETCDKEY | NETCDKEY |
| +04 | Parent lock | PARENTLOC | PARENTLOCK |
| +08 | Startup profile name | LASTPROFIL | LASTPROFIL |

006A60A0 reads **executable-directory\game.cfg**, clears the first byte of its
100-byte scratch before each of the three lookups, and assigns profile, lock,
then CD key. Missing keys therefore clear their own field independently. Failed
file load is ignored; the method still assigns empty strings and returns true.
Native code rejects a lookup value of100 or more bytes to prevent the original
unbounded scratch copy. Earlier assignments persist if a later guard throws.

006A5F80 reads and writes **relative game.cfg**. It preserves loaded unrelated
lines and later duplicates, updates LASTPROFIL/PARENTLOCK/NETCDKEY in that order,
performs the actual DTA write and ignores its boolean result, returning true.
That return is recovered behavior around real I/O. The underlying writer still
reports errors; no successful persistence substitute is installed.

006A6320/006A64E0/006A6570 mutate the respective field before saving, with no
rollback on write failure. 006A63A0 compares the supplied parent-lock string
case-insensitively through the recovered default C-locale comparison, returns
false without saving on mismatch, and clears before saving on match. Empty
matches empty. 006A64B0 tests stored lock length. Native strings replace COW
references/allocators; raw owner layout and allocation-failure behavior differ.
The CD key is never printed or extracted during audits; fixtures are synthetic.

006B6670 selects through the actual profile owner(+20), then persists the
**requested** spelling through configuration(+08) only after selection success.
Normal startup006B6620 does not save game.cfg. A failed directory change can
leave the selected name empty while the separate callback remembers the request,
because actual006AF1D0 returns true after running both loads. Native code retains
the requested text across selection to guard an aliased caller's invalidated
pointer; the original wrapper keeps the raw input pointer.

`StartupProfileName` is now an alias of the actual configuration owner, so the
existing selection API and006A6560 accessor use its loaded +08 value.

## Shared text and owning data composition

The existing006E9230 reader and006E9840 lookup moved into shared source without
duplicate function credit. The reader appends nonempty raw CR/LF-separated
lines, retains embedded NUL bytes in storage, and closes only signed-nonnegative
handles. Key lookup sees each line only through its first NUL, finds the first
case-sensitive substring match, then returns the value after the line's FIRST
equals sign. No whitespace, comments or quoted-value policy is added. Native
guards an invalid original value pointer when a matched line has no equals sign.

006E9690 replaces the entire first substring-matching line with `key=value`, or
appends when absent. Later duplicates survive. 006E94B0 opens the actual DTA
writer with flags10000, writes each stored line length exactly (including NUL
bytes), then writes CRLF separately. Either short or oversized return is failure;
every successfully opened handle closes, including signed-negative ones. Empty
line lists still create/close the writer and succeed. Failed writes preserve the
already-written prefix. Native cleanup also closes after C++ exceptions.

The new data composition initializes profile settings directly, with no binding
default installation, preserves initial word34=0 and sets eight save words to
FFFFFFFF. This differs deliberately from calling profile.reset(), which sets
difficulty2 and performs other reset mutations. Settings+14 follows the prior
explicit native zero policy. Config name and selected profile name stay distinct.

Its prepare/reset methods call actual recovered GameData/HumanStore/options/
profile bodies. Config loading happens during construction, preparation still
attempts both data loaders, and reset/select happens later. Owner destruction
explicitly schedules GameData/config/profile/HumanStore, matching the relevant
006B6010 order. Native exception unwinding and subordinate native data layouts
retain their documented differences. This class covers four application owners,
not the whole application constructor/cleanup; neither earns new function credit.

## Sound-manager movie helper

00437210 handles the manager's retained +48/+68 IShow slots, separately from the
opening state's GameMovie object. With gate00982100 bit2 clear it returns true
without accessing the slot. With gate enabled and an absent slot, a nonnull
filename requests the real graphics+38 factory; a null result returns false.
An **empty but nonnull** filename still creates the object. No filename and no
existing object returns false.

An opened object is stopped(+18) then closed(+10), resolving the slot again
after each callback. A nonempty filename opens `sounds\<filename>` through+0C,
with flag2 for any nonzero LOW loop byte, otherwise flag0. Only an open result
of exactly zero invokes volume+3C then play+14. Negative/positive nonzero open
results both skip playback, while the helper still returns true. Neither null
nor empty filename requests release: the object stays retained in the slot.
Full manager cleanup must release it later.

00436E90 schedules the primary fade without a gate: target+50 is assigned, signed
duration clamps to at least one, and rate+58 stores
float32(abs((current54-target)/duration)) using extended intermediates. The full
sound frame-update body and catalog loader are still dependencies; no empty
update/catalog service is supplied. The helper alone does not establish full
manager construction, registration, playlist or teardown behavior.

## Evidence and validation

Recovery used existing exports and direct section-mapped objdump assembly;
no new Ghidra project or original executable code was run. The
[owner map](startup-owners-map.json) validates both original hashes,34 anchors,
115 direct calls and seven actual engine slots. It includes constructor/caller
context without claiming those complete bodies are source-recovered.

Temporary ASan/UBSan audit passes for raw line/embedded-NUL behavior, all writer
short/oversized stages, signed-handle differences, config missing/partial files,
99/100-byte boundaries, per-field scratch reset, exact read/write keys and order,
lock mismatch/match and failed saves. It composes actual GameData/HumanStore/
binding/options/profile bodies through the new owner, checking early config
load, initial profile state, normal startup without persistence, remembered
failed-chdir requests and legacy profile fallback after the shared-code move.
The sound helper's144-case gate/object/name/open-result/factory matrix passes,
as do slot replacement during callbacks and signed-duration/negative-zero fades.

Strict i386 compile/link composes real DTA, executable-directory/Win32, portrait,
joystick/input-manager, settings/sound/movie and profile adapters; the binary is
never run. Existing native checks and all158 Python tests pass, adjacent settings
and data maps validate, and the unchanged host target is up to date. Seven
negative owner-map mutations are rejected. Temporary fixtures are removed after
validation; no permanent tests or original game/config/DLL writes/execution.

Twelve new census-backed partial client bodies raise the ledger to **252 partial**
(247 client/five engine), **zero complete**, frozen denominator18,783. No new
constructor, existing shared-body, thunk or delegated-engine credit.

Next: full client sound manager registration/state/cache loading and normal movie
coordination, shared gate/control-interface initialization, remaining application
owners/state/protection and cache consumers. Those remain prerequisites for live
controller wiring. No new desktop test is needed for this source-only slice.
