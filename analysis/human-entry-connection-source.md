# Human entry connection ownership — D6 source checkpoint

D6a recovers the **cold connection** and its consumed registry/entry receivers.
It does not close D6 or H2. [Receipt](checkpoints/p2.2-human-entry-connections-20261112.json),
[shared entry](multiplayer-world/shared-world.md), [Human plan](p22-hum-plan.md).
There is no new successful transport service or enabled application loading.

## Proven native boundary

Original client SHA256 is
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
Index lookups preceded exports. WORK is `analysis/work/mission-world-20261103`.
Existing `human-entry-connection-originals` and `human-control-entry-originals`
are reused; new `human-h2-worker-owners-originals` and
`human-h2-worker-retirement-originals` record the expanded lifetime research.
Raw `human-h2-connection-construction.asm`, `human-h2-connection-registry.asm`
and `human-h2-connection-entry.asm` validate the consumed ABI and ordering.
`human-h2-connection-vtables.json` preserves bounded slot00/003FFFFF PE reads.

- **00745D20, ECX this / RET12:** construct the actual0x50 wrapper.04 is the
  first stack word;0C the second;38 the third context pointer.08=0,10=-1,
  48/4C=0 precede clocks. The FIRST fresh00748960 sample writes44; the SECOND
  writes both3C and40. Allocate/construct14 through00744ED0,18 through00745350,
  1C through007458D0, then initialize20 critical section. Each NULL outer queue
  allocation is stored and construction CONTINUES; it is not a readiness error.
- **00744ED0:** own two doubly linked empty sentinels, native list heads10/1C;
  initialize04/08 to DWORD003FFFFF. **00745350** extends this with heads34/40,
  sequence24/28=003FFFFF and timeout2C=200. **007458D0** extends the latter with
  sequence48=003FFFFF. There are ten sentinel owners across one wrapper's queues.
  Allocator bytes and sentinel payload are unproduced and never read.
- **00745E40:** DeleteCriticalSection20 FIRST, then delete queues14/18/1C in
  ASCENDING order, skipping NULL. Cold00745410 destroys its40 and34 lists before
  calling cold00744FC0, which destroys1C then10. Populated destructors additionally
  release queued message virtual08 from tail nodes; those producers/receivers
  remain OPEN. The source has no synthetic packet enqueue or populated-success
  path. The checkpoint covers the reached cold branch only.
- **00746B50, ECX worker / RET4:** under workerB4, store workerD4 into the
  supplied wrapper08 BEFORE vector capacity/allocation. Append without identity
  deduplication. Growth is1 from empty, otherwise twice the current count.
- **00746D20, ECX worker / RET4:** under SAME B4, remove the FIRST matching
  pointer by swapping the last pointer into its slot and decrementing end. It
  neither deletes the connection nor clears its08. Registry retirement also
  does not delete entries. Separate00746890 deletes last connections repeatedly.
- **00730A40:** mode bit1 chooses authority. Under418, scan the SAME C4 records
  forwards: byte08 nonzero and stage74==2 toggle FULL AC with XOR1, then publish
  the fresh AC through peer6C+08. Client does not acquire418: toggle local AC,
  then publish it through process334+08. Failure on a missing wrapper occurs
  AFTER that row's AC mutation; earlier rows remain changed, later rows untouched.
  Re-entry toggles again. There is no boolean normalization or new peer roster.

## Source ownership and verification

`ControlConnectionOwner` owns the three cold queues and OS lock. It derives from
`ControlConnectionRecord`, the typed consumed08 view. The worker registry and
peer6C/client334 borrow this SAME identity. Source record copying preserves the
borrower; native record initialization clears6C without deletion. Positive DWORD
pointer fabrication is rejected. Unproduced generation reads remain required
failures. Cold007466E0 initializes an empty worker vector but DOES NOT initialize
D4; this source does not invent D4=0 at construction.

