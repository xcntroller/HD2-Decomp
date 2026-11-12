# Human shared-entry fragment reassembly source

D6b2a is a bounded source checkpoint within selected H2. D6b2/D6, H2, actual
application integration and desktop acceptance remain open. The received queue
payload/retry/order and DirectPlay/application callback composition are now
separately visible as D6b2b/c in the [Human plan](p22-hum-plan.md).

## Original behavior and ownership

Original client SHA256 remains
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
The research index was consulted before exports. Relevant existing00747A40
was reused; new00747DD0/00747CB0 and sort helpers were exported read-only.
Ignored WORK `human-h2-payload-entry-originals`,
`human-h2-fragment-owners-originals`, `human-h2-fragment-chain.asm` and
`human-h2-fragment-sort.asm` preserve the C/raw contracts. The first export
batch stopped at missing00745520 after four successful exports despite exit0;
its ERROR remains in `human-h2-payload-entry-ghidra.log`. No absent function or
generic delegated sort/allocator helper receives new inventory credit.

00747A40 first receives/decrypts/checks the datagram using existing00747F50 and
00747800. Received-byte counter1C advances before checksum rejection. For a
high-bit fragment, it allocates/copies bytes after the five-byte header BEFORE
reading a fresh clock and appending a32-byte record to socket30/34/38. The
record is id00 (low31 bits), index04 (low7 bits), payload08/length0C, final10
(header id high bit) and clock14. There is **no sender identity in this record**.

00747DD0 sorts ascending unsigned id/index using00747C90, preserving the
original partition/median/insertion scheme rather than an arbitrary equal-key
library ordering.512 bytes/16 records is its sort threshold, not a capacity
limit. Only AFTER sorting does00747CB0 read its fresh clock. Candidate expiration
is strict signed `now > stored_clock + 20000` with32-bit wrapping. Erase shifts
the remaining records and rechecks the same index. Candidate searches require
index0 followed by exact contiguous indices of the same id, ending at final10.
Duplicates are retained, not deduplicated; gaps/duplicates can block one start
while a later start/run is eligible. Once a run is being inspected, subsequent
members are not independently aged. Selection does not drain every eligible
run; it returns at most one for each newly received fragment.

The native final nonterminal probe at00747D58 fetches next.id before checking
logical end. At end, either value gives the same false selection result; the
source bounds that irrelevant read without fabricating an extra record.
00747CB0's expiry and vector destruction do not free displaced packet buffers.
Portable byte owners reclaim those native leaks; native allocator/layout and
leak reproduction are not claimed.

00747DD0 copies each selected payload into the SAME socket14 receive buffer,
frees it and erases its record before advancing to the next. The source retains
that incremental mutation: overflow of the32768-byte buffer fails at the current
copy with prior records already removed. Truncated metadata also fails after
the earlier receive/counter/checksum prefix. No padded bytes or successful
oversized packet are invented. The completed packet keeps **the address/port
of the datagram that triggered this assembly**, including mixed-sender runs.

Socket stop/start does not clear30/34/38. The SAME retained records survive
restart and are retired with storage after54, buffers14/18 and FIFO4C/40,
following007475B0. There is no new socket, packet pool, lock, process callback,
input-bit owner or readiness certificate. The worker still invokes this source
receiver inside its existing00746A10 callback before connection lookup/delivery.

## Focused verification

[Receipt](checkpoints/p2.2-human-fragment-reassembly-20261112.json) fingerprints
source, exports/raw and temporary checks before their removal.

100 ASan/UBSan/LSan families pass actual socket send/receive/reassembly with
500/501/1000/1001/8500/32768-byte reverse-order round trips (including more than16
records), sender mixing, stop/restart retention, checksum/truncated metadata,
unsigned id order, gaps/duplicate starts, strict TTL equality/wrap and candidate
scope, zero-byte completed runs, absent buffer failure and incremental overflow.
The send kernels are drained synchronously against explicit UDP doubles to
isolate reassembly from native wake/reset scheduling; source socket callback
threads also run. An initial audit waiting on rapid per-fragment wakeups timed out. This is
consistent with a signal arriving between native drain and reset; it is not a
controlled reproduction of every such schedule. No new event barrier or source loop
change was introduced to suppress native scheduling behavior.

The adapted prior worker regression passes100 source worker/process/control
families and200 same-worker start/reset cycles, plus actual worker callback
reassembly of an ACK onto the SAME client. It preserves required populated
message failure, callback timeout/retry, source ownership and reverse teardown.
OS socket/event/lock/clock endpoints remain doubles. Frozen prior audit inputs
and outputs are untouched; new temporary inputs use new paths.

Strict i386 checks instantiate actual StartupSession worker and socket receive;
launcher and startup probe compile/link only. No Windows/original binary,
network session, populated World or desktop execution is claimed. No permanent
test is added. D11's confirmed fresh-part5F9/wheel695/EXPL8/smoke14 producer
blockers are unchanged; these fragment receivers do not resolve them.
