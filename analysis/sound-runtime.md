# Sound runtime, registered data and resource lifetime

Source-only recovery, 2026-10-30. Follows [movie coordination](sound-movie-channels.md)
and [catalog ownership](sound-catalog.md). Live preview and launch behavior are
unchanged. No original executable, DLL or Proton was executed.

[sound_runtime.h](../recovered/frontend/sound_runtime.h) composes the recovered
voice, movie and environment phases of the full update, plus reset/start/stop,
tracked references, sector metadata and cached-sample cleanup.
[sound_manager_data.h](../recovered/frontend/sound_manager_data.h) supplies the
registered data owner; [module_registration.h](../recovered/frontend/module_registration.h)
recovers registry lifetime. [sound_services.h](../recovered/host/sound_services.h)
forwards actual engine methods and borrows a live application/scene view.

## Original bodies and ownership

| Original | Recovered behavior |
| --- | --- |
| 00432420 / 00432710 | Construct/find tracked sound, retain frame, refresh enabled/sector/base volume |
| 00432490 | Store track scale, apply base volume times scale, update frame |
| 004324C0 | Set group limit; signed cursor check, preserve prefix, append -1 |
| 00432860 / 004328D0 | Clear attached sector metadata / enumerate root sectors |
| 00432900 | Build metadata from two directed portal arrays |
| 00433330 | Root-crossing attenuation and distance with original float shortcuts |
| 004335D0 / 004336D0 | Sound-manager container destruction / final constructor |
| 004361B0 | Owned voice teardown: volume0, mode(0,1), update, frame release |
| 004368D0 | Full voice → movie → environment update |
| 0048D950 / 0048DA30 | Register module/name / unconditionally clear registry slot |
| 006B66B0 | Actual application activity/mode fallback predicate |
| 00437340 / 004374A0 | Stop pooled voice / lookup active frame; absent from frozen census |
| 004361F0 | Reset resources with clear-samples/retain-primary flags; absent from census |
| 00433840 / 004338A0 | Start / stop with independent force/bit1 gate; absent from census |

