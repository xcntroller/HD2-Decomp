# Startup options and profile selection

Follow-up: [startup configuration/data owners and sound movie helpers](startup-owners.md)
now implement the separate startup-name owner/loading and persistence. The
remaining-scope notes below describe the checkpoint before that follow-up.

The settings/profile-selection batch is source-recovered and audited, following
`f06b41f`. It adds no live behavior. The original launch milestone remains open:
full application/state/protection ownership and sound/control initialization
must precede controller integration. No successful placeholder service is used.

[Settings/options source](../recovered/frontend/profile_settings.h),
[sound/control source](../recovered/frontend/settings_sound.h),
[profile selection](../recovered/frontend/profile_selection.h) and the
[concrete DLL/COM boundary](../recovered/host/settings_services.h) compose with
the existing chunk-file, binding defaults/installer, input manager, directory
map and profile loader. The [preparation record](startup-settings-preparation.md)
retains detailed offsets and research provenance. The
[fingerprinted map](startup-settings-map.json) guards both original binaries,
41 instruction/data anchors, 127 direct calls, two client sound slots and five
engine slots. Run `python3 -B tools/validate_engine_host.py --map analysis/startup-settings-map.json`.

## Loading and defaults

`ProfileSettings` stores all 60 bytes explicitly, preserving float bit patterns,
unaligned seven-byte presets and partially overwritten values. `ProfileOptions`
owns settings and the existing `BindingConfiguration`; no independent binding
installer/default implementation is introduced.

006B3420's default helper writes all established fields and leaves word+14
alone. 006B31C0 originally copies that unwritten word from a stack temporary.
Native reset defines it as **zero**, using a zero-initialized temporary before
the recovered stores. This is an explicit native policy, not an original
default. None of the six application helpers reads it.

006B2D80 resolves the selected display name through the actual directory map
and opens `PlayersProfiles\<resolved>\Options.bin`. It reads sequential
top-level chunks 25A0/version141, 25A6/bindings and 25A5/settings. These are
not children of 25A0. Failed open/root/version resets both settings and bindings
and returns false. A wrong binding/settings outer tag resets both too, but the
outer loader still returns true after a valid version. A second fallback can
discard freshly loaded bindings. Inner reader results are deliberately ignored.

006A8900 clears only configuration records, preserving mode and installed
manager bindings. Version22B8/42 precedes records22B9. Every raw 28-byte record
uses one reused zero-initialized scratch, preserving partial overwrite effects,
padding and order, with dirty byte forced to one. The actual existing reverse
dirty installer runs after the records chunk is left. Version failure has
already left its child; wrong-first/wrong-record tags leave their active chunk
on their own fallback paths. The options caller leaves the containing chunk.

006B3750 reads child1201 as seven four-byte transfers, four seven-byte transfers
and four single-byte transfers. Each read result is ignored, including partial
backend writes. A bounded read can refuse a field without consuming bytes;
later smaller fields can still consume those bytes. A wrong child still gets
left. There is no clamp or postprocessing.

Native malformed-header/version handling takes the corresponding fallback
instead of using an original uninitialized or stale local. Binding records
with fewer than 28 remaining bytes, negative remaining counts or a backend
read that makes no progress throw, retaining prior mutations and closing the
file through RAII. The original can append indefinitely in those cases.

## Applying the values

006B2D40 saves the loader result, applies all six helpers in the following
order even on load failure, and returns the saved result.

| Original helper | Recovered behavior |
| --- | --- |
| 006B3340 | Query I3D driver; if present query again, then state2B = selected quality block byte0 through +18. |
| 006B3210 | Truncate `(gamma-50)*2+255`, keep low16; emit three identical 256-word ramps before each saturated increment, clamp sums to 255..65535; graphics+12C with flag0. |
| 006B32A0 | Multiply float08 by float32(.01), round to float32, call the native client sound driver's volume setter. |
| 006B32D0 | Same float32 scaling for float0C, through the native client movie volume setter. |
| 006B3320 | Multiply float10 by double(.01), truncate to integer, submit through the separate optional control owner. |
| 006B32F0 | Truncate float00*(-9)+1000, use the existing manager axis-limit helper for x/y and 120 for z, then copy byte39 to shared input flag008AEAFC. |

