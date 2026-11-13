# H8 Active order actions and actual HUD marker owner — 2026-11-12

H8.2/full H8 remain OPEN. This is a source dependency checkpoint, with no new
application activation, original/Windows execution, desktop claim or push.
[H8 plan](p22-hum-h8-plan.md), [receipt](checkpoints/p2.2-human-h8-order-action-20261112.json).

## Current-action ownership and transition

Original0052A1E0 resets current action via actual virtual10C00575820 BEFORE
clearing Active174 bit2. Order kind1 resolves target40's model position or raw
order30 and acquires action7 through00582AD0. Driver seat0 of a Car binds SAME
AI130 vehicle path, scans orders starting at1 for the next kind1 and resolves
NEXT target first, then freshly resolves CURRENT target. It submits0055E1D0
with speed1; without lookahead it freshly resolves CURRENT again and submits
existing0055E160. There is no cached target-position substitute. Other seats
and on-foot states publish the action without starting a vehicle path.

Order kind2 acquires action **12**, through00582920 and manager+168/0058B2C0;
it is NOT action4. Order kind4 acquires action8 through00582B50, manager+108/
0058ADE0. Action7 is manager+F0/0058AC90. Three actual process pools eagerly
grow16 aligned records, initialize native kind4 and mode/age/lifetime8/C/10,
retain non-written words on reuse, and return leases to their distinct pools.
Native8+14/12+30 actor borrowers live outside scalar DWORD words. Source size/
STL/deleter/object layout is not native ABI equivalence. Other native pool
families remain outside this checkpoint. SAME Startup pool aggregate now reaches
Active state/factory; no per-Human replacement pool is created. Actual resources
F0 variant owns/ages/retires these new leases. Kind8 acquisition clears18/1C;
kind7/12 acquisition leaves mode/age/lifetime intact.

0055E1D0 preserves signed finite speed, full destination and next-point vector;
strict fabs(speed)>DOUBLE0080FAC8 selects clear4/set1 and publication. Otherwise
it only sets stop4. NaN also stops. Original constant bytes000000E08E79453E
are9.99999993922529e-9. Raw x87 and caller order are retained in WORK.

The transition is exposed by the actual Active state but takes an in-call
pending-order owner sequence; it DOES NOT manufacture that sequence. Native
order manager+18 is separate from current actions and still needs its typed
producer, collision/marker lifecycle and command receivers. Populated raw250/
254/258 remains guarded, and authority00526550 remains required. Successful
kernel verification does not close005282E0/00529F60 or the full Active frame.

## Marker creation, update and return on existing HUD owner

Original004B9240/004B9B10/004B9080 now use the actual command panel's model748
and free-model750 vector, existing first-player-state20/action39 artwork and ten
button textures. No extra model pool, HUD or scene owner is introduced. Missing
748 returns NULL before flags/lookup; callers requiring a marker must still
reject its absence. Acquire takes the last free model or creates/copies kind9
through actual frame4C. Return hides before transfer back; close releases the
same pool. Failed reconstruction endpoints release the checked-out model by
RAII instead of leaking it; native unsafe exception/allocator ABI is excluded.

Fresh normal visuals3Dcommand/pose/move/stand receive A0, first-group18, flags
08000000, repeated fresh group/material lookups, white diffuse/zero other
channels/power and alpha0.6. selector_commands uses red2/0/0, without normal
A0/group18. Reuse skips these material preparations. Kind-based command texture
selection, per-mask defaults4/4000/200 and priority are preserved. Creation's
movement selection deliberately retains pose texture when ONLY8000 is set;
update starts with NULL and sets NULL for that case. Update checks command/pose
nonnull texture before replacing, while movement/stand always replace. Creation
updates selector then hides/updates model; update advances each of the four
visuals before the next lookup, then hides/updates model.

These methods are present on SAME HUD command owner and actual-host instantiated.
Order0058C5A0 still needs to own the checked-out model, set actual EC order data,
link to World root, initialize/register SAME dynamic descriptor and retire it
BEFORE clearing EC/returning model. No collision-visible order producer has been
enabled by these methods. Preserve the descriptor removal→EC clear→model hide/
return→route allocation release→process free-list order in0058D6C0/0058C8E0/
00582640 when completing that next boundary. Existing HudScene close must not
precede retirement of live order loans.

## Verification and evidence

Native ASan/UBSan/LSan action PASS5900/100 lifetimes uses actual process pools,
resources and path with explicit Human/target/engine/World doubles. Tests distinct
kind12/7/8 identity, retained/reinitialized fields, growth/reuse, action retirement,
three fresh driver matrix reads in native order, no-lookahead sentinel lanes,
real governor callback, flags/signed speed/NaN and failure/publication prefixes.
Marker PASS23500/100 lifetimes exercises actual HudCommandLoading and helper,
including bind/acquire/fresh/reuse/update/return/close/missing template and partial
failure cleanup. Engine, HUD text, Factory and camera are explicit doubles.
Temporary marker audit suppresses ONLY pre-existing GCC misleading-indentation
warnings while including unchanged HUD/control headers; new helper is included
under full warnings-as-errors first. No unrelated formatting edits were made.

Strict actual nested-host i386 FULL LINK instantiates the whole Human frame,
Scene2 bridge with EMPTY outer Runtime, actual Active transition and SAME actual
World HUD marker acquisition/update/return. Endpoint Order records/sequences are
explicit fixture inputs; native order production remains absent. Launcher/probe
rebuild passes. No Windows fixtures executed. Temporary audit sources/binaries
are fingerprinted and removed; logs/static evidence retained. Eleven frozen
entries gain partial claims; no complete promotions or delegated-engine credit.

WORK=`analysis/work/mission-world-20261103`: `human-h8-active-task-workers/0052a1e0.c`,
`human-h8-order-lifecycle/{00582920,00582ad0,00582b50}.c`,
`human-h8-order-proxy/{0058b2c0,0058ac90,0058ade0,004b9240,004b9b10}.c`,
`human-h8-order-action-boundary/0055e1d0.c`, `human-h8-order-marker-return/004b9080.c`,
`human-h8-{order-start,order-marker,vehicle-order-request}.asm`. Research-index
receipts precede exports. Shared original SHA8c52ce66...f10b307 is unchanged.

The accidental worktree deletion lost some ignored H6/H8 audit logs. Committed
source/receipts survived at31c436e; recovery independently copied the surviving
HUM research cache, restored shared tool links and regenerated original H8
exports from unchanged read-only Ghidra. Old receipt verification is historical;
regeneration is not a rerun of those audits. Local recovery manifest records the
exact restored/regenerated paths and limits. This checkpoint has fresh logs.
