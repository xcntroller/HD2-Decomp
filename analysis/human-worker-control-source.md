# Human worker and control lifetime — D6b source checkpoint

This completes **D6b1's source worker/socket/control lifetime**, not the containing
D6b/D6/H2 checkbox or a live multiplayer session. [Plan](p22-hum-plan.md),
[receipt](checkpoints/p2.2-human-worker-control-20261112.json),
[prior cold owner](human-entry-connection-source.md). Actual DirectPlay admission,
current-application callback composition remains D6b2c. Retained messages and
process414 delivery now have [D6b2b source](human-retained-message-source.md);
received fragments have
[D6b2a source](human-fragment-reassembly-source.md).

## Storage and shared ownership

The native client SHA256 is
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
Research index precedes new Ghidra exports; bounded assembly validates signatures
where default C is misleading. WORK uses new `human-h2-worker-*`,
`human-h2-socket-*`, `human-h2-empty-*` and `human-h2-client-*` variants.

- Native singleton is0097D070. Client constructor00723D20's raw ECX proves worker
  **process+33C**, with socket **+34C**. Thus workerD8 **IS process414**, the
  existing packed-event callback0072C360. No second callback or transport owner.
- PE `.data` starts00832000, raw size00036000, virtual size0015B568. WorkerD4
  (0097D480) and socket74/78 (0097D430/434) are in the loader's zero-filled tail.
  **Constructor omission does not make the actual singleton unproduced.** Generic
  registry construction still leavesD4 undefined; only the explicit process-static
  storage tag supplies this loader producer.007467B0 subsequently writesD4=0.
-007466E0 constructs007474E0 socket BEFORE B4, then writes A4=0/D8=0. Source
  clears SAME process.callback414; actual0072E1A0 later installs the existing
  packed-event callback. Socket owns its two empty0x28-sentinel lists,32768/800
  buffers and54 critical section. B4 belongs to the worker;418/430 stay distinct.
- One source scratch owner represents process009830E8. It reuses the existing
  0073CF50/0073D000 bit kernel, retaining unwritten bits between messages. It is
  **not a Human input/quantizer owner**. Initial allocation padding is zeroed
  and unclaimed, as in the existing portable bit-buffer adaptation.

## Start, callbacks and retirement

007467B0 creates workerB0 completion event, starts00747690 socket, resets registry
end without deletion/capacity release, publishes A4=1/AC=0/D4=0, takes ONE fresh
clock for CC/D0, then creates007466D0→00746EB0. Socket start preserves actual
WSAStartup/socket/bind and starts00748370→00748220 BEFORE publishing74/78 events.
No artificial publication barrier is inserted: early callback reads observe the
real static zero or a previous closed handle. Both loops set thread priority2.

00746EB0 repeats00746DA0 maintenance then00746A10 receive until AC. Maintenance
holds B4 over forward connection visits; its later clock difference is signed
wrapping and strictly>500 before00746E00(true). That receiver publishes CC BEFORE
B4 iteration. Receive forwards one eligible fragment, polls UDP, publishes D0,
reads type/identity and takes B4.00746900 takes B4 AGAIN internally, returns the
first matching04, then endpoint10/0C publication precedes00746170. The recursive
lock is preserved. Caller mutation of04, CC/D0 and socket event/stop fields uses
C++ atomics where native shared fields otherwise create C++ data races.

00748220 waits wake78, drains00748180, resets wake78, then rechecks70. Drain pops
FIFO under54, UNLOCKS for send/free, and relocks. Sending updates SAME native
sockaddr fields and20 byte counter, ignoring raw send return as original.

00746890 deletes LAST connection through its actual owner while holding B4,
then decrements fresh end.0072E4E0's source path now calls reverse deletion,
clears334, then00746840, BEFORE418 record retirement and input-manager shutdown.
00746840 publishes A4=0/AC=1 before B0(1000), then00747750 socket stop, then B0
close. Socket stop writes70, signals78, waits74(1000), closes74/78/socket,
writes socket=-1 and WSACleanup.00746750 storage retirement deletes B4 BEFORE
socket54/buffers/list4C/list40/vector storage, then registry capacity.

Native timed waits are **not** proof that a callback has stopped. Source retains
actual thread handles and joins before destroying their storage. Pending joins
raise a checked failure with ownership retained; source-only `stopping` metadata
allows retry even after native A4 cleared. Callback exceptions remain failures,
not synthetic native completion signals. Additional RAII fallback joins before
storage destruction and consumes throwing pre-A4 acquisition prefixes. This is
source exception safety, not native timeout/fault recovery or handle-leak parity.

## Cold control protocol and caller prefixes

