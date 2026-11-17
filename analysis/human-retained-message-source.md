# Human shared-entry retained messages and process delivery

D6b2b completes the bounded retained-message/queue/process-FIFO **source**
dependency. It does not complete D6b2c admission/current-application callbacks,
D6, H2, nonoffline application entry/frame/reset or desktop acceptance.
[Plan](p22-hum-plan.md),
[receipt](checkpoints/p2.2-human-retained-messages-20261112.json).

## Original boundary

Original client SHA256 remains
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
Research index preceded new exports. WORK `human-h2-queue-message-originals`,
`human-h2-queue-body-originals`, `human-h2-queue-process-originals`,
`human-h2-message-raw.asm`, `human-h2-reliable-raw.asm` and
`human-h2-queue-vtables.json` preserve C/raw/caller/vtable evidence.
Existing payload/ACK/retirement exports were reused. The message export batch
stopped at absent007459B0 after three exports despite exit0; its ERROR is retained.
The earlier absent00745520 export failure is also retained, not successful proof.

Queue vtables008165F0/00816614/00816638 select base/reliable/ordered receivers.
Message0081665C selects00745B40 write/00745BF0 read and00745B10 release. Native
pool00983150 owns records and bytes. Source typed byte/shared ownership adapts
that allocation/refcount boundary: pending/outgoing/incoming/completion/container
handles refer to ONE record and payload. There is no second packet pool, input
history or quantizer runtime. Pool/allocator/refcount/layout/SEH ABI equivalence
is unclaimed; exported allocator bodies are not newly credited implementations.

007459E0/00745AD0 do not produce message14. Source sequence stays unproduced
until actual enqueue or decode.00745EA0 acquires/copies payload before20, writes
length0C/channel10/queue08, then enters20 and invokes queue14+4*index. Index3
reaches native20 lock storage instead of a queue; source fails there under20.
The returned source weak borrow is not ownership or readiness.

00745B40 writes queue2/length16/channel6/sequence23/generation1, then copies raw
payload bytes and advances the bit cursor. Its48-bit prefix preserves byte
alignment in the actual container; no new padding is emitted.00745BF0 sign
extends length16 (`movswl` at00745C0E), reads remaining fields, allocates and
copies. Negative signed lengths and truncated copies remain checked failures
after the reached field prefix, without fabricated bytes.

## Queue and container behavior

-00745130 advances queue04 and writes message14 BEFORE outgoing10 append.
  Reliable/ordered00745520 additionally reads fresh clock and retains pending34
  with deadline `now+timeout2C`, interval200 initially.007451D0 takes outgoing FIFO.
-007456D0 first retires acknowledged pending records from the FRONT while signed
  message14<=queue28, then reads one fresh clock. At most7 overdue records are
  scheduled per call, strict signed `now>deadline`; each deadline uses the OLD
  interval, then interval doubles capped5000. Pending and retry copies share the
  same record. ACK receipt alone does not retire pending ownership.
-00745210 lifts sequence23 relative to queue08 via007450C0. A newer sequence OR
  one more than9 behind becomes08 (including a backwards move); otherwise local
  flag4 marks duplication. It appends incoming1C. Queue0's callback suppresses
  flag4; reliable/ordered queue identities are separately admitted by the callback.
-007455A0 rejects a lifted sequence<=completed24 or already in completion40.
  Otherwise it retains completion BEFORE base incoming admission. Queue1 takes
  incoming FIFO via00745190; queue2's00745920 only takes exact next48+1 and clears
  local flag4.007457C0 advances contiguous completion24 separately AFTER delivery.
-00746490 allocates the actual container sentinel BEFORE20, gathers each queue
  in ascending order only when port10!=-1, writes current connection08 into each
  outgoing record18, then serializes common header, ACKs and container. Empty
  nonforced output returns nothing; forced invalid-port output is ACK-only and
  retains outgoing messages. Container count writes8 bits while every record is
  serialized;256 records therefore write count0. This native truncation is kept.
  Container release00747130 is FIFO, after20 unlock and before socket send.
