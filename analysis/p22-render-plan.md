# P22-RENDER implementation plan

Fetched/actual implementation base:447e073ed90f9fc6fb0fdd305fe6d725115d1786.
Existing feature/p22-render worktree preserved. No reset, master advance, merge or
push. Subsequent master/UI advances were inspected read-only, not imported.
Source-only; application mission loading remains disabled.

## Existing rendering to preserve

- [x] Preserve session0069B870 mode/config policy,006C9260 stages, actual Scene78,
  existing volume leaves, fades, text/font/overlay, graphics/camera and lifetimes.
  Absent0069B870/00486BF0 and actual RET00724CB0 receive no invented credit.
- [x] Inspect research/p22 through Git; verify source/evidence and SAME C4 volume
  owner versus224 clock. Scene15C render camera differs from Sight's128.
- [x] Inspect latest UI74220d4 through Git; full HUD/command/modal consumers have
  advanced. Record its exact unmerged receivers and special-model requirement.

## RENDER-owned implementation

- [ ] **BLOCKED:** complete00482260's original per-volume3C/40 first read. Actual
  allocator-residual bytes have no provenance; constructor/read/refresh do not
  initialize them. User requires original behavior: no invented first sample.
  All containing camera/visibility/hemisphere/distance/history-update/order/draw/
  restoration source around this read is implemented. Keep this whole item OPEN.
- [x] Bind00486BF0 over SAME GEffects C4/224 and actual frame/graphics endpoints.
- [x] Implement positive00482E50 and00483E50/00483DE0 process resources, geometry,
  graphics calls, absence/failure gates, ordered retirement and retry.
- [x] Implement concrete GameUiRenderStages, actual Scene adapter, lifecycle-owned
  callback installation/retirement and positive0079B760/0079AC40 debug geometry.
- [x] Implement actual CSC_RunCLouder opcodeAD producer through existing compiler/
  context/runtime and SAME module→effects façade→Startup process loan. Retire
  callback and cold-configured resources before closing the owning module.
- [x] Implement GEffects44/004870A0 special2E0 enable/deferred/immediate restore,
 004871A0 timed producer,00487240 flicker/expiry and004888C0 retirement. Bind actual
  opcodeBB THUNDERSTORM_SetOn and SAME effects/frame/random owners. The current
  UI modal interface can call this concrete service after its owning integration.
- [x] Correct native Scene exception guards: failed registration retry, failed
  re-registration retaining its borrower, latched callback primary over later
  endpoint failure. Native guards remain distinct from original engine behavior.
- [x] Preserve DATA APIs and four evidence levels; record actual resource/camera/
  volume/history/procedural/special-frame operations without added engine reads
  or randomness. Empty snapshots are not branch-inactivity proof.
- [x] Native ASan/UBSan/LSan: positive geometry/resources, actual module/VM loans,
  special-frame behavior, Scene gates/order/failures/retirement. Strict concrete
  i386 World construction/render/VM/callback link and application link. Fingerprint
  and remove temporary sources/binaries; retain logs and checkpoint receipts.
- [x] Reconcile remaining scope: no other independently actionable RENDER item
  found in the inspected required call graph. RENDER remains source-INCOMPLETE
  only at the confirmed residual-history boundary above; do not relabel it UI.

## External integration and later acceptance

- [ ] UI owns importing/binding its unmerged GameUiPresentation consumers.
  Base command_before_scene rejects004B6720; latest UI implements it. Current
  GameUiCommandModalSource::retire_command_special_model(World&) must forward
  to world.scene().effects().configure_special_model(false,nullptr,true).
  Human/Player/camera/Sight/input/console sources remain their actual owners.
- [ ] FRAME supplies original render-before-update scheduling, camera updates and
  effects/module update order. Timed lightning's live weather/network caller is
  outside this RENDER receiver implementation; no positive caller is fabricated.
- [ ] OWN/LIFE install/activate the callable lifecycle and preserve borrower
  retirement before Scene/World/camera/texture teardown. RENDER does not activate.
- [ ] Observe real selected resources, loaded populations and positive branches
  in the authorized engine workflow; user-operated desktop acceptance is separate.

[Implementation/evidence](p22-render-implementation.md),
[strict history proof](p22-render-history.md),
[latest checkpoint](checkpoints/p22-render-bindings-20261111.json).

Current stop condition: independently implementable RENDER work is done; further
RENDER closure is genuinely blocked by missing original-state provenance for the
first3C/40 read. Do not rerun the confirmed allocator investigation without new
evidence or relax the user's original-behavior decision. Continue P22-FRAME in
its separately authorized worktree after this verified checkpoint.
