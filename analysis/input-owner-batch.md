# Input manager and binding configuration — source completion batch

2026-11-03. The previously verified scalar/input batches are committed as
`0c3527d`. This next batch implements **14 complete original bodies**, source-only,
from the17 candidates in the previous queue. Three retain specific recovery work.
The expanded pre-Phase2 completion pass remains open.

[Source](../recovered/input/client_input.h), [17-entry disposition index](input-owner-batch.json),
[full original bodies and calls](input-owner-batch-map.json).

## Delivery boundary

The new `hd2::input::client` entries have original Win32 receiver, stack, record and
field contracts. They are compiled and exercised together in a temporary audit.
They are **not wired into the accepted desktop host**. Its checked `InputManager`,
profile/settings adapters and ownership guards remain separate. No UI behavior,
profile format or user data changes. Source completion does not imply desktop
integration or completion of the old host adapters' distinct safety policies.

The CRT private heap and original engine/IJoy methods remain delegates. Rebuilding
their callers earns no credit for those bodies. The pointer-triple end-insertion
helpers cover the contracts actually used here; their generic middle-insertion
functions receive no new credit. No functions are added to the frozen ledger.

## Completed entries

| Original entry | Complete source contract |
| --- | --- |
| 006C6A20 | ECX initialization, EAX=this; four pointer-derived allocator bytes, null pointer triples/graphics and20/20/20; preserve every other byte |
| 006A81C0 | ECX initialization, EAX=this; byte0=this>>24, preserve three padding bytes, empty triple and mode2 |
| 006C6FF0 | Reset binding/first/second ends; zero scan/key counts and only four mouse count bytes; preserve devices, capacity, gates, limits and flags |
| 006C6B60 | Reset output lengths before callbacks, initial joystick count with freshly resolved base, reverse null-checked +08 releases, clear joystick end afterward |
| 006C6CC0 | AL/RET12 attachment: validate graph/every device before mutation; forward +04 retain/append, no rollback, key gate/limits/flag finalization |
| 006C7480 | AL/RET8 installation: repeated growth, output-growth branch, wrapped counts, map publication, old-kind reread, partial failures, joystick configuration, unknown/null replacements |
| 006C7440 | AL/RET20 pack/forward method on caller's ECX manager; truncate kind/code to WORD, preserve direction/device DWORDs |
| 006A8600 | Thiscall/RET4 setting application through relocated global manager; device0, ignored installer result, dirty clear only after return |
| 006A8630 | Thiscall/RET4 reverse exact-mode pass, initial count/fresh base lookup, irrespective of dirty |
| 006A8690 | Thiscall/RET4 modes2/3 only; apply before storing selected mode |
| 006A86C0 | ECX/RET0 dirty pass, one entry gate, reverse iteration, mode reread per record, other tags shared |
| 006A8C00 | ECX/RET0 primary defaults: clear, append all92 ordered records, apply dirty in current mode |
| 006AA090 | ECX/RET0 alternate defaults: same ownership/order contract with all92 alternate records |
| 006A8BE0 | Thiscall/RET4 selection: alternate only for selector1, primary for every other DWORD |

The earlier queue incorrectly described006C7440 itself as choosing the global.
Disassembly shows it preserves its ECX manager. Its caller006A8600 loads008AEA48
at006A8618. Both methods now have their actual receiver/stack boundaries. The
source relocates that global as a C++ data symbol, with zero-initialization followed
by explicit pointer-derived initialization and native lifetime registration. This
registration is not original CRT/SEH recovery credit.

## Storage, failure paths and opaque bytes

The i386 assertions cover joystick begin+08/current+04/secondary+48 and8C stride;
scan map+20, key counts+60, six mouse counts+A0, limits+A8/+AC/+B0, attachment
byte+B4, binding begin+BC, output begins+CC/+DC, key gate+E8 and mouse gate+F0.
Configuration is0x14 bytes, with pointer triple+04/+08/+0C and mode+10.
Native64 audits use their host pointer sizes; those are not claimed as original
layouts. Placement tests compare every surrounding byte, not just field values.

All original insertion calls in this batch insert one record at the current end.
Growth is `size + max(size,1)`; no reserve-all or resize-all shortcut. Each completed
append remains visible if a subsequent allocation/handler throws. Relocation copies
raw records without another retain, and frees the old allocation before publishing
new pointer fields. Inline middle-copy loops in attachment and clear/detach are
unreachable with those fixed current-end arguments; their bytes remain in the map.
The four free operations remain second/first/bindings/joysticks, including nulls.