The [fingerprinted map](sound-runtime-map.json) checks 61 instruction anchors,
90 direct calls, nine client virtual targets, 21 engine slots and three PE
zero-filled ranges. Client SHA256 is
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`;
engine SHA256 is `12c61eed2aec0c45700ad0a5ddfd7cbb3cbf0ea15ec67de155790bb24e7756ee`.
Addresses are translated through PE sections, never assumed to be file offsets.

Static constructor00433590 supplies singleton00899728, size104 hex. Its entire
range is beyond the containing section's raw bytes and within its virtual size:
section RVA432000, virtual size15B568, raw size36000. Loader zero-fill therefore
defines the fields that the final constructor does not write. This strengthens
the earlier borrowed-state contract: an arbitrary uninitialized caller still
has unknown values; this particular global starts zero before constructor writes.

Final data: module6/name `Sound`, driver volume0, movie volume/current/target/rate1,
primary movie null, requested index-1; secondary target/current/rate0/movie null;
pending and transition metadata0; listener null, driver delay0, settled byte0;
four groups each limit3/cursor0/three -1 slots. Clock00899830 and catalog00899838
are separate owners. The registry0089BAD8 has 19 borrowed slots. Duplicate kinds
overwrite; destruction clears its slot even if a later module overwrote it.

The original manager destructor frees container buffers and unregisters. It
does not shut down engine voices/movies or clear sector userdata. The native
owner likewise performs no automatic engine operations; explicit shutdown must
precede live-owner destruction. Native containers free their own allocations,
so their allocation/lifetime ABI remains different from the original.

## Voices, samples and tracked references

Update snapshots gate value2 once before processing voices in reverse order.
Null/inactive slots skip; active slots lacking a frame deactivate and append
their index to the free list. A nonplaying frame is stopped unless flags4 is
set. Playing frames with flags10 add signed milliseconds times float0.001 to
live volume. The extended result is stored as float32, but the original extended
comparison determines whether it strictly exceeds target. Equality/unordered
retain flags10; greater clamps and subtracts10. Engine volume/update follow.
Callbacks changing the live gate do not change this frame's movie-phase gate;
the queued primary selection helper still observes the live gate.

Pooled stop00437340 leaves handle-1 untouched and invalidates every other handle.
An active valid slot asks the real sound driver whether it is initialized. When
initialized and a frame exists: disable, volume0, then transfer the owned frame
to its variant's extra-sample vector without addref, or release it if uncached.
Clear the voice frame/cache after transfer. When the driver is unavailable,
frame/cache references survive. Both paths deactivate and enqueue the index.
Owned teardown004361B0 uses volume0/mode(0,1)/update/release instead.

Playback context004345EE..00434601 proves the real I3D driver factory kind4
creates sound frames. Context00434C00..00434C13 proves cached primary frames
populate voice+14 with direct WORD+E4 retention. This validates the checked frame
release target used by the new adapter; it does not recover or credit the play
producer00433FA0 or restore00435240. Variant caches own primary plus extra frame
references. Reset clears primary first, then extras in reverse; names, catalog
loaded state and extra-vector capacity survive.

Tracked construction always resolves position/sector, even when disabled,
captures current engine volume, starts scale1 and increments WORD+E4 directly
with wrapping. Lookup is reverse. Existing entries update enabled; only enabled
entries refresh sector. They reread live volume as the new base and reapply the
old scale, so repeated registration can compound attenuation. Reset releases
tracked references, retaining vector capacity.

## Sectors and environment

Root-sector enumeration uses slot44 with callback, manager, filter10 and null
name. Callback00432900 attaches 12-byte metadata through sector userdata+EC,
then traverses both directed portal arrays in reverse. Only links to the root
contribute. Each contribution stores float32:
`area += radius * radius * float(PI) * (opened ? 1 : float(0.1))`.
The first array reads radius before the opened query; the second reads it after.
Clamp area above40; scale uses the original extended arithmetic:
`(1 - (40 - area) * float(0.025)) * float(0.9) + float(0.1)`.
Cleanup clears userdata before freeing metadata, retaining vector capacity.

00433330 resolves missing sectors from copied positions. Only a root/non-root
crossing adopts the other sector's scale+08; all other pairs use1. Distance uses
the original positions, ordered z²+y²+x², stored to float32. Signed raw bits below
322BCC77 yield0; squared distance within float1e-8 of1 yields1; otherwise square
root. This helper does not implement the separate portal-routing body00432BE0.

After movie processing, environment work is ungated. A missing actual application
returns before clock/driver work. Otherwise clock00899830 wraps by signed delta
bits, and the scene listener's world matrix is refreshed if necessary. Only a
signed clock greater than1000 triggers periodic tracking; reduction leaves1000
unchanged and maps1001 to1. With a prior listener sector, enabled tracks refresh
their sector and rescale when it changes. A changed live camera sector rescales
all tracks, including disabled ones. These attenuation calls use zero positions.

Nonzero driver delay subtracts signed delta with wrapping and clamps signed
nonpositive results to0. Until settled, nonzero delay uses the original INTEGER
factor `(2000-delay)/2000`, or0 when delay>=2000. Consequently ordinary positive
delays remain silent; delay0 applies actual configured driver volume and marks
settled. No smooth fade substitutes for that behavior.

## Reset, startup and engine boundary

Reset proceeds with gate2 set or actual application fallback006B66B0: activity
nonnull and mode3. Clear group slots/cursors; destroy voices, freeing voice/free
index/registration storage; release tracked references while retaining storage.
Unless retaining primary, invoke actual primary cancellation and release it;
secondary releases directly without explicit stop/close. Reset primary scale/
target/pending only when dropping primary; always reset secondary scale/target1.
Rates/request/metadata survive. Optionally clear sample caches; clear settled.
Listener, delay and sector metadata are not reset by this operation.

Start/stop use force OR clear gate BIT1 (value1), independently of reset's bit2.
Start invokes reset(1,1), actual catalog load, sector enumeration, listener null,
delay2000 and driver volume0. Stop invokes reset(1,0), then sector teardown.
Their reset may legitimately skip while the outer operation continues.

SoundServices checks sound-frame slots00/24/30/78/8C/AC against
1001C370/1001E610/1001BB10/100548D0/10054C40/10054F30. Scene+E0 is10047A90;
sector+44 is1001DE40; driver+0C/+14 are1007A840/1007A9B0. It calls the actual
UpdateWMatrixProc export1001E390 with ECX, and reads actual frame/sector fields.
Portal opened uses the original self-on-stack slot04; the portal's type-specific
factory/vtable target has not yet been established. No guessed target is asserted.
Enumeration catches native callback exceptions and rethrows after the DLL boundary.

The borrowed live SoundWorldView supplies actual scene and current camera sector,
and is reread as needed. Application00698500 and camera-owner0069A350/00464E90
context is preserved; the base camera's slot8C is purecall, so a concrete derived
owner must be recovered before live composition. This is an explicit dependency,
not a permanently empty world service. Application-mode fields are also borrowed
actual state, not a synthesized success callback.

## Small follow-up: primary playback position

00433810 (manager slot74) initializes a DWORD output to zero and, when the primary
movie exists, calls IShow+38. It returns the output regardless of HRESULT, without
gate or opened checks. The source helper lives in sound_movie_selection.h;
SettingsServices checks slot38 against engine10072400 and passes self/output
pointer on the stack. The engine method can leave the output untouched on error;
the client zero initialization is therefore observable. Millisecond conversion
stays delegated to the real engine. This body is absent from the frozen census,
so no new function credit is awarded. Playback producer/restore remain open.

Temporary ASan/UBSan audit covers no movie, success/failure with no write and
written outputs including the full unsigned range (15 calls). Strict i386
adapter composition compiles/links only; the fixture is not run. Existing158
Python tests pass; temporary fixtures are removed. Live behavior is unchanged.

## Tiny follow-up: indexed catalog entry

004326D0 (manager slot8C) borrows an entry from the existing catalog. Negative
group/entry indices and unsigned upper-bound failures return null; an existing
null entry also returns null. There is no gate, implicit load, retain or mutation.
SoundCatalog::sound_entry implements those checks in their original order.
A null in-range group is original undefined behavior; the native guard throws,
after the negative-entry check. The result remains borrowed from catalog ownership.

Temporary ASan/UBSan audit passes 42 boundary combinations, null-group guard
ordering and lookup after clear. Strict i386 compile/link passes; the Windows
fixture is not run. Native checks,158 Python tests and fingerprinted evidence
pass. Temporary fixtures are removed. The body is absent from the frozen census;
the ledger stays278 partial. Included with prior sound work in this source-only
commit checkpoint; playback/cache production and live wiring remain open.

## Verification and next work

Temporary native ASan/UBSan audit passes voice reclamation/cache transfer,
driver-unavailable ownership, group resizing, tracked volume compounding,
metadata and clock boundaries, disabled tracking, integer driver delay,
reset modes/cache cleanup, forced/gated startup, gate mutation during update,
registry/static initialization and capacity retention. Ninety voice fades and
12 portal calculations match independent hardware x87 arithmetic. The actual
DTA/LS3DF adapter composition compiles and links under strict i386 settings;
the Windows executable was not run. Existing native checks and158 Python tests
pass. Eight altered evidence maps reject, including file-backed/out-of-range/
empty zero-fill claims. Temporary audits are removed; no permanent tests added.

Fourteen new census-backed partial client entries bring the ledger to278 partial
(273 client/five engine), zero complete, frozen denominator18,783. Existing
004368D0 now covers the full algorithm. Absent-census start/stop/reset/reclaim,
constructor wrapper, release forwarder and delegated engine bodies get no new
credit. Native string/vector/class/refcount/allocation/SEH ABI, invalid-pointer
guards and unhandled original undefined paths remain limitations. No desktop
delivery is claimed. This source-only checkpoint includes the small follow-ups above.

Next: recover actual playback/cache population and restore (00433FA0/00435240),
spatial routing where required, shared gate/control lifecycle and concrete
application/camera/state/protection/cache owners, then compose original startup
and controller. Do not expand menu flows before that launch path is rebuilt.
