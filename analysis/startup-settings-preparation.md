# Next implementation: original settings and profile selection

Implemented and audited in the follow-up [settings/profile-selection slice](startup-settings.md).
The preparation below is retained as the original research checkpoint.

Preparation checkpoint, 2026-10-30, following `f06b41f`. This is verified static
research and an implementation order, **not implemented settings behavior**.
The live preview is unchanged. The original launch milestone remains open.

## Evidence and retained exports

Client SHA256:
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
Engine SHA256:
`12c61eed2aec0c45700ad0a5ddfd7cbb3cbf0ea15ec67de155790bb24e7756ee`.
Use section-mapped `PEImage(Path(...).read_bytes())`, not VA-minus-base offsets.

Existing ignored `analysis/work/` directories contain decompiled C and JSON
instruction/call references; do not repeat the earlier export work:

| Directory | Functions needed next |
| --- | --- |
| `profile-preparation` | 006B2D80 options loader; 006B3210/32A0/32D0/3320/32F0/3340 application |
| `profile-movie-state` | 006B3750 settings reader, 006B31C0 defaults, 006A8900 binding reader, 006B33E0 quality getter |
| `profile-selection-dependencies` | 006B3420 default bytes; existing binding reset and profile dependencies |
| `application-profile-startup` | 006AF1D0 outer selection |
| `game-data-variants` | 006B6620 startup profile selection |
| `startup-settings-preparation` | Newly exported 006B2D40, 006A6560, 0072EA20, 00742210 |
| `startup-settings-sound-verified` | Newly exported 00433510/3550, 004323E0 and constructor004336D0 |

Two sound methods were absent from Ghidra's function definitions. New
`tools/DefineSettingsFunctions.java` verifies the client hash/base and actual
vtable pointers 0080F834 -> 00433510, 0080F83C -> 00433550 before transiently
defining them. It ran successfully before ExportFunctions in a **read-only**
project; the log confirms discarded changes. Earlier `startup-settings-sound`
is an empty failed export directory, not evidence. These two methods are also
absent from the frozen census: source recovery must not manufacture credit.

Reproduce with the existing analyzeHeadless command in HANDOFF, adding
`-postScript DefineSettingsFunctions.java` before ExportFunctions and using a
NEW output directory. Check individual export messages, not just exit status.
Native objdump confirmed the x87 arithmetic and ECX owners described below.

## Implement one coherent settings/profile-selection batch

1. Recover the 60-byte settings value, defaults, settings and binding readers,
   Options.bin loader and exact load-then-apply orchestration in frontend source.
   Reuse the actual ChunkFile, BindingConfiguration/defaults and input manager.
2. Recover the client sound-manager setters and the separate 0072EA20/00742210
   control path. Add concrete checked DLL/COM boundaries only where evidence
   establishes ownership and method signatures. Preserve real absent-device
   branches; do not use permanently empty owners as successful substitutes.
3. Compose outer profile selection006AF1D0 with actual settings and profile
   loading, then startup selection006B6620 using distinct application owners.
4. Audit this whole source batch before documentation/count updates. Then
   continue remaining application/state/protection/cache dependencies as usage
   permits. Wire the live controller only once those bodies are real.

## File and reader contracts

006B2D80 uses current display name at profile-owner+00, resolves its physical
directory through the existing exact wide map, and opens
`PlayersProfiles\<resolved>\Options.bin` in mode0 on a fresh ChunkFile.
First chunk25A0 must contain DWORD141 (hex); leave it before the next chunk.
Open/header/tag/version failure calls006B31C0, closes, returns false.
Then expect chunk25A6 and call006A8900; otherwise reset defaults. Leave that
active chunk. Next expect25A5 and call006B3750 on owner+38; otherwise reset
defaults again. Leave, close and return true, ignoring inner reader results.
These are sequential chunks, not children inside25A0. Both fallback points
reset settings AND bindings; a second fallback can discard freshly read bindings.
Original short-header paths can reuse stale tags/uninitialized words; make any
native guard explicit rather than attributing deterministic results to them.

006A8900 first clears only BindingConfiguration.settings, preserving its mode
and installed manager bindings. Require child22B8/DWORD42, then child22B9.
Read raw 1C-byte records into one zero-initialized, reused scratch record,
force dirty byte+18 to1, append in order, then apply_dirty_bindings006A86C0.
Wrong tags/version restore default layout0 using existing006A8BE0. Check the
export for branch-specific leaves: version mismatch already left22B8 and
returns immediately; other fallback paths have a different active chunk.
An original trailing 1..27 bytes can loop forever because bounded read refuses
without advancing. Add an explicit native termination guard. A short backend
read can partly overwrite the reused scratch; do not zero it each iteration.

006B3750 enters child1201 and reads exactly these ranges into settings:
six DWORDs at00/04/08/0C/10/14, DWORD18, four seven-byte blocks at1C/23/2A/31,
then individual bytes38/39/3A/3B. Ignore individual read results, preserving
prior/partly overwritten values. It leaves the child even for a wrong tag;
outer loader leaves25A5. No extra clamp or post-processing occurs here.

Default006B3420 writes these relative settings offsets (all numbers hex):

| Offset | Value / store size |
| --- | --- |
| 00 | 42853333 / DWORD (approximately 66.6f) |
| 04,08,0C,10 | 42480000 / DWORD (50.0f) |
| 14 | **Unwritten** |
| 18 | 2 / DWORD |
| 1C,20,22 | 0 / DWORD, WORD, BYTE |
| 23,27,29 | 01000100 / DWORD, 0100 / WORD, 0 / BYTE |
| 2A,2E,30 | 01010101 / DWORD, 0101 / WORD, 1 / BYTE |
| 31,35,37 | 01000100 / DWORD, 0100 / WORD, 1 / BYTE |
| 38,39,3A,3B | 0,0,1,1 / BYTE |