-00746390 reads ACKs and decodes ALL00747240 records under20 BEFORE any queue
  admission. A malformed later record preserves ACK mutations but admits none
  of the earlier decoded records. It then takes00747310 one at a time, comparing
  received one-bit generation against the FULL connection08; a mismatch retires
  the record. No invented parity-only comparison or admission is added.

## Callback, process owner and retirement

00746170 snapshots actual process414/workerD8 ONCE after receive admission.
00745FB0 locks20, tries queues0/1/2, takes one and unlocks.00745F20 detaches payload
and clears record1C/0C BEFORE virtual08 release; completion may still retain the
now empty record. The callback is outside20, while containing worker B4 still
protects the connection. Suppressed queue0 duplicates also retire their bytes.
The payload is freed before the next take. A throwing callback preserves already
taken payload retirement and leaves later messages/completions owned; it does not
run the final completion round or certify a completed worker callback.

Actual0072C360 reuses the SAME0072C170/0072C230 process430 FIFOs, combining
`(channel<<5)|flags`, choosing authority by mode bit1. Allocation/clock/copy and
430 append remain the existing source receiver. It does not dispatch a World
while holding20/B4. Queue counters1F8/204 now use C++ atomics for actual callback
versus host status observation; host predicate reads those existing counts rather
than unlocked STL queue links. Active worker selects the real queue owner, not
transport/World readiness. Payload mutation remains under the original430 lock.

0072D0E0's client branch submits only when334 is present, without418 or peer-stage
checks. Authority holds418, selects the requested slot or allFFFFFFFF, reads6C
BEFORE74 and submits only stage2 connections. There is no active08/local-peer
predicate. Flags split arithmetic-right-shift5 logical channel and low2 queue.
Submission is not an immediate socket flush or successful network delivery.

00745410 releases pending messages from the TAIL first, retires40 nodes/head,
then34 head.00744FC0 releases outgoing records from the TAIL, then retires1C
nodes/head and10 head. Native completion/incoming node destruction does not
release undelivered records; source RAII additionally reclaims those native
leaks. This differs from native allocator/leak behavior, not ownership order.
The connection still retires20 BEFORE ascending14/18/1C queue owners.

Source reset-time publication of mode7 uses SAME worker B4, keeping native
publication BEFORE wrapper deletion/worker stop while defining C++ synchronization
with callback reads. Connection08 publication is atomic too; source definedness
metadata still rejects reads without an actual producer. A generic cold backend without worker ownership retains its
original write; an owned worker lacking the publication endpoint fails honestly.
Other non7 mode writers and DirectPlay callbacks still require D6b2c/D9 composition;
no general concurrent mode mutation, callback readiness or full frame is claimed.

## Verification

100 ASan/UBSan/LSan families cover real message serialization (including a static
byte golden),0/1/3/4/7/32767 lengths, signed-negative length, missing sequence,
three queue types, shared pending lifetime, ACK retirement, strict deadlines and
clock wrap,7-retry cap/backoff5000, unordered/ordered gaps, duplicate suppression,
full generation filtering, absent port, queue3 required failure, full-decode
failure prefix, count256, callback snapshot/exception and reverse record teardown.

The same families run actual source worker/socket threads, receive reversed
fragments containing a600-byte message, dispatch through the SAME connection/
queues/captured414 into actual authority and presentation430 FIFOs, then reset
through actual C4/local packet owners with mode publication and ordered worker
retirement. OS/pump endpoints are explicit doubles. Prepared-host address04/stage74
are declared prior inputs, not recovered admission. The original frozen D6a cold
audit also passes100 on NEW output paths; unchanged frozen inputs are untouched.

Actual StartupSession i386 instantiation and launcher/probe compile/link pass.
Windows outputs are not executed. Temporary new audits are fingerprinted and
removed. No TSan, original/network/World/application/desktop execution claim.
D6b2c, D5/D7–D12 and D11's confirmed producer blockers remain open.