-007460F0 uses signed `clock44+2000 < now`, then00746000 modulo256 ping sequence.
  Identity and sequence copies precede20 acquisition. Ping header is type2 then
  identity, direction1 bit, sequence8 and timestamp32. Reply elapsed zero clamps1;
  only matching sequence publishes4C. Ping send remains UNDER20.
-00746120 refreshes3C, runs actual cold queue maintenance (including reliable/
  ordered fresh clocks), then00746490's real empty container sentinel acquisition
  before20. Header type1/identity8, two23-bit ACK values with one-bit suffixes,
  then empty count8. ACK identity copies precede header serialization. Builder
  unlocks20 and retires its sentinel BEFORE socket send. No outgoing payload
  producer is invented.007450C0 sequence lifting is implemented and checked with
  wrap/old-ACK goldens; absent frozen virtual helpers receive no new credit.
-00746170 retains endpoint/40 and ACK progress before a required populated-message
  failure. Empty ACKs use SAME callback414/D8: cold00745F20/00745FB0 take then
  00746630/007457C0 completion rounds preserve their separate20 lock scopes.
-007477A0 checksum sums complete plaintext DWORDs, writes low24 header bits and
  XORs complete body DWORDs with fresh key28+length*2048. Remainder bytes stay
  unchanged.00747800 reverses and checks that checksum.00747860 uses direct queue
  below500; at500 or above it increments24, splits500-byte pieces, writes index
  high bit and final flag, and defers them.00748070 forwards at most ONE when
  signed now is STRICTLY above shared00983100. Empty payload envelope is3 bytes;
  truncated received headers remain checked failures, not fabricated padding.
-00723D20 connection prefix retains SAME process338 address, constructs its owner,
  and publishes334 BEFORE port+1/registration,
  then force-flushes.00722180 FFFF0005 success publishes SAME00982104 masked7F
  BEFORE334+04, with no premature slot bounds gate. Failure byte to5044 is
  unmasked. FFFF0016 unregisters, deletes, clears334, then5044; application
  status11 routing remains the containing caller, never deletion of a World here.
-007260A0 CREATE_PLAYER tail takes actual PREPARED record04 (producer00724DC0),
  publishes multimediaA8/A1, constructs and publishes6C, clears40..60 under418,
  UNLOCKS, then registers. Missing04 retains its native earlier prefix. Destroy
  unregisters before delete then6C clear. INDICATED_CONNECT_ABORTED clears00/name,
  unregisters/deletes but **does not clear6C**. Source weak lifetime metadata rejects
  use of that stale borrower; it neither silently returns NULL nor prolongs ownership.

`StartupSession` contains this ONE worker as its LAST member: callbacks retire
before every borrowed process/platform owner. Real Win32 socket/event/thread
imports are compiled through a concrete platform; launcher/probe link ws2_32.
Cold reset admits this owned worker, while existing non7/interface/peer guards
remain required. No new host/join application flow has been enabled.

## Focused verification and remaining work

Native ASan/UBSan/LSan PASS100 families, **200 same-worker start/reset cycles**,
with ACTUAL source worker/socket/connection/C4/entry/cold-reset owners and source
callbacks on real native C++ threads. OS socket/event/lock/clock, graphics flags
and firewall endpoints are explicit doubles; no actual Windows/network execution.
Tests force early event scheduling, genuine suspended-callback timeout/retry,
throwing thread acquisition, identity publication/failure prefixes, host418-unlock
ordering, callback414 alias, ACK/ping locks/deadlines/wrap, checksum rejection,
fragment499/500/501/1000/1001 boundaries, abort borrower invalidation, and callback
failure without a native completion certificate. Required populated-message and
fragment failures preserve mutation/cleanup prefixes. Prior D6a100-family cold
regression uses NEW output paths. Actual StartupSession i386 and launcher/probe
compile/link are separate checks; no permanent tests or desktop claim. No TSan
claim; it is unavailable in the local sanitizer runtime.

**D6b2 remains:** compose actual DirectPlay admission/name/address/record and
completion/termination callbacks with current application11, preserving one
client04/server00 owner. Retained00747240 message/payload receivers, queue retry/
ordering and actual414→process430 FIFO delivery are now D6b2b source-verified;
[contract](human-retained-message-source.md). Application entry/frame remains open.
00747A40 retained fragment reassembly is now D6b2a source-verified;
[its contract](human-fragment-reassembly-source.md) records lifetime/failure limits. Callback414 presence, A4 or thread creation cannot mean world
or transport readiness. D7/D8/D9 actual entry remains open. Known D11 fresh-part5F9,
wheel695, EXPL8 and smoke14 blockers are unchanged; no defaults were added.
