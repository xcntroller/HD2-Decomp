# Shared Human projection frame process — 2026-11-10

Source continuation of the FULL first P2.2 checkbox. Application still declines
loading. No mission frame/presentation/desktop acceptance is established.
[Receipt](checkpoints/p2.2-human-projection-process-20261110.json).

## Actual process and Items frame receiver

007DD390/007DD3B0 call0078AD70: this is the shared Human projected-model process,
not inventory iteration or mesh fragments. It uses SAME primary0098AD3C records
as0078B180 visibility, secondary0098AD2C fading records and0098AD48 payload pool.
Startup now owns these lists,400 per-Human counters and process clocks. The
World's HumanPresentationShared borrows that process with a checked single-World
link; its former private projection list/unused400-word copy are removed.

Authority invokes the update only with mode1. Presentation returns with mode1;
otherwise0098B874 accumulates elapsed and, when SIGNED>=100, updates exactly100
once and subtracts100 AFTER the callback succeeds. All clocks use DWORD wrapping.
0078AD70 returns before any clock/Scene access without mode2. With mode2, it
reads actual session28 as a DWORD (not FLOAT) and Scene11C every invocation,
including empty queues. Previous scene epoch is0 when Scene11C<2, else epoch-1.

0098AD64 adds elapsed and tests strictly SIGNED>1400. The reached branch consumes
one SAME CRT rand()%200 and subtracts1400+remainder once, EVEN WITH EMPTY LISTS.
Then it refreshes0098AD60 from selected-profile quality byte2, maskedFF. It walks
primary records forward; when the unsigned per-Human count exceeds0084F5A4,
it decrements that counter and expires the row. Bounded source checks reject
indices outside the original400-word table. No count saturation is invented.

Independently, nonzero0084F5A8 expires the oldest primary prefix while
record.time+lifetime_seconds*1000<=actual session28, using unsigned wrapping
comparison. It does not search later records after an unexpired head. Both
expiry paths compare actual frame11C unsigned against previous scene epoch.
An old frame unlinks38(NULL,0), Scene84 removes it, modeB8(3) runs and the payload
returns to the free pool retaining its frame reference. Periodic expiry removes
the primary row BEFORE mode3; timestamp expiry calls mode3 while the row remains
published. Typed list ownership preserves these callback observations, while
allocator/node ABI is excluded. Visible/recent frames move to the END of the
secondary queue with record.time reset0; per-Human count has already decremented.

0098AD68 independently adds elapsed and tests SIGNED>=300, subtracting exactly300
once. Each secondary row advances time by300, regardless of elapsed overshoot.
At unsigned time>=3000 it detaches/removes, unlinks the row, calls mode3 and pools
the retained frame. Otherwise FLOAT alpha is1-time*FLOAT.00033333332976326346.
Only a strictly lower alpha updates; NaN/previously lower alpha is left alone.
At time<=300 the native mode0/blend5 calls precede alpha+A8 and frame24. Alpha
reads actual visual1E8, distinct from GEffects' other opacity fields.

## First/last user and final shutdown

0078A4F0 publishes enabled1, per-Human limit40 and lifetime40 seconds BEFORE
texture acquisition; the500 scalar remains separate. First/last user operations
do NOT reset0098AD64/68 or0098B874.0078A5F0 visits primary then secondary forward,
detaches/removes and RELEASES each frame reference before pooling the empty
payload; only then resets400 counters and retires shared textures. World failure
cleanup uses the same lifetime before Scene retirement. A second World cannot
clear the first World's process while attempting construction.

Native update returns payloads LIFO without releasing their detached frame
references.0078A980 at actual Items3 startup-module shutdown releases those frame
references before payloads; the source process pool now occupies that boundary,
before later module shutdown. Active World/rows must already be retired. The
original free-node allocator lists are represented by typed STL allocation,
not credited as recovered allocator bodies.

Footprint/projected-frame production0078AA10 and upstream0075D210 remain required;
there is no invented append API.0078B240 save/0078B7F0 restore are research only.
Actual production queues therefore remain cold, but their original frame clocks,
RNG and preference side effects now run through module3. The positive expiry,
fade and pooling policy is implemented and tested using populated fixture
records, with engine leaves as doubles. It is not a claim that actual gameplay
already produces projections.

## Verification and evidence

Temporary native ASan/UBSan/leak checks cover mode/clock gates, strict1400/300/100
boundaries, RNG/profile order on empty lists, per-Human limit retirement, primary
row publication at mode3, retained pool references/order, unsigned epoch/time
wrap, oldest-prefix expiry, single fade steps/overshoot, lower-alpha/NaN behavior,
presentation failure/retry and actual private process timer persistence across
World facades. Strict i386 instantiates actual World/Players/module3/visibility/
retirement and both18-module passes; other simulation leaves remain declarations.
Application link and tracker checks are separate. No Windows code was run.

Evidence under ignored WORK: `frame-projection-update.asm`,
`frame-projection-process-address-refs.json`, `frame-item-ticks.asm`,
`frame-item-actor-consumers/0078ad70.{c,json}`,
`frame-projection-process-dependencies/` pool/list/CRT exports,
`frame-projection-producers/` research-only producer/save/restore exports and
previous0078A4F0/0078A5F0/0078A980 exports. One new frozen partial0078AD70; no
complete promotions or credit for absent007DD390/007DD3B0.

The actual containing World constructor now emits the bounded source diagnostic
`P2.2 event=world_constructed result=success loaded14=<0|1>` AFTER its owners and
packet link exist. loaded14 is read from the actual session, not forced0. No
attempt ID is fabricated before application integration. This marker establishes
construction only; diagnostics parser v1 does NOT yet recognize it. The exact
format is recorded in tools/runtime_diagnostics/INSTRUMENTATION.md for adoption.
