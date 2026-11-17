# H8.4 Dog Body order storage — 2026-11-17

The actual Dog actor now owns one current314 record and five338 records through
DogBodyOrders. Existing330/334/records names alias that SAME storage. There is
no private mirror queue, default live Dog, new process owner or activation flag.
This supplies the order dependency for the unfinished live Body/controller;
H8.4 and H8.2 remain open. Dog activation is still rejected at its existing gate.

Original image/evidence provenance is in [Dog source](human-h8-dog-source.md).
Research-index lookup preceded use of existing C/typed-v2 variants and new static
raw inspection. WORK/human-h8-dog-body-order-queue.asm proves005C4B90/005C4C30/
005C4C70.005D4C70 initializes only record1C/20 (absent frozen, no new credit).
005C7D30 resets current/queued cursors, current action23/posture0/stance0 and
head/tail-1. Cold construction does not produce head/tail, so order operations
require that explicit producer rather than accepting a fake empty queue.

005C4B90 advances the five-slot tail and overwrites the oldest record when full;
it resets only1C/20, retaining prior commands/metadata. Inlined005C5030 append
uses cursor3, lower bound4, writes at most four commands as the bound decreases.
Consumption visits cursor downward.005C4C30 advances head or marks the queue
empty, retaining the old tail.005C85A0/005C8600/005C8660 read the LAST queued
record metadata, otherwise the current record. Copy-to-current copies all nine
native words before front removal; source metadata provenance accompanies them.

Raw005C4CA2 proves005C4C70 writes HEAD35C=4 when TAIL360 becomes negative; it does
not wrap the tail. This surprising native store is preserved. A subsequent
native-invalid metadata index fails explicitly; the source does not silently
repair it. Bounded clear detects a native nonterminating invalid head/tail state.

Unwritten native10/14/18 metadata is not made successful by the typed cold zero
padding. The source tracks whether a real producer has supplied it. Cursor reset
preserves both prior metadata and its provenance.005C5030's eventual metadata
publication has an explicit API. The live transition routine must still resolve
its original ordering/early reads; no generic successful defaults were added.

Additional retained raw evidence pins multiple bases:005D5290 passes Actor+90
to005C8370; the latter invokes clip initialization005C6E10 with Body+10, then
Body005C7D30 with the original Body base. Thus clip model+10 is Body20/ActorB0,
not ActorA0's clip vtable. Existing C exports do not express this adjustment.
WORK/human-h8-dog-body-base-adjustment.asm is authoritative. Native Body
priorities278/27C/280 and newly enqueued metadata have early reads that still
need exact producer/order reconciliation before live activation. Do not assume
zero from the source's padding or extend D11's four authorized defaults to them.
The focused constructor/init raw, factory005D4A60 and existing controller bind
records are retained; this is a concise unresolved risk, not proof of complete
alias/write provenance. No new activation compatibility policy is adopted here.

Verification:175556 strict native ASan/UBSan/leak checks pass, including an
independent deque model across50000 operations, all five wrap slots, overwrite,
copy/consume/capacity, native head-wrap behavior and missing producer/metadata/
invalid index failures. The actual queue/source storage is exercised; this does
not run Dog Body/engine/activation. Strict actual full module+Dog owned storage
i386 link and forced launcher/probe rebuild are recorded in the receipt. No
Windows artifact or original program is executed. Six frozen bodies receive
partial source credit only; no absent initializer, inlined append or engine
credit. Existing H8.3 closure and HUM matrix state remain unchanged.
