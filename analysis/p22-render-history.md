# Strict volumetric history boundary —2026-11-11

The user chose **preserve original behavior**. No first-visible initialization,
zero-history policy, ignored delta, extra randomness or successful replacement
has been introduced. The current source guard is explicitly a reconstruction
safety boundary, not behavior attributed to the original executable.
[Plan](p22-render-plan.md), [receipt](checkpoints/p22-render-history-20261111.json).

## Original path and allocator evidence

Client SHA2568c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.
Static instruction inspection only; no original code or Windows output executed.
Research index located the original constructor, loader, reader, refresh and
allocator variants before new bounded instruction reads. Relevant initial-state
question is now narrowed from an unknown initializer to allocator-residual bytes
on the inspected mission load path; the actual historical bytes remain unknown.

1.00482BB8 pushes48;00482BBA calls ordinary operator_new007E520F.
 00482BC2..00482BD5 tests the returned pointer then calls00481320(0).
 00482BF1 calls00481810 on that same allocation. The direct relative-call scan
 finds the constructor call at00482BD5; export-reported callers agree.
2.007E520F pushes1 then size, calls007E81D9. The1 is new-handler use,
 **not a zero-fill flag**.007E81D9 retries allocation through007E8205 or returns
 null when the new handler is absent/false; an installed handler may throw.
 Existing input-owner-batch.md independently documents this allocator contract.
3.007E8205 dispatches by the CRT heap mode. The ordinary fallback at
 007E82DE..007E82EC rounds size and calls imported HeapAlloc(heap,0,size).
 It does not request or perform a payload clear. IAT0080F130 identifies HeapAlloc.
 This observation does not assume actual runtime heap mode or fresh-page content.
4. The mode3 small-block allocator007F0035 selects/reuses free-list blocks.
 For requested48 its rounded block size is50. At007F02FB..007F0305 it writes
 the allocated block header/footer;007F0336 returns block+4. The user history
 lanes3C/40 therefore lie at block+40/+44, outside both allocator tags
 (block+0/+4C) and the freed-block link words. The successful reuse path does not
 initialize these payload lanes. Fresh or recycled blocks need not have the
 same contents. The private allocator is not reconstructed or credited here.
5.00481320 initializes light, lists/scalars and44=RandFloat*2pi, but never3C/40.
 00481810 has serialized field writes18/1C/20/28/2C/38/34/30 and particle
 records, but no3C/40 input. Serialized chunk ID3C writes **field30**, not3C.
 00480FA0 refresh writes44 from actual world-position length; no3C/40 write.
6.00482260 first reaches those words only after the actual light relation,
 hemisphere and distance gates.00482843 forms ECX=volume+3C;
 0048284E calls wrapped atan2 difference007885B0, then00482853 adds44 before
 00482862/64 finally write the CURRENT direction into3C/40. These stores do
 not establish the prior values consumed by that same first call.

The machine code reads concrete old memory in an actual original process; it
is not a request for a random angle, an implicit(0,0)/(1,0), or a defined first
sample. The recovered source lacks provenance for those original heap bytes.
For example, prior directions(1,0) and(0,1) are both possible finite payloads,
and produce different first deltas for the same current direction. Nonfinite
payloads are also not excluded by this allocation/constructor path. Consequently
field2C==0 does not establish a safe equivalence shortcut: unknown/nonfinite
arithmetic and later field changes cannot be replaced by an assumed finite zero.
Historical Tutorial definitions cannot establish live positive-path state.

Owner18/1C=(1,0) reset is a DIFFERENT contract, established by00482DDB/00482DE2.
It does not initialize per-volume3C/40. RandFloat call order and known44 refresh
remain unchanged. No initialization is copied from an engine camera or fixture.

## Implemented reached-state diagnostics

The actual loader retains an owning diagnostic copy of each submitted light
reference. This is the authored lookup name, not a new engine query or proof of
the resolved frame's current name. It has no native object-field/ledger claim.

Actual00482260 execution now records its input clock bits, the camera position
and old/current horizontal direction, owner phase10, selected volume/index,
distance14, pre-update44, field2C and current direction bits. It reports whether
3C/40 are known without reading unknown memory. Only known prior lanes are
recorded as floats. The reached missing-history event identifies the authored
light before the existing guard throws. FLOAT values are copied as raw32-bit
words, not rounded diagnostic decimals. No extra frame reads or random calls.

GameRenderStages uses the existing DATA observe_data_operation around concrete
pre-scene and debug operations. GameSessionLifecycle wraps render policy and
actual Scene78 separately: policy completion (including config skip) does not
claim that Scene78 or a draw executed. C-callback failures remain latched by
GameScene and rethrown through the SAME exception path. The observer owns copied
values only; no new frame/World callbacks, cache, clock or registry were created.

These opt-in events use the caller's existing resource observation scope. They
do not create a live mission, manually invoke a production observation callback,
or infer original execution from a fixture. Source exceptions and source guards
remain separate from engine failures and actual desktop acceptance. Disabled or
full diagnostic stores cannot turn failure into success or replace its primary
exception, even when a cleanup endpoint also throws.

## Focused verification and remaining work

Native ASan/UBSan/LSan executes actual volume decoding and the reached containing
render guard with explicit engine/procedural doubles. It verifies decoded light
identity/coefficient, exact clock/camera/distance bits, enabled/disabled/saturated
observations, unchanged unknown history, zero particle draws, no extra frame
reads and original primary error when cleanup also fails. It does not supply a
successful initial-history producer or claim actual mission reachability.
Strict actual i386 World/GEffects/Scene/UI/RENDER/lifecycle/DATA composition and
the existing application link pass. Windows outputs were not executed. Audit
sources/binaries are fingerprinted and removed; concise logs/static instructions
remain in ignored WORK. No ledger credit for diagnostics or allocator research.

UI reviewed read-only at19f049b3c28b7d5229acf14db8d54a766a35ac97. Its general
HUD controls have advanced; the RENDER seam is unchanged. The unmerged branch
still identifies actual Human orders, conditional occupied actor/FPV and console
producer dependencies; do not repeat the old all-HUD-missing claim. Source base
remains447e073; observed master83e77da was not imported/reset/modified.

All other independently implementable RENDER services remain source-verified at
c1bbd09. The selected containing checkbox stays OPEN at this exact first-read
boundary. To remove its guard faithfully requires original-state provenance or
an evidenced original producer, not another guessed initializer. The user has
rejected redefining that behavior. Do not repeat allocator/constructor research
without new contrary evidence; preserve strict failure and keep OWN/FRAME/UI/VM
integration and user desktop acceptance separate.
