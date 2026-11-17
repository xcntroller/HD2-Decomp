# H8 process orders, collision markers and Active list — 2026-11-12

H8.2 and full H8 remain OPEN. This source checkpoint extends the
[order/action and HUD model checkpoint](human-h8-order-action-source.md) through
the actual process order owner, registered marker lifecycle and Active250 list.
[Receipt](checkpoints/p2.2-human-h8-order-owner-20261112.json), [plan](p22-hum-h8-plan.md).
No original/Windows/application execution, desktop claim or push.

## Same-owner production and retirement

0058A3F0/005825C0 use DISTINCT Startup008AB048 manager+18, native kind4 order
records; these are not current-action4 or action1 records. The shared process
pool eagerly allocates16, grows by mutable batch, takes LIFO free records and
outlives all Human order leases. Issuer8, target40 and marker84 are separate
borrowers/owners, never host pointers in scalar DWORDs. Constructor growth80=8,
known scalar payload stores, xyz100000 scratch and48/49/4A resets are recovered;
unknown W lanes/native allocator/vtable layout are unclaimed.

0058C5A0 uses SAME HUD command template/model pool. After004B9240 returns,
marker84 is published BEFORE EC is set to the actual order, position8, link38
and update24. Link uses SAME World root **200, mode1**; camera root1F8 would be
wrong. The actual dynamic descriptor has flags40, mask8000, model, order context,
bounds(-.2,0,-.2)/(.2,1.6,.2), radius1.6 and order position. First world-matrix
read is a live borrower; a SECOND fresh read is copied/inverted. Registration
006EF3E0 into SAME scene buckets is LAST. No duplicate World/grid/model owner.

0058D6C0/0058C8E0/00582640 retire buckets BEFORE EC clear/model hide+return,
then release native74's route allocation BEFORE process free-list publication.
Marker84 remains observable during cleanup callbacks until retirement finishes.
Typed byte ownership reserves the route allocation's lifetime only; its positive
layout/producer is still required. Reconstruction exceptions attempt all marker
cleanup and report through the existing frame cleanup-failure channel; failed
model return releases its checked-out owner. Native unsafe allocator/exception
ABI is excluded. Missing template or grid is a required failure, not a marker
placeholder. Marker presentation update calls004B9B10 without moving position.

## Actual Active250 sequence

The actual Active state now owns one typed pending sequence. Insert0052ED00/
0052EB20 first tests count==10 then original target/raw-position navigation
0052ECA0. Rejection returns the incoming process record BEFORE speech68/1/0/1.
A trailing kind2 is returned directly; otherwise a trailing action32 restores
formation then returns. Insertion-after decrements its index on this replacement;
append uses the end. Serial1BC=game2C+1 follows publication. Unfrozen insert/
getter/navigation-wrapper starts receive no separate function credit.

0052EE60 first merges into the selected stored order through0058D720. Lengths
use (X²+Y²)+Z² and **unordered also copies**, as verified in raw x87. Nonzero
flags replace before marker refresh. For index0,18E=1 and REQUEST kind2 copies
REQUEST20 to Active160 and raw B010/B020 before direction setters. This is
separate from the stored order: a zero direction request can leave stored20
unchanged while publishing raw zero. Serial is last, preserving failure prefixes.

0052EFA0 writes serial BEFORE index0 query2000/path stop/202/174|1/206, selected
formation restoration for action32 and kind4's hold suppression. Process return
precedes stable list erase. Slot borrowers remain inspectable during pool/engine
callbacks even when the owning lease is retiring.0052EF20 iterates original
count backwards, then its existing final serial/query/path/202/206/18D stores;
positive hold can deliberately leave a new kind2. Normal close runs this before
clearing Human44. Reverse RAII lease retirement is an additional rollback guard.

00531F60 reads the retained FIRST pointer slot even after an erase; its scalar
record still belongs to the process pool. Source retains that typed borrower,
uses its flags/action before reacquisition (including same-record LIFO reuse),
then appends an actual kind2 marker/order. It does not substitute current B050
for that native retained storage. A cold vector with no prior first slot remains
an explicit guard; the unsafe cold/null-storage branch is not credited.0052F110
compares frame<=serial unsigned, including wrap.0052F0E0 uses selected Player28,
not current2C, then existing005A91A0 formation owner. Actual i386 instantiation
caught derived-scene template deduction creating a different formation type;
the call now explicitly uses the already-owned ActorFrames base type.

## Focused verification and remaining boundary

Native ASan/UBSan/LSan PASS84400/100 lifetimes uses actual order pool, marker,
descriptor/buckets, Active sequence algorithms and AI resources/direction. Human/
AI container/World/HUD/engine endpoints are explicit doubles. Covers publication
visibility, two matrix reads, register-last and removal-before-EC/model return,
growth/reuse, retained scratch/route release, missing template/grid, position/
link/matrix failure, cleanup failure channel, append/index/trailing replacement,
rejection-before-speech, count10 short circuit, request/stored direction split,
NaN merge, serial/failure prefix, retained first slot, hold suppression, reverse
reset including positive hold, selected formation dispatch and full retirement.
The previous actual HUD-marker owner audit remains valid; no unchanged H1–H7
kernel audit was repeated.

Strict actual i386 FULL LINK instantiates process order creation/update/return,
Active insertion/update/action/retire/hold/reset and entire World Human frame,
including real HUD, buckets, selected-Player and formation types. Launcher/probe
rebuild passes. Windows fixtures are not executed. Temporary audits fingerprinted/
removed; logs/static evidence retained. Twelve new frozen partial claims; no
complete promotions or delegated-engine credit. STL/RAII/layout, cold undefined
inputs, finite direction constraints and positive route layout remain limits.

NEXT:005282E0 pending processing/00529F60 setup,00534340 action-state command,
00528950 decisions,00536120 follow and the remaining00526550 authority chain.
Actual HUD/command caller and Human190 ownership/reference producers must be
reconciled when selected; raw190 remains guarded. Native route74 producer/frame,
positive actor lifetimes, mandatory Player/Script/control/hazard/water, async
ShotMgr and Car/OWN/FRAME/application/render/desktop remain required. Successful
order ownership does not permit an idle-success Active authority frame.

WORK=`analysis/work/mission-world-20261103`: original lifecycle/proxy C, order
initialize/start/list/update/merge/query raw ABI dumps, original image SHA
8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307. Index receipts
precede exports. Insert/query wrapper recovery uses raw bytes; no Ghidra types or
original assets changed. The earlier deletion/recovery limits remain documented
in the previous checkpoint and local recovery manifest; this checkpoint's logs
are fresh.
