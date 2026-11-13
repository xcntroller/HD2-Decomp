# Updated implementation order

Current source review2026-11-13: PACK4fccf12 merged into masterfa94dee; [current verification](evidence/pack-master-verification-20261113.json). The prior overnight source review was799a793 against841d89a. The shared movement solver and its detours are complete for the recovered source boundary. Guard/Area, Player and Script frame dispatch, base timer queries and the actual module facade have also been supplied. [Blocker matrix](blocker-matrix.md)

## A. First truthful loading attempt

1. **Use completed Steam GUARD.** Targeted review at `122ef38` confirms startup initialization,896 shared CRT draws, thread/descriptor/blocked/action processing and original observation publication for the existing offline graph. The original Steam jump skips private validators. Preserve the concrete queries when installing mission loading under OWN. [GUARD verification](evidence/guard-verification.json)
2. **HUM and reached positive leaves.** Reuse Agent #1's current reconstruction. Finish the DD0 action, air/floor, remaining geometry/animation events and B0 families actually reached by the selected mission. The item virtual24 route reaches `007D4FD0` and ShotMgr `0044B910` when firing occurs. Runtime type11 requires its embedded rigid-body virtual24. Populated Dog metrics remain conditional. Avoid assigning every interaction or weapon operation to the initial frame merely because the callback exists. [Reverse trace](reverse-dependencies.md)
3. **FRAME using completed UI source.** Construct the concrete Runtime over the existing owners and instantiate `GameMissionModuleFrames`. Bind session-level `0069A560`, `0069AF10`, `0069B230` services for Players, FPV/HUD/text, engine/model/dubbing updates, effects and shared blast processing. UI-owned receivers are complete and merged through UI7a3f7b7 and masterdd2fb6c: reuse `GameUiPresentation`/`GameUiSceneCallbacks`, preserving their same-World checks and borrower lifetime. Supply external gameplay/action producers when reached. The actual session/application consumer is still missing. [Current owner observations](evidence/blockers.json)
4. **Use completed RENDER source.** Reuse already merged RENDER345a390 and masterdd2fb6c wiring: `GameRenderStages`, lifecycle-owned callbacks/render policy→actual Scene78, positive volume/procedural/debug and special-light/VM services now exist. Preserve GEffects+C4/clock224, actual Scene15C, current-allocation history reads and callback-before-stages-before-World retirement. Master already has full UI7a3f7b7; combine its adapters with FRAME under OWN/LIFE. Reached conditional effects update producers and actual engine/desktop observations remain separate requirements. [Render verification](evidence/render-verification.json)
5. **OWN using completed LIFE source.** Install `GameSessionLifecycle` from the focused `f9d5f03` review, together with frame/render services, in `GameApplicationState`. Retire frame borrowers before explicit lifecycle close, then reset SessionOwner. Use the existing World-owned bounds, mutable squad and installed resolver. Unwind every completed loading/activation prefix before resetting the parent scene. Native checks should cover partial failure and repeated construction before the user desktop attempt. [Application trace](reverse-dependencies.md)

PACK4fccf12 is source complete: all23 payload families, five grammars and selected actual Human effects use the real process/FIFO/module13/Scene2 owners. Its source prerequisite is satisfied. Containing H8/FRAME simulation remains separate. DATA records selected-resource populations and source fault guards. Both remain conditional relationships in the graph. [Mission reachability](mission-reachability.md)

The first attempt must expose actual stage, validation or receiver failure and preserve coherent owners. Record resource fingerprints, actor/state kinds, first authority/presentation/Scene20 callbacks, Scene78 completion, callback stages, active camera and graphics present. A failed required stage leaves readiness tied to its actual completed state.

## First successful frame and full acceptance

P22-VIS requires original actors/collision, active camera and genuine scene/update/present execution after the startup fade. Keep updates running during the approximately 3–4 second black/fade interval. Observe contact/material, packet and Runtime branches that static source cannot establish.

P22-REPEAT requires user-operated Python failure return, Main re-entry and repeat mission loading with correct cleanup. P22-SECOND retains detailed-plan second original mission coverage. ALPS1's active data may require Dog services and additional positive branches. Full controllable-player/objective/save/result milestones follow their main-plan scope, while every operation actually required by mission loading remains part of P2.2. [Acceptance and reachability](mission-reachability.md)

## Targeted DATA implementation closure

DATA-owned implementatione76afc1 is already merged. Use its actual resource/file/cache scopes and loader/frame/render hooks. Existing native tools independently validated the historical captured bytes. OWN/FRAME/RENDER and VIS/REPEAT/SECOND retain fresh runtime context, populated collections, reached positive failures and desktop acceptance. The old missing GEffects material94 receiver claim is superseded by actual source. [DATA verification](evidence/data-verification.json)

## Targeted UI implementation closure

UI7a3f7b7 is merged and masterdd2fb6c wires its same-World render loan/callback/lifecycle. Reuse the completed HUD/FPV/text/control owners. Earlier reviewebdcbf5 againstb132e6d remains historical. The five documented overlap resolutions were compiled over05dbdee; preserve newer Car fuel/collision-clock/mounted state during actual integration and verify the composed build. Construct UI after World readiness; remove containing callbacks and destroy the adapter before World close. Required external producers remain with HUM/Actor/Items/PACK and session/render coordinators. Full acceptance still needs actual rendered mission, failure/repeat and second-mission observations. [UI verification](evidence/ui-verification.json)

## Targeted RENDER implementation closure

RENDER345a390 is merged; masterdd2fb6c completes available UI/render/application Scene78 wiring. The review against2c96a7d remains historical. Do not reconstruct another volume history initializer, process cloud owner, render stage or Scene callback bridge. The original bug reads current unwritten allocation3C/40 before replacing it; this source behavior is now implemented without a chosen angle or extra random draw. Integrate RENDER callbacks with the current UI/FRAME lifecycle borrowers and actual OWN invocation; preserve the already wired UI modal special-frame retirement to `world.scene().effects().configure_special_model(false,nullptr,true)`. Preserve conditional water/weather/game64/colour update failures until their actual producers exist when reached. Desktop Scene completion/presentation/fade, repeat and second mission remain open. [RENDER verification](evidence/render-verification.json)

## Current FRAME gate

The2026-11-13 [FRAME reassessment](../p22-frame-dependency-review-20261113.md)
keeps containing H8 frames, first-use physics5F9, positive non7 retained-item
restoration and conditional services open. FRAMEc36bba8 and OWN939fb70 remain
unmerged; OWN's later activation/controller source still guards frame forwarding.
Do not reopen PACK, UI or RENDER source completion to cover these owning contracts.
