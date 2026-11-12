# Merged UI / RENDER wiring handoff —2026-11-12

Use `/home/control/Documents/Projects/HD2-Decomp-p22-render`, feature/p22-render.
Current implementation base973b4f445240ab92df71f62200a71b25e48d8318 has exactly
mastera820e7018a625dc8a31a314706ab2f4122e0f2d8's tree629d5e206ec501ee0a791f7108bc748e30dfe28c.
User merged UI7a3f7b7 and RENDER345a390, then authorized completing wiring.
No master/other checkout writes, reset, merge or push. Do not follow inherited
HUM/master handoff as this worktree's task. No original/Windows execution.

[Plan](analysis/p22-render-wiring-plan.md),
[implementation](analysis/p22-render-wiring-implementation.md),
[receipt](analysis/checkpoints/p22-render-wiring-20261112.json).

**Merged UI/render and available application render wiring is complete.**
-004B5E40 modal cleanup now reaches actual same-World GEffects through UI's
 bound GameRenderStages; Human/order source no longer implements render cleanup.
 Its genuine gameplay/camera/Sight/realtime operations remain required.
- Actual GameUiSceneCallbacks binds the UI render loan before engine publication,
 rejects wrong World/duplicates and retires via callback token. Old destruction
 cannot retract a replacement; stale dispatch rejects before touching resources.
 Adapter source is in game_ui_scene_callbacks.h, instantiated with actual UI.
- Existing lifecycle owns UI/render/callback; engine stop/unbind precedes callback
 loan retirement, then render and World/resource destruction. Lifecycle Scene
 entry also verifies its exact owned dispatcher is still installed.
- GameApplicationState::render now uses original0069B870 mode/config170 policy
 and actual bound Scene78. Scene checks registered/active/borrowed presentation;
 missing/suspended dispatch fails. session_owner() exposes SAME existing parent
 for OWN composition with its backend and status_storage alias, not a new owner.
 No simulation or graphics-present work moved into render. DATA policy/Scene
 observations remain separate; actual RET00724CB0 remains a no-op.

Native ASan/UBSan/LSan PASS100 loan/callback/stage/special-effect families and
100 actual GameScene registration/rollback/replacement/admission/Scene78/policy
families. UI/World/engine/platform endpoints are labelled doubles. Actual host UI/
render/application/lifecycle compile+link i386 and existing application build PASS;
Windows outputs never run. Defined throwing gameplay modal double has NO render
cleanup method; actual stages supply it. Temporary audits/stubs fingerprinted/
removed. Ledger evidence links only, zero new credit/promotions/count changes.
Full tracker retains unrelated missing legacy0062D880 WORK evidence limitation.

**Exact next dependencies; no successful activation was enabled:**
1. FRAMEc36bba8 is NOT merged into this base; game_session_frames.h is absent.
   Import that owning branch before input/fixed/update integration; do not copy
   another coordinator. Preserve frame_retirement_started_/retire_frames BEFORE
   this Scene/callback/render retirement when reconciling lifecycle changes.
2. OWN installs/owns ONE actual lifecycle and activation state under the application
   (using session_owner(), parent.backend(), status_storage()), completes genuine
   loading/activation and controller/cutscene services, then forwards updates/input.
   Application.initialize still declines; input/fixed_update/update still reject.
3. FRAME requires mutable original64-word input views; current ApplicationState
   interface uses const sampled views. Resolve writable controller provenance;
   do not cast arbitrary read-only storage merely to satisfy types.
4. Real Human order/path/presentation/selection/Sight/realtime/occupied and PACK
   producers remain typed UI/Runtime requirements. Interface presence does not
   prove all callees are missing or that every branch is reached in Tutorial.
5. Actual loaded populations, engine draw/present and user mission/failure/return/
   repeat acceptance remain unobserved. No desktop or mission-reachability claim.

RENDER-owned source completion and actual allocation-dependent history from
345a390 remain preserved. Further full-loop wiring follows the owning FRAME/OWN
imports/contracts; no remaining merged UI/render cleanup seam is left open.
