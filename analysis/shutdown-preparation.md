# Failure and shutdown delivery preparation

Research checkpoint: 2026-10-31, after accepted playback commit `cc8b528`.
The [implementation sweep](shutdown-delivery.md) now supersedes the current-gap
list below; bounded desktop acceptance is now recorded in the delivery contract.
Milestone0 item4 remains **open**. This document prepares one implementation
sweep within the accepted English800x600, standalone gate7, opening → Main/Exit
launch boundary. No runtime code changed and no new desktop test occurred.

## Finish line for item4

Deliver acquired-owner cleanup for startup failures, state construction/update/
render failures, normal status4 Exit, window close and bounded frame-limit exit.
Keep the real device/module/archive lifetimes and original fatal-versus-fallback
policy. Add final cold control shutdown, module shutdown dispatch and window
position saving. Verify the reached resources are released before their devices;
retain the first failure while attempting remaining independent cleanup.

This does not require completing setup.exe/registry negotiation, integrity VM,
live network/gameplay, all menu flows or the original object ABI. The original
extra setup/cache owners below must stay named as excluded when not acquired by
the rebuilt checkpoint. Do not represent their absence with successful services
or credit directory-list clearing as loaded-resource teardown. If a new producer
is introduced, its actual owner/cleanup must join this delivery boundary.

## Original paths and ordering

[The evidence map](shutdown-map.json) checks both original fingerprints,46 byte
anchors,18 module shutdown slots, six engine slots,20 direct calls and three PE
zero-fill ranges. Read-only Ghidra exports are under ignored
`analysis/work/shutdown-{owners,modules,reached,auxiliary}-20261031`; the global
reference listing is `analysis/work/shutdown-refs-20261031.txt`. Transient method
definitions were discarded from the project. Reuse established playback,
[control](control-startup.md), [data](startup-owners.md),
[directory](resource-directories.md) and [window](window-position.md) contracts.

| Path | Original evidence | Required policy |
| --- | --- | --- |
| Setup failure | 006DCAE0 call006DCB55 →006DBFC0; false destroys the controller through006C0790 and returnsFFFFFFFF | Fatal. Original skips006DC3A0 and event close here; native acquired-owner unwind is a documented safety difference. |
| Controller-start failure | 006DCC52 →006C0820; false returnsFFFFFFFF directly | Fatal; do not claim the original uses one universal cleanup path. |
| Graphics/3D initialization | 006DBFC0 →006DB0F0 and006DB4C0, each with a false abort | Fatal; close partial native acquisition once. |
| Sound initialization | 006DC1F6 →006DB3B0 is void; next call continues regardless of sound status | Preserve silent fallback and close partial sound initialization. |
| Missing/stopped movies | Recovered ordinary playback tail and accepted omitted-movie test | Continue sequence/menu; not a fatal startup failure. |
| Window save failure | 006DC3A0 signed negative dtaOpenWrite handle skips save; later return values ignored | Do not turn persistence failure into failed launch or skip remaining teardown. |

Normal 006DCAE0 exit restores the saved window procedure, then invokes006C08D0
only for active status1/6/5 with gatebit1. It destroys the controller006C0790,
closes debug writer008AC338 and transient owner0098C0FC, invokes006DC3A0, closes
manual-reset event008AED10 and returns1. The rebuilt executable deliberately
uses process success0/failure1; original return1 is not a reason to change that.
The debug writer is actually created after successful original controller start;
it is not a proven empty original branch. The rebuilt launch has no such writer,
transient service or manual-reset event and must not invent them just to close them.

006C08D0 bypasses its three service calls006DC940/0072A0B0/0072C5D0 when gate&7=7,
but still calls006C0920. That body invokes active state virtual+08, clears caches
**2→1→3→4→5**, deletes/nulls active unless reinitializing, then closes mouse for
gatebit2. The original assumes active is nonnull. Native shutdown tolerates an
already-released state. Current status4 after update already calls native
shutdown and0072E4E0 reset; outer repeated shutdown does not imply a second actual
mouse close because the host checks the real enabled state. Status4 at update
entry, WM_CLOSE and frame-limit exit instead rely on the outer shutdown.

006DC3A0 then performs:

1. **0072E460**:0072E4E0(1) reset; release/null control+0C and+10 through virtual00(1);
   00723220 releases+14 through virtual+10(1); if+5018, delete startup critical
   sections+418 then+430 and clear flag; set global gate7; release/null+4EE0
   through virtual+0C(1).0072E1A0 explicitly initializes all these optional owners
   to null. Their absence is justified for the standalone host, not every mode.
   Final reset remains required even when status4 reset already ran. Preserve
   the separate peer-owner lock until its owner is destroyed.
2. **006B6010**: GameData+04, configuration+08, profile+20, humans+10, then other
   application owners+18/+1C/+14/+24/+28/+0C. Native ApplicationStartupData owns
   and already closes the first four in that order. The other original owners
   are constructed by006B4A60; they are excluded, not proven original nulls.
