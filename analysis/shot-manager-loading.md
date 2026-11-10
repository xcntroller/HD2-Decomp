# ShotMgr cold loading and retirement — source checkpoint, 2026-11-10

Full first P2.2 checkbox remains open. Source-only; application loading disabled,
no first world frame or desktop acceptance. [Combined receipt](checkpoints/p2.2-session-loading-consumers-20261110.json).

`shot_manager_loading.h` supplies the actual cold process state and a world
facade borrowing Startup's module14 registration and SAME00982100 mode. Process
state constructs0044D920's two16-element pools: aligned C0 marks0044DB40 and40
decals0044DB10. Known constructor writes are preserved; undefined bytes are
zero guards and earn no credit. Private cold LIFO ownership provides no runtime
acquisition/projectile/decal producer. Native active vectors remain unproduced;
populated rows reject rather than use invented release/damage/render services.

0044DDE0 queries QPF then QPC on BOTH authority/presentation paths; outputs are
ignored as native. Authority00455490 creates an actual kind9 `humanzones` model,
requires EXACT load status0, otherwise releases/nulls it and returns. Success
finds ten actual markers with maskFFFF, links to scene200 with argument0 and
hides the model.00455370 metadata names c/f/l/m/h/i/s/t/o/p correspond to bones
1/6/13/14/9/10/20/21/16/17.00455520 releases only the model and nulls it; marker
borrowers deliberately remain untouched. No runtime hit-zone collision walker
is claimed.

Presentation sets148/14C=100,150=50 in offline7 else20, zeros trace/shell
borrowers, creates actual `WeaponOdds` and accepts NONNEGATIVE load status.
A negative status retains the model but skips marker lookup. Successful lookup
uses mask1 for trace/shell3rd/shellFPV. It reserves three actual pointer-vector
buffers to100/100/50-or20 without populating records or shrinking prior capacity.
0044E520 then loads x_decal_00..08.tga, x_expl_00.tga, x_blood_00..05.tga with
flags2. Original HRESULTs are ignored: the host owns the actual output pointer,
including a nonnull output on nonzero status. Missing texture output remains
null, with no successful texture substitute.140 resets after these calls.

0044E110's cold retirement preserves authority hit-model release and common
mark-pool cleanup; presentation retires cold decal pool, decal textures in
ascending order, explosion, blood textures ascending, WeaponOdds, then common
mark pool. TraceC0 nulls with model release; shellC8/CC borrowers remain native
stale values until the next activation overwrites them. Unproduced live lists
are guarded, not simulated. Explicit finish can run both passes; facade close
reclaims acquired prefixes, retracts its process borrower and rejects later
activation/finish. Untouched facades preserve the constructor's cold pools.
Startup owns this state across worlds; vector capacity survives retirement.

Focused ASan/UBSan/LSan checks both paths, exact0 versus nonnegative model gates,
clock/marker/16texture order, optional missing outputs, model/texture release
order, ALL40 endpoint exception prefixes, process reuse/capacity and stale/foreign
world rejection. Engine/frame/clock/cache endpoints are doubles. Strict i386
constructs actual Startup state, GameActorServices, original process registration,
both activation/finish passes with the full existing Human/VM graph. No original
or Windows code executed. No projectile simulation or rendering body is supplied.

Evidence in WORK=`analysis/work/mission-world-20261103/`:
`module-load-activation-map.json` vtable0080FA18;
`shot-manager-loading-boundary/{00455490,0044e520}.{c,json}`;
`shot-manager-lifetime-required/0044dc30.{c,json}`;
`shot-manager-cold-pools/` and `shot-manager-pool-owners/`;
`shot-manager-{constructor,retirement}-abi.asm`,
`shot-manager-vtable-writes.asm`, `shot-manager-hit-zone-names.json`.
Read-only export first stopped at guessed non-entry0044DC70, next at absent
0044E110; their logs are retained. Frozen-entry/pool-owner exports completed.
0044DDE0 and0044E110 are absent frozen; the latter also has no Ghidra function,
so static original instructions are primary. Neither gets tally credit.
Constructor-only hit-zone metadata/pool helper forwarding gains no new credit.
Six actual frozen bodies gain partial status:0044D920/0044DB10/0044DB40/
00455490/00455520/0044E520. No complete promotions or whole-body ABI claim.
