# Player requested item drop — 2026-11-17

Source-only H8 continuation after committed OWN1d54083 import. This closes the
positive Drop input requirement documented in the Player sender contract; it
does not close conditional H8.4 or enable application frames.

## Original and same-owner path

Original client SHA2568c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.
Research index queried005A2C50/0053E9D0 before exporting. C/raw in ignored
`analysis/work/mission-world-20261103/h8-player-drop`. C005A2C50 loses arguments;
raw RET0C proves entry/slot/direction stack arguments. Factory005A3FAA installs
Player00811848; actual table+70 (008118B8) is0053E9D0. Its RET8 agrees with
identity/vector, not the decompiler's incomplete declared function signature.

Actual0053BF90 retains fresh hand3 and all existing time/flags/cooldown gates.
005A2C50 makes three SAME engine RNG draws; X/Y scaled draws spill before adding
direction; all three sums spill before multiplying2.5. It reads the fresh B0 and
entry identity after those draws, calls virtual70, then publishes Human178.
The current typed path requires that B0 still be the same Player; callback
retirement/replacement rejects as source safety policy. It reads the canonical
item again after validation. Random/queue exceptions cannot publish178.
No random consumption occurs on the rejected item/cooldown prefix.

0053E9D0 copies and normalizes XYZ with native Z+Y+X square order; it separately
computes original speed using X+Y+Z order, signed FLOAT-bit threshold322BCC77,
near-unit epsilon and sqrt. A zero vector normalizes its Y axis to1 but retains
zero speed. Command10 stores identity8, direction10, speed20 and actor24=-1.
It appends to actual FIRST80/84 command queue, rather than the presentation
second90/94 queue. Existing authority common-command/drop receiver retains
inventory/loose-item effects; pressing Drop does not directly remove inventory.

The source uses existing owning packets/STL queue, not native aligned allocator,
vtable, capacity-growth or allocation-null behavior. Direction fourth lane is
copied by0053E9D0 when supplied;005A2C50 only produces XYZ, so typed scratch
padding is zero and no native fourth-lane provenance/equivalence is claimed.
The original wire/physical consumers use XYZ. Existing Guard drop helper is
unchanged; this checkpoint does not re-audit its prior source claims.

## Verification

Focused native ASan/UBSan/leak checks cover RNG order/values/throwing prefix,
ordinary/zero/unit/tiny-negative velocity, preserved explicit fourth lane,
command identity/actor and unordered NaN speed. World/process/queue dispatch
is not executed by that numeric audit. Actual full World i386 compile/link
instantiates005A3DE0→0053BF90→new World binding→actual Player request method.
The temporary executable is never run. Final build/check receipt below.

Only0053E9D0 gains a frozen partial claim;005A2C50 already has a Guard source
claim. No complete/engine/absent-function credit, desktop or matrix promotion.
[Receipt](checkpoints/p22-h8-player-drop-20261117.json).