3. **0048DA80**: ascending fresh reads of all19 registry slots0089BAD8..0089BB23;
   skip null, invoke module virtual+08. Never cache a snapshot or reverse order.
4. **00783220**: release eleven actual auxiliary pool owners (details below).
5. **006E85A0**: clear auxiliary vector008CA3AC/3B0 and release/null edit-font
   resource008CA3B8.
6. Save **window.cfg**, while graphics/window/rw_data are alive: open_write flags
   10000hex, signed-negative failure, otherwise GetWindowRect and two separate
   four-byte little-endian left/top writes, close including handle0. Reuse the
   recovered helper/adapter. Do not manufacture original stack bytes after failed
   GetWindowRect; choose and document a native valid-coordinate fallback or skip.
7. Clear loaded-resource caches **1→2→4→5**; category3 is absent from this tail.
8. Close **I3D+04 → ISnd+08 → IGraph+10**, each with a null getter check and a
   fresh getter for the actual call. Native owner identities remain guarded.
9. Close/null all16 nonnull archive slots008AE9F8..008AEA37, ascending.
10. CoUninitialize. The current outer COM/module guard must outlive device and
    archive cleanup. Engine/rw_data/joystick code must remain loaded through all
    owners and deleters.

## Module shutdown is bounded and recoverable

| Kinds | Actual slot+08 targets | Work |
| --- | --- | --- |
| 0,1,4,5,6,8,10,11,14,15,16,17 | 00405B00 | Verified RET, including Sound6; not a replacement for sound-runtime reset. |
| Physics2 / Versions7 / Temp Actors12 / Actors13 | 004106D0 /0043DC80 /00476F90 /00475740 | Verified RET. |
| Items3 | 007DCE00 |0078A980 list cleanup;500 object slots; shrinking request vector. |
| Vertex9 | 00474A20 | Gatebit2; destroy entries from the end through00473720. |
| 18 | Null in current normal registration set | Skip; reject an unsupported native replacement module. |

Items constructor007DCA30 zeros all500 slots and request storage; ordinary
startup007DCD20 sets current=-1.0078A980 checks gatebit2, releases the first linked
list's engine object references, frees both lists and zeros heads0098AD48/4C and
counter54. These are PE-zeroed initially and the standalone host has no producer.
Recover an explicit checked cold Items shutdown and reject populated native
owners before mutating them; do not claim dirty Items shutdown implemented.
Vertex shutdown should use existing actual VertexAnimations storage and cleanup,
through a real module shutdown dispatcher.00474A20 and the empty RET targets are
absent from the frozen census and receive no new function credit.

## Extra original owners are not empty-cache evidence

Successful006DBFC0 reaches006E8590 and00782FC0 after window restoration, before
returning success.006E8590 forwards &008CA3B8 to006E8530: actual editfont.bmp
texture load, negative result clears output, success clears the matrix then
optionally reads tables/fnt_matrix.bin.006E85A0 later releases that texture and
auxiliary records. The current menu uses its separately owned bitmap font and
has no producer for this edit/debug-font system.

00782FC0 allocates two24DC-byte owners, seven1114-byte owners, oneA90-byte owner
and one34-byte owner. Their constructors007835A0/007833C0/007838A0/00783A60 also
allocate backing blocks55F8/2220/189C/F0 respectively. Final00783220 invokes
00783690/00783440/00783910/00783AC0 then frees outer storage. Destructors free
backing blocks and release stored engine references. The wrapper **does not null
its globals**, so it is not evidence for safe repeated original shutdown.
These allocations are absent from the current checkpoint; do not allocate fake
empty pools, call them required menu services or mark them ported. Integrating
their eventual producers requires actual construction and cleanup together.

Loaded cache owners are distinct from the already integrated directory lists:

| Category | Owner | Controller clear | Final global clear |
| --- | --- | --- | --- |
| 1 | 008AEC90 | Second | First |
| 2 | 008AEC54 | First | Second |
| 3 | 008AEC18 | Third | Absent |
| 4 | 008AEBDC | Fourth | Third |
| 5 | 008AEBA0 | Fifth | Fourth |

Each clears loaded vector+18/+1C, lookup tree header+08 and releases/nulls current
reference+38. Directory registration is separate+24/+28/+2C state. Shared erase
bodies006C53D0/006C5250/006C50C0 retain engine objects at+E4; category3's006C5560
uses DWORD+04. Do not infer resource type from that offset alone. Current host
MenuScene directly owns real engine models/frames; text/cursor own their own
resources; HumanStore owns portrait references; movie/text/voice owners have
explicit playback cleanup. None populates these original client loaded-cache
vectors/trees. Prove their concrete owners release before drivers in the native
failure matrix; leave the complete original caches and their producers open.

