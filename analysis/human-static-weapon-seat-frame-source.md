# StaticWeapon mounted Player reference frame — 2026-11-17

H8 source-only continuation. Player0053BF90's actual flags10 branch now uses
StaticWeapon/type21 Actor90 (00604B70) through the SAME World seat presentation
facade, loaded seat vector, cached occupant, and Scene frame owner. Car remains
on its existing004D9E10 receiver. Conditional20/34 still require actual owners.

## Evidence and behavior

Research index queried00604B70/0059C390 before new exports. Original client
SHA2568c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.
Read-only C/raw retained under ignored WORK/h8-static-seat-frame; native
StaticWeapon00813130+90 resolves00604B70. Raw352-byte body supplies arguments
where C loses the fallback seat index. Signed negative/out-of-range seats return
NULL before any occupant/frame/view query.

Native caches the initial seat occupant in EBP. If present, it unlinks that
occupant's model to NULL, queries actual0069A4B0 (FPV58 OR59), and only if false
queries ActorCC/00604E30 (authored NO3RD). It looks for cameraFPV* or camera3RD*
with10020 on the current seat marker. A NULL pick repeats BOTH view/CC gates,
then searches the current marker's parent. It does not cache mode/name/marker
across that fallback. Even a NULL final pick relinks the original cached occupant
to the current marker and updates its model before return. Its model is re-read
following the link callback. This differs from Car's fresh-occupant relookup.

The helper borrows the existing seat vector and callbacks. It adds no camera,
World, marker, roster or selection owner. ActorSeatPresentation's typed Actor90
method is mandatory; missing20/34 families retain the existing explicit family
requirement. It installs no successful substitute for them. Invalid required
marker/parent fails as source safety policy; RAII attempts relink during exceptions
and records a secondary cleanup failure without masking the primary exception.
Native allocator/vtable/SEH equivalence is not claimed.

## Verification

Native18 ASan/UBSan/leak assertions cover signed bounds with no callbacks,
view/CC short circuit, direct/fallback query names and order, changed view and
marker, replaced seat occupant but cached original actor, fresh model after link,
lookup exception restoration and absent occupant. Seats/actor owner identities
are real test records; engine methods are explicit doubles. Actual World i386
compile/link instantiates the full Player input path and Actor90 facade on actual
StaticWeapon loading. Windows/original outputs are never executed.
[Receipt](checkpoints/p22-h8-static-seat-frame-20261117.json).

Only frozen00604B70 earns one partial claim. No engine/absent-function/complete
credit or matrix/HUM source-complete claim. H8.4–.6 remain open.