Arithmetic uses extended intermediates, explicit float32 argument stores and
the low32/low16 bits of signed64 truncation, matching the original `__ftol`
scratch/return convention. Native code throws for nonfinite or out-of-int64
conversions rather than invoking undefined C++ conversions. Floating-point
environment changes and original exception/control-word behavior are not a
recovered application policy.

The native sound view borrows actual manager fields and the mutable gate
00982100. Both setters leave stored state and backend untouched while bit2 is
clear. 00433550 stores +3C and calls ISnd+14. 00433510 stores +44 and calls
movie+3C only if movie+48 is nonnull and its +08 query succeeds, passing
float32(scale54*volume44). This is the active movie channel. It is not a second
global sound-driver volume. Movie is resolved again after its opened callback.

0072EA20's wrapper+0C branch submits exactly
`[20,28,1,value,FFFFFFFE,0,0,0]`. 00742210's null interface returns800401F0;
otherwise interface+20 receives self and the block on the stack. Negative
HRESULTs survive; nonnegative results normalize to zero. The caller ignores
the result. The native wrapper and object are genuinely borrowed and optional;
no interface is manufactured or permanently empty successful owner installed.
Its creation/lifetime and semantic name remain open.

The host boundary checks getter exports and the exact engine targets:
I3D+18 -> 10015770, IGraph+12C -> 100700A0, ISnd+14 -> 1007A9B0,
IShow+08 -> 10071B70 and IShow+3C -> 100724D0. ISnd volume returns an integer;
the client setter's float return is implemented natively, not confused with
that DLL return ABI. External COM+20 has no fixed engine/client RVA.

## Selection and ownership

`ProfileState` composes the SAME original application+20 profile owner's
directories/name, options and persisted payload. `StartupProfileName` describes
the distinct application+08 owner's string+08, whose accessor is006A6560.
It does not pretend to recover that owner's constructor or persistence loader.

006AF1D0 copies an input name before clearing the current name, so aliased input
survives. ANSI uses the physical candidate directly; wide mode uses exact mapped
physical name or empty. A nonempty candidate is tried through the real
executable-relative directory adapter. Only success stores the display name.
Executable cwd restoration runs regardless of chdir success. Then the explicit
shared `ProfileSelectionWords` owner clears all 256 DWORDs corresponding to
00868A2C..00868E28 inclusive, in reverse order. Options load/apply and actual
binary/legacy profile load both run regardless of their boolean results;
selection returns true. Native safety exceptions remain failures, not success.

006B6620 optionally scans profiles first, then obtains the initial name from
the distinct startup owner and selects through the profile owner. Its scan flag
is the original application+30 byte. The shared selection words remain owned by
the caller for later consumers; their wider purpose is not guessed.

## Validation and remaining scope

Temporary ASan/UBSan audits cover exact defaults/unknown-word policy, every
settings payload length0..60, partial backend transfers, binding modes/dirty/
padding/reverse install order, all outer fallbacks, branch-specific nesting,
27 trailing-record lengths, gamma boundaries, missing driver, sound gate,
absent/unopened/active movie, all HRESULT paths, failed/missing options/profile
loads, aliased selection input, full256-word clear, directory scan/mapping,
missing wide mapping and distinct startup owners. Actual recovered readers,
defaults, installer, manager tuning and profile finalizer are used throughout.

Independent inline x87 instruction arithmetic agrees for **175,877** random
finite settings across gamma/input/control integer conversions and float32
sound scaling. This runs new native audit code, never original executable code.
Strict i386 compile/link composes real DTA, Win32 profile directories, graphics,
joystick/input manager, settings, sound/control and profile paths; it is not run.
Existing native checks, all158 Python tests and the unchanged host target pass.
Seven negative map mutations are rejected. Temporary audits are removed after
validation; no permanent tests, original-game/DLL execution or desktop changes.

Nineteen newly represented original bodies are in the frozen inventory: **240
partial** total (235 client/five engine), **zero complete**, denominator18,783.
00433510/00433550 are absent from that inventory and receive no credit. Neither
the sound constructor nor delegated engine/COM methods receive extra credit.

Next coherent launch batch: full application+08 startup-name ownership and its
loading, client sound-manager/movie ownership and gate lifecycle, and actual
control-interface creation/lifetime where reachable from normal startup. Then
finish application/state/protection and loaded-cache dependencies before wiring
the shared controller. No new desktop test is needed for this source-only batch.
