# Containing world loading owners — 2026-11-10

P2.2 source checkpoint only. The application still declines loading; both plan
checkboxes remain open. No original, DLL, Proton or Windows fixture was executed.
[Verification receipt](checkpoints/p2.2-containing-world-loading-20261110.json).

`host/game_world_loading.h` now constructs actual instances, replacing the strict
fixture's borrowed dependency aggregates. `world_loading::World` owns cold
Scene2 callbacks, one GameWorldScene, the vertex instance registry, the actual
Human/Players/camera/FPV/HUD/audio/script graph and all eighteen module facades.
The existing SessionOwner retains the engine scene, tables, HUD/text/overlay and
application live-squad borrower. Startup retains process animation/cache/input/
SOUND/CRT state, Car definitions/tyres/materials and vertex clock0089B398.
Catalogue and model suffix remain required live caller-owned inputs.

## Construction and identity

Cold Scene2 Human0059BF00 requires process mode before the world dependency
aggregate exists. WorldActorLoadingConsumers can borrow that exact mode word
while its remaining methods reject. WorldLoadingDependencyLink binds the single
completed aggregate only after every target exists; duplicate/foreign binding
and unbound callback paths fail explicitly. The actual containing constructor
checks the same Startup, cold runtime, parent engine scene and parent table owner.
Human services/presentation/factory/consumers do not borrow unconstructed storage.
Failure retracts only this aggregate's bindings. Retire Human/VM/FPV/audio users
before unbinding, and retain cold callbacks through Scene2 destruction.

All graph owners use the same frame/EC registry, Items/model caches, collision
buckets, per-Human lifetime map, Players roster, process pools and CRT. The script
module has its actual source runtime/context ownership and checked frame link;
HUD owns its real panels and same inline pointer context. Mode/local player and
HUD difficulty are read from actual process state rather than constructor copies.
Prepared packet/spawn/camera receivers are actual members of this same graph.
No nonoffline, P2.4, particle, damage or other required positive receiver gains a
successful fallback. P24-F05 remains the persistent application squad contract.

Original004DB08A calls00698500,004DB08F reads game54,004DB0C7 passes that pointer
as ECX into0051D780. This confirms Car navigation uses the FIRST parent dataset;
Human state creation separately receives the second set. Retained instructions:
WORK/navigation-owner-binding.asm. Navigation owns no duplicate records.

Items/model services borrow SessionTimeWord, which rereads actual session.words28
without aliasing a uint32_t reference onto byte storage or caching a second clock.
The old native-word borrower is retained for existing source compositions.
Music reads the parsed C8/CC words live:006CC760 B840/B880 may replace them AFTER
cold graph construction. SessionWorldInput now borrows the actual signed local
player identity and resets that same process word. Car resources use Startup's
same persistent registries/receivers/camera and the scene callback's actual pool.

## Preserve the pre-collision failure tail

006CA780 loads Scene2, actors, sounds and check tables before006EE570. Its
006CADE0 module loading tail also runs on early failure. Constructing the real
module graph only after the tree would make that required failure path impossible.
The SAME bucket owner now exists cold and binds the actual loaded tree's dynamic
grid once. Required bucket queries/insert/move reject an absent grid. Deferred
CollisionSweep rejects even zero-displacement queries before its real tree/grid
identity binding; the existing explicit-null-tree constructor retains its earlier
native branch. No synthetic geometry or no-hit service is introduced.

Scene load binds buckets to the tree's final stable allocation. World binds the
actual Human sweep at that stage. Scene retirement detaches/clears buckets BEFORE
tree destruction, so a cold destructor never dereferences a freed grid. Existing
bucket insertion/move/order/deferred recycling/descriptor cleanup semantics remain.
The cold module graph can therefore execute real +14 receivers before a tree
exists. Actual ordered loader invocation remains the next integration step.

World destructor order keeps module facades before their camera/script/Human
borrowers, those borrowers before vertices/Scene2 and process callbacks last.
The loader still must execute original ascending +14/+1C/+18 and reverse +20
passes, presentation finish/dubbing/authority finish/HUD order, then retire this
source graph before resetting the parent engine scene. Constructors publish no
loaded status. Required meaningful integrity observations remain unresolved.

## Focused validation

Strict i386 compile instantiates the actual cold World, full Owners constructor,
actual script module and all eighteen module dispatch paths. It does not execute
engine operations. Focused native ASan/UBSan/LSan fixtures use explicit process/
math/mesh doubles and actual recovered consumers/grid/sweep/time/music/input:
checked cold mode and live player identity, invalid/duplicate/foreign binding,
retirement/rebind; real bucket migration/recycling/detach/repeat; sweep rejection
and same-grid validation; live time/music changes; signed local-player reset.
Application compile/link and existing27 tracker tests pass. Temporary completed
fixtures/binaries are hashed in the receipt and removed; logs/research remain.
No new frozen-inventory function bodies are credited: totals stay34 complete,
1,949 partial (1,944 client/five engine),18,783 frozen. Continue the SAME full
P2.2 checkbox through ordered loader, required positive receivers/guard
observations, first camera/update/render, failure/repeat and user desktop test.
