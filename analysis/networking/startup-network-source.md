# Startup and DirectPlay ownership — working contract

The full N1 startup/network checkbox remains open. This research covers its
bootstrap, ownership, native DirectPlay host acquisition and cleanup dependencies;
transport acquisition alone cannot replace the required application/profile,
archive/module, world-readiness or callback receivers.

Original HD2DS fingerprint is fixed in [manifest](binary-manifest.json). New local
read-only/noanalysis Ghidra exports: `server-bootstrap-20261109`,
`server-bootstrap-deps-20261109`, `server-transport-inputs-20261109`.
All used entries have successful fingerprinted JSON/C pairs. Static assembly of
00779450 and the installed dplay8.h/dpaddr.h definitions resolves indirect calls.
Original executables, DLLs and Windows fixtures have not been executed.

## Bootstrap completion boundary

00725850 owns the manual-reset Win32 event,00706120 application owner, message/
engine/network/controller loop, archive reader and ordered teardown.00725260:
COM MTA →007251D0 executable-directory selection →TEV(name,22Fh) →00787EA0
network owner →00724450 IGraph →007245C0 title →00724630 I3D →00724660 settings
→007247A0 archives →00725120 eight resource directories →00725240 language/module
owners →007F1240 session/AI-related consumers →C runtime random seed.

The required headless engine is LS3DRF.dll, not the main client's LS3DF.dll.
Static PE export mapping locates TEV at10030A40.10030A4A compares the second
argument with22Fh,10030A51 sets AL=1 and10030A59 returns for that exact input.
It is therefore not an unresolved protection computation on this reached path.
This engine entry is delegated and receives no server/client ledger credit.

00725340 closes network00788210 and settings006F8DD0, module registry004A3AE0,
remaining consumers and caches, then I3D/IGraph,16 archives and COM.00706180
retires the application critical section and polymorphic state. Bootstrap failure
and later normal shutdown must retain original ownership order without a default
successful backend. Initial settings/profile defaults and module callbacks still
need their actual containing owners before integration can be credited.

## Native transport contract

00779450 first closes prior work007882B0, then performs additional logging/config/
autorestart/UI preparation. The native transport acquisition portion is:

1. CoInitialize(NULL); create DirectPlay8Server with CLSCTX_INPROC_SERVER.
2. Initialize flags0 with original callback thunk0040C6A8 →0077DCB0 →0077DCF0.
3. EnumServiceProviders twice, filtering the supplied provider GUID; first call
   supplies buffer size, second fills it. Original reads the first provider GUID
   without validating count/size; native storage must validate this boundary.
4. Create DirectPlay8Address with CLSCTX_ALL; SetSP with the **requested** provider,
   not the enumerated first GUID. GetSP writes the saved GUID slot; result ignored.
5. If requested provider is TCP/IP and selected port is nonzero, AddComponent
   `port`, DWORD/4 bytes. LAN uses31567; Internet uses configured+220. Original
   ignores AddComponent failure and lets Host determine its resulting HRESULT.
6. Application description:72 bytes on i386, flags1 (CLIENT_SERVER), zero instance
   GUID, original application GUID, max/current players0, supplied wide session
   name, no password/reserved data. Admission limits/password are separate callbacks.
7. SetCaps flags0: size20, connect timeout2000, retriesFFFFFFFF, keepalive4000.
   The original ignores SetCaps HRESULT. Set host mode1 dedicated/3 listen before
   synchronous Host; one device address, null security/credentials/player context,
   Host flags0. Preserve actual Host HRESULT; failure resets mode7.
8. Release temporary address; retain the real server endpoint. Optional voice,
   GameSpy, serialization/session initialization and player state follow in original
   code and remain required containing services, not successful no-ops.

007882B0 cancels all async operations (handle0/flags8000h), closes server with
flags0 and releases it before retiring callback-owned state.00788210 also closes
auxiliaries, serialization owner and both critical sections. Callback context must
remain stable through Cancel/Close/Release; no UI/world pointers may expire early.

The [source transport component](../../recovered/networking/directplay_server.cpp)
requires a genuine caller-supplied handler, passes
borrowed API messages only during that callback, and cannot signal game readiness.
It must not expose an acquired COM host as a completed session. A missing handler,
API acquisition failure or malformed provider metadata must fail explicitly.

## Session-name discrepancy

0077BC50/0077BD50 (ACP/UTF8) compute length+1 for sizeFFFFFFFF, then call
MultiByteToWideChar with source length `-1` and **destination capacity length** while
requiring the API to include its NUL. The original ignores the return and writes
its own NUL at the final index. For an ASCII name the supplied output capacity is
one wchar short. The caller does not establish what prefix the API leaves on
failure; static evidence alone cannot certify partial output or its runtime name.
Assembly0077BC6B..94 and0077BD6B..97 confirms this; it is not a decompiler guess.
The transport boundary therefore takes an already-owned wide name explicitly;
no guessed conversion or uninitialized stack contents are introduced. This does
not reconstruct the original name helper or solve its integration discrepancy.
Actual configuration-to-wide-name integration remains open and must record its
safe behavior and original-fidelity implications.

## Verification boundary

The production component compiled and linked against the Windows i386 SDK;
static assertions verify the72-byte application description and20-byte caps.
The temporary native shim compiled the same production .cpp under ASan/UBSan/LSan:
21 acquisition/metadata/HRESULT cases and4000 lifecycle cycles passed. Cases cover
COM changed-mode/reference balance, each acquisition stage, malformed provider
size/count, ignored GetSP/port/caps errors, actual Host HRESULT, owned names,
callback exceptions, joined worker callbacks and callbacks during cleanup.
Temporary sources and binaries are hashed in the
[verification receipt](startup-network-verification.json), then removed.

These injected COM objects establish call/ownership behavior only. Live COM/runtime
compatibility, discovery and a genuine client/server session remain unverified.
Only00779450's native TCP/IP acquisition and007882B0's server COM cleanup subsets
receive partial ledger credit. The actual game callback0077DCF0, voice, auxiliary
sockets, player queues, profiles, engine/module owners and full bootstrap remain
dependencies. This does not deliver an independently runnable server.

The transport requires lifecycle calls on its constructing thread and a stable
caller-supplied handler/context through stop. It balances only successfully
acquired COM references; RPC_E_CHANGED_MODE acquires none. Provider bounds checks,
exception-to-HRESULT conversion and defined owned-name input are deliberate native
safety boundaries, not claims that the original performed those checks. No
delegated Windows/engine API implementation receives executable-function credit.
