# Launch gate and control-owner data initialization

Source-only recovery, 2026-10-30. This closes one concrete prerequisite of the
first launch delivery item; it does not complete integrated startup or a desktop
milestone. The full owner and the semantic name of its optional control interface
remain unresolved. No successful substitute device/interface service is supplied.

Follow-up: [startup integration](startup-integration.md) now supplies a concrete
test executable and resolves reached wrapper payloads to actual Win32 imports.
Original binary self-integrity machinery and full control lifecycle remain open.

## Confirmed launch dependencies

Existing read-only Ghidra exports and newly checked section-mapped assembly give
the following chain. [Evidence map](control-startup-map.json) fingerprints the
base client and verifies25 instruction/data anchors,46 direct calls and the
gate's PE zero-fill range. No engine implementation is credited.

| Original boundary | Evidence and current result | Remaining dependency |
| --- | --- | --- |
| 006DCAE0 | Controller allocation006C0750, setup call006DCB55 →006DBFC0, then006DCC52 →006C0820 only after setup succeeds | Complete main-loop and failure ownership, not just shared controller algorithms |
| 006DBFC0 | Registry/setup handling then006DC1AA calls0072E1A0 with ECX0097D070 before graphics initialization006DB0F0 | Setup wrapper, real device/owner order and protection operations |
| 0072DDD0 →0072DE00 | Static constructor selects0097D070; constructs subordinate storage and separately writes gate7 at0072DF71 | Full constructor and subordinate types; this body is context only |
| 0072E1A0 | Gate7, explicit field resets, two Win32 critical sections and32 appended records; now reconstructed | Actual caller composition with full owner |
| 006DC1EF..006DC225 | Setup continues006DB300,006DB3B0,006DB4C0,006DB4E0,006DB6E0,006DBE60,006DBFB0 | Preserve failure gates and intervening protection-state effects |
| 006C0820 | Recovered prepare006B65D0/reset006B6620 then initial state006C0CA0 | Full application ownership and concrete state |
| 006C0CA0 →0066EAE0 →00688040 | Normal video state/descriptor creation leads to video initializer; existing exports preserve actual bodies | State lifetime, subtitles/music and protection prelude |
| 00688040 | Indirect protection calls at00688081 and006880D7 precede descriptor installation; normal non-credits path includes sound-manager fade | Protection is also inside video initialization, not only archive setup |
| 0072E460 | Full shutdown invoked from006DC3A0; device reset/interfaces first, lock deletion, then gate7 | Full teardown remains unrecovered; native lock RAII alone is not this shutdown |

The protection calls are present in the static instruction path. This is not a
claim that their effects can be discarded or that current pseudocode signatures
are trustworthy. Their called implementation/state needs independent recovery.

## Implemented source contract

[frontend/control_startup.h](../recovered/frontend/control_startup.h) represents
the fields consumed by0072E1A0, not the entire0097D070 owner. Its gate is the same
borrowed word consumed by recovered sound/settings/controller code. The actual
setup caller supplies that owner, not a fabricated constant-gate service.

Initializer order: clear object4EE0/word503C; gate=7; word5054=50, word5058=0,
byte5060=1, words448/4E4=0; InitializeCriticalSection(owner+418), then+430;
byte5018=1; word5044/byte4DF8=0, handle4D3C=-1, service14=null; word5064=0,
word5068=50, word506C=300000; wrapper10/wrapper0C=null, byte08=0; callback414
identifies0072C360; append32 freshly initialized0xC4-byte records.

Existing records survive in order; initialization does not clear the vector or
insert at its beginning. When full, capacity grows by max(current count,1),
matching the original ordinary allocation path. Repeating initialization appends
another32; lock reinitialization itself requires the original surrounding
lifecycle, not repeated calls on live locks. Pointer resets do not release old
objects; do not treat this initializer as a shutdown operation.

Record constructor00725A40 delegates to reset00725A50. Reset writes exactly95
bytes: byte08; nine zero DWORDs40..60; zero DWORDs00/3C/6C/74/7C/A4/AC/B0/B4/BC;
bytesA1/A2 zero; DWORDs64/68=2,70=FFFFFFFF,B8=50. The other101 bytes are unwritten.
Native records track which bytes are known; backing zeros are never exposed as
original defaults. Reset preserves previously known unwritten bytes. Original
copy0072BD50 copies49 DWORDs; the native value copy preserves bytes and their
known/unknown mask. No independent behavior is inferred for opaque fields.

Callback414 is a typed identity in native source, not an executable pointer into
the original client. Context0072C360 packs an event and routes by gate1 to
0072C170/0072C230. Its service and destinations remain unimplemented and cannot
be enabled through this data owner. This limitation prevents claiming full ABI
or complete original manager recovery.

[host/control_startup.h](../recovered/host/control_startup.h) allocates actual
i386 Win32 critical sections, using InitializeCriticalSection and
DeleteCriticalSection. It rejects duplicate/out-of-range initialization; native
RAII releases partially created locks on failure. It owns only those two locks,
not devices, optional interfaces, callback services or the full original owner.
Native vectors, exceptions and byte-validity masks replace original ABI/SEH and
undefined stack reads. Consumers must finish before lock storage is destroyed.

## Gate writers: starting points, not a complete lifecycle

Constructor0072DF71 and initializer0072E1CB both explicitly write7. Shutdown
0072E4BC and device reset0072E5FA also write7. Other checked direct writes:
00722B78 computes1 or3 from application byte170;00722BB9 restores7 on the shown
failure path;0072330E/00723C26 write12 hex;00723F7F writes2;007240B0 writes7.
These runtime branches are not proven normal-launch transitions. Address-taking
sites and indirect writes still need tracing; a linear disassembly search is
not a complete cross-reference proof and can decode data as instructions.
There is no justified global rule that gate is permanently7 or permanently2.

The data initializer clears the same+0C wrapper consumed by SettingsControl.
Its absent-wrapper path is real, but this does not prove later startup leaves
it absent. Recover actual creation/call order before composing live settings.

## Validation and accounting

Temporary ASan/UBSan audit passes exact95/101 validity, all196 bytes across256
sentinels, record copies, eight existing-vector prefixes, append/capacity behavior,
lock-call visibility/order, both injected lock failures and checked offsets.
It composes existing SettingsSound with the actual initialized gate and existing
SettingsControl with the cleared wrapper; no invented control response is used.
Strict i386 composition links the real Win32 lock and LS3DF settings adapters;
the Windows executable is never run. Existing native checks and158 Python tests
pass. Five altered evidence maps reject. Temporary fixtures are removed.

Ghidra ran read-only/noanalysis against the separate research project. Exports:
analysis/work/launch-gate-owner-20261030 and launch-gate-lifetime-20261030.
The first request included invalid function entry0072DF60 and stopped after four
successful exports; the second used actual constructor0072DE00 and all five
exports report decompiled=true. No saved project or original game code changed
or executed. Do not infer success solely from headless process exit status.

Two census-backed partial client entries (0072E1A0,00725A50) bring280 partial
(275 client/five engine), zero complete, frozen18,783. Constructor wrapper,
copy helper, full-owner context, callback and Win32 APIs receive no new credit.
Live behavior and all five launch delivery checkboxes remain unchanged.

Next: inspect the00A59000 implementation/initial state and actual control/interface
creation, then close full application/video owner lifetime and compose reached
startup dependencies. Reuse this initializer; do not restart its gate/record
research or expand unrelated sound branches before reachability is established.
