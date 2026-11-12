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
- [x] Inspect latest UIebdcbf5 through Git; full HUD/command/modal consumers have
  advanced. Record its exact unmerged receivers and special-model requirement.

## RENDER-owned implementation

- [x] Complete00482260's original per-volume3C/40 read from actual owned,
  unwritten allocation payload. Preserve allocator dependence, not the exact
  heap bytes of a historical run. Use an original-sized48-byte payload under
  the existing Volume owner, explicit native byte-copy endpoint (no C++
  indeterminate float evaluation), and original current-direction stores.
  No initialized angle, randomness, fixture scalar or first-visible policy.
  Verified first/repeated draw, raw bits including nonfinite, partial allocation,
  endpoint failures, state restoration and retirement; focused native/i386/application checks pass.
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
- [x] Reconcile remaining scope: RENDER-owned implementation source complete;
  external UI/FRAME/OWN/LIFE composition and engine/desktop acceptance remain.

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

Current status: **RENDER implementation scope source complete**. The previous
history stop condition is superseded below. External integration and actual
engine/desktop observation remain open and are not claimed by source audits.
[Completion receipt](checkpoints/p22-render-allocation-20261112.json).

## Ownership correction and current work

The prior stop condition demanded provenance for values from a particular
historical original heap. That exceeds the source-level behavior contract:
00481320 leaves the allocation lanes untouched;00482260 reads the actual current
allocation then stores the current direction. Preserve that allocation-dependent
bug without prescribing its numerical result. This is RENDER-owned work, not
an external P22 dependency. The original static findings remain valid; no
allocator/constructor re-investigation is needed. Native allocation/OS byte-copy
is a source ABI/safety adaptation, not original engine execution or a guarantee
of the same heap trajectory across different processes/toolchains.