006B31C0 builds a stack temporary, copies all fifteen DWORDs into owner+38,
then resets bindings at owner+EC with layout0. The copied +14 DWORD really
has undefined stack provenance; no original default is established. Choose and
document a defined native representation before implementation, never read
uninitialized C++ storage. The application helpers below do not consume +14.

## Apply order and verified arithmetic

006B2D40 saves loader result, then calls006B3340,006B3210,006B32A0,006B32D0,
006B3320,006B32F0 in that order **even on loader failure**, finally returning
the saved result. All six receive the same profile/options owner in ECX.

| Helper | Actual calculation / effect |
| --- | --- |
| 006B3340 | If I3DGetDriver() nonnull, query it again; driver+18(self,2B,quality byte0). Getter006B33E0 uses owner+38, selects block1C/23/2A/31 for quality DWORD18 equal0/1/2/other. |
| 006B3210 | Truncate `(double(settings.float04)-50.0)*2+255.0`; use low16 bits as ramp increment. Three contiguous arrays of256 uint16; each emits current value before adding, clamps sum below255 to255 and sum >=65535 to65535. GetIGraph()+12C(self,ramps,0), actual engine target100700A0. Default50 gives increment255. |
| 006B32A0 | settings.float08 times float32 constant0.01, round/store float32; client sound-manager+84 thiscall(float), return x87 float discarded. |
| 006B32D0 | settings.float0C times same float32 constant0.01; client sound-manager+7C thiscall(float), return discarded. |
| 006B3320 | Truncate settings.float10 times float64 constant0.01; call0072EA20 on0097D070. Retain this unusual integer conversion; do not replace it with a guessed volume curve. |
| 006B32F0 | Truncate settings.float00 times float32(-9.0) plus float32(1000.0); manager008AEA48 tuning006C6C90(value,value,78). Then copy settings.byte39 to008AEAFC. Reuse recovered tuning helper. |

Verified constants: 0080F574 float32(0.01), 0080F8F8 double50,
008159E8 double255, 008159F0 float(-9), 0080F708 float1000,
00811968 double0.01. Match x87 intermediate precision and __ftol truncation
where defined; specify native handling for NaN/out-of-range rather than C++ UB.
Graphics/driver calls pass self on stack; the two **client** sound calls use
ECX self. Do not reuse the DLL calling convention for those client setters.

004336C0 returns client singleton00899728, not ISndGetDriver. Constructor
004336D0 installs final vtable0080F7B8. Both setters gate on00982100 bit02:
if clear, neither stored value nor backend changes, and old value is returned.
00433550 (slot84) stores manager+3C and calls ISndGetDriver()+14(self,float).
00433510 (slot7C) stores manager+44, checks active movie+48 through004323E0
(nonnull and movie+08 true), then calls movie+3C with manager.float54 times
manager.float44. This is movie volume; do not apply it as a second global
sound-driver channel. Track manager construction, gate initialization and movie
ownership before attaching these setters to live startup.

0072EA20 checks its owner+0C wrapper. If present, submits eight DWORDs
`[20,28,1,value,FFFFFFFE,0,0,0]` through00742210. That wrapper checks its own+04
interface: null returns800401F0; otherwise interface+20(self,block), retaining
negative HRESULT and normalizing nonnegative to zero. Caller ignores result.
Interface creation/lifetime and semantic name remain unverified; investigate
existing0072EA20/00742210 callers instead of inventing a service implementation.

## Outer selection and startup ownership

006AF1D0 retains the input name before clearing profile-owner+00. ANSI candidate
is that name; wide candidate is exact mapped physical name or empty. For a
nonempty candidate, use existing006E8F10 to try PlayersProfiles/candidate;
only on success store the original display name. Restore executable cwd
unconditionally. Then zero **256 DWORDs at00868A2C..00868E28 inclusive**:
assembly starts EAX424, subtracts4, stores at[EAX+868A08], ends after EAX24.
It calls006B2D40 and006AFB00 on the SAME profile owner regardless of their
results, then returns true. Preserve global-array ownership for later consumers;
do not silently discard the clear because its wider purpose is still unknown.

006B6620: if application.byte30, scan profiles using application.pointer20;
query006A6560 on application.pointer08, which returns that owner's string+08;
select it (or empty text if null) through application.pointer20. The two owners
are distinct. Do not source the startup name from the just-cleared profile owner.

## Validation prepared for the implementation session

- Temporary native sanitizer audit: exact defaults including unknown+14 policy;
  valid/missing/wrong-version files; sequential tags; fallback resetting both
  bindings/settings; partial reads, odd trailing binding bytes and balanced leaves.
- Reuse actual defaults/installer/manager, not an independent imitation. Test
  binding mode preservation, forced dirty bytes and reverse application order.
- Record real helper calculations/order at default and boundary values, loader
  failure, missing driver, gate disabled, absent/active movie and HRESULT paths.
  Verify actual float32 versus float64 scaling and uint16 gamma increment.
- Compose original directory map/cwd, Options.bin, profile payload/finalizer and
  startup owners. Cover failed chdir, missing wide mapping, aliased input name,
  full256-word clear and both loads despite failures.
- Extend fingerprinted evidence with actual new anchors/calls/slots. Strict i386
  compile/link real adapters only; no agent-side Windows/game execution. Run
  existing relevant checks once; remove temporary audits afterward.
- Only credit bodies actually recovered and present in the frozen inventory.
  Current baseline remains221 partial (216 client/5 engine), zero complete.
  No new desktop test is required until live behavior is changed; that interactive
  slice still waits for user confirmation before commit.