`StartupSession::make_game_connection` uses its existing00748960 clock basis and
real Win32 critical-section storage. `begin_game_entry_connections` routes to
00730A40 through its actual process records/local identity/mode. Neither is a new
host/join service; no application entry path has been enabled. Registry B4 uses
the existing process lock-service index2, distinct from418/index0 and430/index1.

Portable typed pointers, geometric STL vector and RAII replace original layout,
allocator, vtables and SEH. Queue outer allocation sizes differ with C++ layout;
NULL continuation order is preserved. Each sentinel remains a separate owning
allocation, with inaccessible native padding/payload. A throwing constructor
reclaims acquired source storage through RAII; no native exceptional leak or
bitwise allocator equivalence is claimed. Source RAII releases B4/418 on a
checked failure; native fault/unwind lock behavior is not established. Failure
prefix claims concern mutations BEFORE that failure, not native recovery after
a fatal pointer dereference. No protocol sequence mutation, retry,
acknowledgement, packet-pool or live thread implementation is supplied here.

Temporary native `human-h2-connection-records-native.cpp` runs100 families with
ASan/UBSan/LSan. It uses ACTUAL source cold connection/queues, typed C4 records,
registry and00730A40, not connection/registry substitutes. OS clock, locks and
allocation endpoints are explicit doubles. It checks distinct clock reads, all
three NULL outer queue cases, lock-before-queue retirement, positive full-word
XOR in modes1/2/3/7, duplicate registration, swap-last first removal, retained08,
allocation failure after08 publication, missing wrapper partial prefixes, record
copy/reset and re-entry. Only prior workerD4 is explicitly seeded: this is not a
worker-start test. Strict i386 instantiates actual StartupSession clock/Win32 lock,
constructor, registry and entry receivers; launcher/probe compile/link separately.
No Windows output or original binary is executed; no desktop claim. Temporary
audits remain in ignored WORK until D12's consolidated cleanup.

## D6b remains required

The source checkpoint is intentionally separate from the containing worker and
caller lifetime. Expanded native evidence establishes these next dependencies:

1. **007466E0** constructs inline socket owner007474E0 BEFORE B4; writes worker
   A4=0/D8=0, leavesD4 unproduced. Socket construction owns two0x28 sentinels,
   empty vector,32768/800-byte buffers, socket=-1,24=0,28=1742BDCD,7C=0/80=0 and
   critical section54. **00746750** deletesB4, retires socket007475B0 and frees the
   pointer vector; it does NOT stop threads or delete the vector's connections.
2. **007467B0** creates worker completion eventB0, starts actual socket00747690,
   resets vector end, setsA4=1/AC=0/D4=0, publishes SAME fresh clockCC/D0, then
   creates007466D0→00746EB0 thread. Socket start does actual Winsock/socket/bind,
   starts00748370, then creates its completion/wake events. Preserve this order;
   neither callback is a delegated OS implementation or successful empty worker.
3. **00746840** when active setsA4=0/AC=1 before waitingB0(1000), stops socket
   00747750, then closesB0. Socket stop sets70=1, signals78, waits74(1000), closes
   both events/socket, writes socket=-1, WSACleanup. Native waits are NOT proof
   of safe source thread joining. Recover actual callbacks and caller teardown.
4. **00723D20** publishes actual process334 BEFORE port10=descriptor port+1 and
   registration; **007260A0** publishes peer6C and zeroes40..60 under418, UNLOCKS,
   THEN registers. Disconnect unregisters first, deletes actual wrapper, clears6C.
   Preserve failure prefixes and SAME worker/record owner, not a second service.

D7 status9 and D8 received0 composition still depend on those lifetimes. Actual
live synchronized transport acceptance is Phase3, but these reached source
owners/callbacks cannot be replaced by successful placeholders in P2.2. Existing
D11 unproduced part5F9/wheel695/EXPL8/smoke14 findings remain unchanged blockers.
