# Merged UI / RENDER wiring —2026-11-12

Current isolated worktree/branch: HD2-Decomp-p22-render / feature/p22-render.
Implementation base973b4f445240ab92df71f62200a71b25e48d8318 has EXACTLY the tree
629d5e206ec501ee0a791f7108bc748e30dfe28c of merged master
a820e7018a625dc8a31a314706ab2f4122e0f2d8. Status/refs/history/worktrees inspected
before changes. No master/other checkout edits, reset, merge or push. Inherited
HANDOFF addresses HUM/master; current user authorizes this isolated wiring scope.

## Selected full boundary

- [x] Inspect merged UI7a3f7b7, RENDER345a390 and LIFE/actual application source,
  original006C9260 stage/004B5E40 modal/004870A0 effect and retirement contracts.
- [x] Wire command-modal render-owned cleanup directly to SAME World GEffects
  through actual render stages. Remove that operation from required Human/order
  sources; keep all their genuine gameplay/selection/Sight/realtime requirements.
- [x] Bind UI render-service loan during actual Scene callback construction,
  before engine publication; reject foreign World/duplicate/null binding. Preserve
  rollback, callback replacement and token-safe unbinding; retire loan after
  engine stops callbacks and before render/World destruction. Route stage1/debug
  through the same bound receiver. No new owner registry or rendering pipeline.
- [x] Wire actual GameApplicationState.render to existing0069B870 policy and
  SAME Scene78 when a presentation dispatcher is bound; expose its existing
  constructed SessionOwner for OWN composition. No second lifecycle, loaded flag,
  activation success or render/update scheduling ownership is introduced.
- [x] Verify actual merged HUD/Scene stages and modal-to-effects composition,
  failed registration/retry, retirement and cross-World guards with focused native
  ASan/UBSan/LSan (doubles explicitly labelled). Strict actual i386 World/loading/
  UI/render/callback/lifecycle link and application build, sequential heavy work.
- [x] Record exact remaining imports/OWN/HUM/PACK interfaces and verification
  levels; fingerprint/remove temporary audits, update receipt/HANDOFF/plan and
  commit the verified whole wiring checkpoint.

## Available and external boundaries

- Merged source already wires lifecycle.render→policy→actual Scene78→
  GameUiSceneCallbacks→UI HUD/text/fade/overlay and actual pre-scene/debug stages.
  Stage2 command geometry now uses merged positive004B6720, not an old throw.
- Modal cleanup is now bound through the same-World render loan; the old
  gameplay-source render method is removed. Required gameplay actions stay explicit.
- FRAMEc36bba8 is NOT merged into this base. Its GameSessionFrames/lifecycle
  input/fixed/update coordinator APIs are absent. Inspect read-only; do not copy
  a competing coordinator or assume that branch's implementations are available.
- GameApplicationState render now forwards the original policy/actual bound Scene.
  Initialization still declines activation and input/fixed/update are not wired.
  OWN loading/install/activation and FRAME services stay explicit; no successful
  loading is enabled just to produce an image.
- Actual Human orders/path/presentation/Sight/realtime, other gameplay/PACK
  producers stay required. Real resources/populations, engine execution/present
  and user desktop acceptance remain separate. No original/Windows execution.

## Final result

[Implementation](p22-render-wiring-implementation.md) and
[receipt](checkpoints/p22-render-wiring-20261112.json) record completed merged
UI/render/modal and application-render wiring. Selected available integration is
complete; actual loading/activation and input/update forwarding remain OWN/FRAME.
FRAMEc36bba8 is still absent, not silently replaced. Original layout/engine ABI,
actual mission runtime/present and user desktop acceptance are separate.