Important correction to older allocation assumptions:007E520F calls007E81D9 with
handler use enabled. At007E81EE..007E8204, failure calls007EF932 and **returns null**
if the handler is missing/false.007EF932 reads handler0098BF48, returns1 only for a nonzero handler result, and does not invent a throw.
A handler can itself throw. The source delegates heap storage through malloc/free,
rounds zero/minimum storage as the original ordinary heap branch does, preserves
the FFFFFFE0 size cutoff and retry/handler result contract, and exposes the allocator
boundary for native fault injection. Original private-heap internals are uncredited.

Copy helpers006AC840/006C9210 and inline binding/output copies skip only destination
address zero. A failed first append can therefore publish null begin with end and
limit equal to one record's byte size. Integer address arithmetic preserves that
state without introducing C++ null-pointer arithmetic. Attachment of one device
can still return success, with an outstanding retain and no accessible record.
The audit covers this defined, non-faulting path. Later low-address reads/writes
remain original raw-memory preconditions; their instructions are statically
accounted for rather than executed under sanitizers. The checked desktop owner
keeps its native cleanup policy. A throwing handler leaves completed prefixes and
the latest retained device outstanding in the source ABI entry.

Default records reuse one stack scratch record. Construction writes bytes00..18;
all1C bytes are relocated. The three opaque trailing bytes are propagated, never
assigned a fabricated default. Original006AA050 ignores its eighth argument and
preserves those bytes. The existing independently recovered184-record data table
is reused, without repeating its decoder audit. New audits check all meaningful
fields, dirty eligibility and capacity across both layouts and valid/invalid modes;
an initialized opaque-byte fixture separately verifies full-record relocation.

Graphics map publication uses stdcall explicit-self +B4; IJoy +04/+08/+30 use
thiscall, with DWORD selector/flags for +30. Results are ignored where the original
ignores them. [Existing interface evidence](graphics-input-services-map.json) and
[IJoy contracts](joystick-devices-map.json) supply the verified method identities.
Reference callbacks/borrowed active records retain original valid-storage
preconditions; new host bounds exceptions are not added to these raw entries.

## Three remaining candidates

These are recovery work, not external blockers or blanket Phase2 deferrals:

- **006C7870:** the raw manager and actual snapshot layouts now exist. Port the
  producer to them, including exact FILD/FSTP, clamp/divide, ordered comparisons,
  signaling NaNs, caller CW/status/TOP and both POV branches. Remove native output/
  POV bounds exceptions only in the raw entry and account for all original paths.
- **006C6AB0:** `destroy` implements the normal reverse-detach/free sequence; original
  handler0080B255 and its MSVC exception-registration/unwind state3 remain to recover.
  Native lifetime registration is not equivalent to that body. Keep it partial.
- **006DB620:** raw clear/attach and joystick view are available. Recover its actual
  GetIGraph/global call boundary, language006B67B0/text006B1090 intrusive string
  return/release ownership and handler0080C048. The host startup template remains
  partial; ordinary locale success does not close its original string/EH ABI.

## Verification

One consolidated sweep passes existing native regressions,158 Python tests and
strict application/preview/startup targets. A later concrete null-allocation finding
required repeating only the affected raw sanitizer/i386/evidence checks.

The temporary ASan/UBSan fixture exercises actual raw entries:40000 differential
installer transactions; all256 keyboard counts; constructor return/write bounds;
callback-mutated old kind and mode; growth/capacity/output preservation; scan-map
and joystick failures; ignored callback failures; reverse reference release;
12 x87 control modes with preserved exception flags/stack;48 layout/mode combinations;
70 interrupted defaults builds; allocation throws and null returns; opaque-byte
relocation; explicit free order. Custom failure cases balance leaked references in
audit cleanup only. Strict i386 fixture compilation/linking passes, never executed.

All17 full original body hashes/ranges match the frozen inventory. The map checks
32 anchors and266 calls, including allocator/record helpers. Emitted code inspection
checks all15 raw methods (including the partial destructor), correct stack cleanup
or verified tail exit, relocations and body digests. Constructor comparison using
`-O2 -mno-sse` reports **zero exact/two different**; no new exact match is claimed.
The other methods' absolute data/allocator relocations are recorded in the ABI
report; they are not silently masked by the limited exact-comparison tool.

Logs, code/artifact hashes and preservation manifest are under ignored
`analysis/work/input-owner-batch-20261103`. Temporary fixtures are removed after
verification. No original game/DLL/server/Proton/Windows code was executed; no new
desktop acceptance, staging, packaging or push. The user handles packaging.

Ledger after this batch: **34 complete client,530 partial (525 client/five engine)**
with the denominator18,783 unchanged. Subsequent user direction postpones the
standalone completion pass: bring required producer/lifetime/startup or cursor
work into the [Phase2 delivery milestones](phase2-delivery-plan.md) when needed.
The previous numerical/owner batch is retained as backlog, not the next task.
