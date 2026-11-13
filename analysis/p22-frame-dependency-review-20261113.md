# FRAME dependency reassessment —2026-11-13

**FRAME remains blocked; source closure is not justified.** Review of the clean
`HD2-Decomp-p22-frame` worktree on feature/p22-frame atc36bba8 against committed
PACK4fccf12, HUM H851082a2, masterfa94dee and OWN939fb70. No source merge/import,
other-worktree change, original/Windows execution or application enabling.
[Machine-readable evidence](p22-frame-dependency-review-20261113.json).

The user requested closure if FRAME is now unblocked. PACK's completion resolves
its selected packet-effect dependencies; it does not complete the containing
Human simulation or the remaining FRAME contracts. Available later source is
recorded below so the old handoff is not mistaken for current availability.

## Advanced dependencies

| Family | Current committed source availability |
| --- | --- |
|PACK|4fccf12 integrates selected Human packet effects through51082a2, including common commands/DocumentPlace23, mapped actions/HeavyFire, general inventory, clothing, special animation, equipment, speech and rank. Its source closure explicitly leaves full H8 frames and conditional nonoffline work open.|
|HUM|H8.1 containing composition and many actual World/Runtime leaves exist. Formation, Script events, item/weapon, attachment, damage and some timed interaction starts have advanced. The old54-operation compile list is historical, not54 still-missing bodies.|
|Physics|PACK/master contain real RigidBody.update and GameWorldActors.update_detached_body with full ContactEvent ABI. The FRAME base predates them. This is an available import; it retains the first-use5F9 guard below.|
|UI|Completed GameUiPresentation HUD/control/FPV source exists on current master and OWN. It is no longer only an unavailable UI branch. Actual FRAME external-source binding is still needed.|
|RENDER|Current master contains completed per-volume3C/40 raw-allocation history, Scene/render-stage/callback/procedural/special-frame/debug source. The former3C/40 source blocker is resolved; it must not remain listed as an unresolved external defect. FRAME has not imported it.|
|OWN|939fb70 supplies actual application mission activation, controller and failure ownership over merged UI/RENDER. Its session input/fixed/update still call missing_frames pending FRAME integration; no completed frame Runtime is installed.|

## Remaining source blockers

1. **Containing Human simulation remains unavailable.** In PACK4fccf12,
   `recovered/game/human_frame_runtime.h` still explicitly rejects
   `advance_human_state` (actual Active/Remote B0), remaining00751480 action
   receivers and00755500 containing control motion. This is not merely an unused
   adapter name: `HumanWorldLoadingServices::advance_human_state` calls
   `runtime.advance_human_state` for kind3 authority (among other families);
   only kind3 presentation has the completed special dispatch. The actual H8
   plan still leaves H8.2 mandatory offline decisions/receivers, H8.3 async combat
   and H8.4 reached actor dependencies unchecked. PACK command starts do not
   complete those frames. No mission capture proves the missing paths inactive.

2. **Fresh rigid-part5F9 has no native producer.** The real later physics source
   in both PACK and master still throws before reading/toggling an absent5F9
   alternating-query byte in004CB2B0. Importing it does not establish the original
   first-use state. It is a reached conditional physics obligation, not a missing
   whole RigidBody implementation, and cannot be replaced by zero/false/random.
   The RENDER3C/40 solution is a separate original allocation/ownership contract;
   it supplies no5F9 producer or physics policy.

3. **Positive retained-item restoration is still missing.** FRAME's
   `GameSceneActorFrames::restore_loose_item` binds005B7EC0's offline7 RET and
   requires `runtime.restore_loose_item(item,SAME Runtime)` otherwise. PACK's
   Runtime retains `retained_items_` during creation and clears the vector, but
   supplies no positive005B7EC0 restoration consumer. Packet ownership and packet
   effects being complete do not establish saved pose/item restoration.

4. **Other conditional/nonoffline and containing integration remains open.**
   Actual FRAME `session_frame_loading.h` still rejects0069AF10 nonoffline
   match/peer and0069B230 clock/match/control continuations. Current Human Runtime
   also retains nonoffline input/presentation/visibility and reached contact/
   respawn requirements. Dog/Sight hooks remain external in current
   GameMissionModuleFrames. OWN's GameApplicationSession explicitly guards frame
   forwarding. UI/render/OWN source availability therefore does not constitute
   a bound complete FRAME execution graph. Their completed bodies should be
   imported with matching owners when the required graph can be completed.

The first Human and physics findings alone prevent closing FRAME, independently
of the later integration/desktop gates. This review does not claim every guarded
branch runs in every mission; it preserves the plan's positive completion boundary
without inventing fixture-derived inactivity.

## Result and verification

Keep the FRAME source checklist open. The verified session/input/control/lifecycle
and Runtime11 containing checkpoint atc36bba8 remains intact. Resume with actual
remaining H8/runtime/physics contracts, then integrate available UI/RENDER/PACK/OWN
owners and verify the complete affected graph. The existing source54-operation
JSON retains its base-qualified sites and now links this reassessment.

Verification was committed-source comparison, direct caller/guard inspection,
exact revision/blob/SHA256 pins, JSON validation and `git diff --check`. No rebuild
or sanitizer rerun was warranted for unchanged production source. No function
credit, ledger/README tally, readiness flag, successful placeholder or desktop
claim was added.