## Current gaps and concrete implementation order

1. **Make acquisition and cleanup explicit.** Replace nested catch-only cleanup
   in application_launch with a completion record for initialized control locks,
   devices, setup consumers, controller/state and hook. A single coarse Phase is
   insufficient for an exception during setup. Track partial lock initialization
   before record allocation can throw. Normal and failure cleanup must attempt
   independent later releases even if a callback throws; preserve the first error.
   Avoid throwing destructors and repeated successful device closes. Existing
   MenuDriver and graphics constructors already close negative partial init;
   start_menu_sound already closes failed init and returns silent fallback.
2. **Close active/partial state before devices.** ApplicationController has no
   shutdown destructor; its unique_ptr alone does not call state.shutdown.
   OpeningBackend has a resource-release destructor, but ApplicationMenu has no
   lifecycle fallback. Add explicit exception-safe state/menu cleanup so stacked
   borrowers deactivate before products, then inputs/text/cursor/scene. Check
   partial factories and callbacks. Current implicit member destruction skips
   deactivation; it is not, by itself, proof of a dangling-reference dereference.
   Restore the saved window hook at the intended boundary before final teardown.
3. **Finish reached startup teardown.** Add final cold control shutdown and
   explicit lock deletion; keep repeated reset semantics and guard unsupported
   +4EE0/interfaces. Close real sound runtime/catalog while devices and dependent
   profile data still exist. Close the four data owners, then ascending actual
   module shutdown. Keep unacquired auxiliary/cache systems explicit. Keep input/
   joystick ownership valid until their last borrowers finish.
4. **Persist and close the outer owners.** Wire the existing window save helper
   with an explicit valid-coordinate failure policy. Close3D→sound→graphics,
   then archives, then COM/module owners. Ensure constructor failures and external
   close/frame limits use the acquired subset, not the full original normal tail.
   If device owners gain explicit close, clear ownership before a destructor can
   repeat it. Preserve startup_probe/engine_window behavior sharing those owners.
5. **Audit and request desktop acceptance.** Use temporary native failure
   injection across each acquisition/callback and real portable owner composition;
   verify reverse release within each owner, cross-owner order, no device calls
   after close, first-error retention, no live owner leaks/double close, repeated
   cleanup and pre-acquisition exits. Compile/link real Win32 adapters and all
   three targets strictly without executing them. Run existing native/Python,
   evidence/mutation and frozen-ledger checks. Remove temporary audits.

Desktop acceptance for the implementation should cover ordinary Exit and window
close during a movie and menu, frame-limited exit, saved window.cfg bytes and a
repeatable staged real startup-resource failure. A staged failure option must
hide only validated copies and restore them on exceptions/collisions, following
the existing omitted-movie policy. Choose a file the original loaders actually
treat as fatal; missing movies and missing options/profile fallbacks are not
fatal tests. Capture ordered teardown/result logs and restored copies. Request
only tests relevant to that implementation; agent must not execute Windows code.

A concrete fatal-loader fixture is a staged loose override at
GameData\Gamedata00.gdt containing the six-byte header `02 00 06 00 00 00`.
Confirmed006BB540 requires root1 (optional metadata3 first); an unknown first
root leaves the file open, rejects the next open and returns false with no loaded
roots.006B65D0 still attempts HumanStore, writes game+E8=1 and combines both
loader results, so controller preparation fails. This avoids a short-header
native exception. The original DTA flags0/default override0 prefer loose files
([lookup contract](startup-archives.md#actual-lookup-precedence-and-lifetime));
prove the fixture was selected in the actual run rather than assuming an archive
was hidden. Never modify installed assets or archive bytes. Back up any existing
staged override and restore it even on failure or replacement collision.
Item4 is checked only after implementation and the user's desktop confirmation.
Item5 remains the independent final launch matrix/accepted commit checkpoint.

## Research validation

A temporary ASan/UBSan audit of the existing ApplicationController confirms:
implicit destruction releases the state without its shutdown callback; explicit
repeated shutdown calls the state once and closes an enabled mouse once; a
throwing state callback leaves the active owner installed and skips mouse close.
The actual ColdControlFrame template safely repeats its cold reset with the
expected firewall/graphics/peer/record/queue order. This establishes current
behavior and needed work, not delivery of the missing cleanup coordinator.
The audit is removed. No permanent tests or runtime source changes were added.

Read-only exports succeeded for33 requested bodies (9+2+12+10); the initial
probe found six indirect targets absent from Ghidra definitions. Those six were
subsequently byte/vtable-guarded and defined only transiently for successful
export. No original game/DLL/Proton code ran. Evidence and four negative
mutations pass; native suite,158 Python tests and frozen ledger check pass.
Function totals remain304 partial (299client/fiveengine), zero complete,
frozen18,783. Research adds no credit; existing window-slice006DC3A0 partial
coverage is not a reconstructed full normal tail.
