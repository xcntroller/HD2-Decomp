# P22-OWN implementation —2026-11-12

[Plan](p22-own-plan.md). Isolated branch `feature/p22-own`; fetched origin/master
and actual implementation base **fa94dee38922a21397289a7b5746819c83b42d89**.
Branches/worktrees/status/history inspected before creation. No pre-existing OWN
branch/path. No master/other checkout writes, resets, merges or pushes.

## Completed available application ownership

Actual kind7 in `application_launch.cpp` supplies a mandatory concrete session
factory to GameApplicationState. The controller installs that exact type1 BEFORE
initialize, as before. The factory checks controller.active identity and composes
GameApplicationSession<actual Controller> over the SAME existing SessionOwner,
parent.backend(), accepted request, live squad, native status_storage, Startup,
Graphics, driver, module, catalogue and frame clock. No second World, roster,
registry, camera, cache, device, clock, RNG, scheduler or presentation owner exists.

`game/application_mission.h` is a small owning virtual boundary needed because
World includes its parent's host header. Its SessionActivationState aliases the
actual application status; it never writes a synthetic loaded14/status5. The
concrete factory product owns the existing GameSessionLifecycle, whose one
GameMissionLoadingSession owns the existing World. Initialization now invokes
actual006989B0 ->006CA780 instead of the previous unconditional failed-load lambda.
Actual false results and exceptions remain failures. Factory-construction failure
unwinds its acquired prefix; a complete product is installed only on success.

The concrete session binds actual lifecycle UI/RENDER callbacks before activation.
Render forwards to lifecycle0069B870/actual Scene78, preserving original stages,
configuration170 policy and renderer state/history. Application fallback callback
forwards to the exact installed lifecycle dispatcher, which validates the Scene
callback identity. No new callback suppression, synthetic camera or empty HUD.
Network overlay00724CB0 remains the existing actual RET.

Native64-word writable input provenance is resolved from Startup's SAME produced
first/second vectors; both size and exact begin identity are required. No const
cast, copy, input re-poll or alternate neutral collection is introduced. Application
input/fixed/update forward through the owned session boundary. Actual FRAME
forwarding is explicitly guarded until the owning coordinator is imported.
Successful initialization is **not enabled with missing FRAME**: if real activation
reaches its success return, the missing-frame error triggers ordered close rather
than allowing an unbound session to run. These are missing-body failures, not
installed successful receiver implementations.

The application also exposes actual0067FAA0 mission_save_blocked over its SAME
retained A8 descriptor: two fresh forward scans, empty/all4 ->false, any0 ->false,
other populated states ->true. Malformed reached pointers reject as a source
safety guard. This is not saved-state production or a successful save.

Saved-state query now reads actual mode04 under ApplicationStartupData,
initialized0 by006B4670, via Startup and application accessors. Raw0069B230
reads008AE924+4 (application+28 mode), NOT GameData/application+04 and not
a saved descriptor pointer. This corrects the stale FRAME interface comment.
The existing route remains sole mode08/selection authority; only the missing
initialized scalar is added to the existing application data owner. No new save
producer or reset is invented. Future actual save producers must write that
SAME field; nonzero values are read unchanged.

Warning19C7/2536 is concretely bound through application ->owned session ->actual
Startup localization ->same-World HUD append_message with FLOAT255 RGB (mode2),
or original console route otherwise. It preserves2536 first format(format,0),
then0069BF70's second no-argument format, while19C7 uses only the second pass.
Native INT zero formatting is bounded to supported integer conversions and the
original512-byte game-message boundary; unavailable varargs/pointers/floats,
extra arguments and overflow reject as source safety guards. No configuration170
logging gate is introduced into this direct game-message operation.

The controller now owns original scalar04, constructor0, with a deferred request
setter/getter. It is distinct from active-state04/status, state pending28 and
controller phase28. The application provides exact current-state identity and
checked deferred request delivery for the future FRAME external adapter. No
save/load is performed or claimed by setting that word; its broader save browser
consumer remains a later original application path.

The existing FrameClockPlatform now lives in LaunchOwners, before services and
controller, and supplies BOTH the existing outer FrameClock and mission graph.
Explicit teardown resets controller/services before this SAME clock. This changes
native source lifetime placement, not simulation/present ownership or RNG order.
Startup now owns the actual persistent item-model suffix string; item/scuba/FPV
caches retain its reference. The original pointer0084E8E4 points to00867980 with
an initial NUL in the fingerprinted client; no guessed extension is appended.

## Failure and lifetime rules

Only genuine activation success may pass the application boundary; source guards
verify actual loaded14/status5 on that return. Subsequent native status5 ->1 ->6
and other FRAME transitions remain admissible: there is no status5-only dispatch
gate. Scheduling is still the existing render-before-fixed/update application
frame and graphics present remains at its original outer owner.

Activation exception is retained while close is attempted. Secondary cleanup
failure is separately available and does not replace it. A failure allocating
the initialization diagnostic string also rethrows the original exception. Failed close retains
the session graph, loaded state and parent, rejects further dispatch, and permits
explicit close retry. False activation closes once; it does not automatically
retry a failed cleanup twice. A close returning with loaded14 still set fails.
Application shutdown closes the complete mission BEFORE unbinding game views,
resetting SessionOwner or status. If initialization and cleanup both fail, its
original exception wins at the receiver boundary. Destructor cleanup is guarded;
member order still destroys lifecycle/World before parent even on final unwind.
Final destruction always retracts Startup game views, even after failed explicit
close; no dying application pointer remains in that process borrower. Explicit
shutdown failure still retains graph/views for retry until final destruction.
This safety/exception layer is source policy, not recovered native SEH equivalence.

Existing receive_request releases the failed application before constructing its
error UI. A mandatory cleanup failure cannot become a successful UI return;
explicit retry or final guarded unwind applies. No fallback UI receiver has been
installed to make missing gameplay operations look successful.

## DATA observations

The concrete session uses existing ResourceObservations plus ChunkFiles and
LoadedResources observe_scope around actual activation, close and render. Actual
loader selection/resource events, body operation events, exceptions and same-World
population snapshots use the existing DATA APIs. Snapshot failure records a drop
and cannot mask a primary engine/source failure. No diagnostic callback is manually
invoked and called a production binding, and no fixture collection is a mission.
Historical resolver bytes, static requirements, actual loaded populations and
executed runtime branches remain four different evidence levels. No new live
mission capture exists. The session exposes its actual store for callable inspection.

## Exact remaining dependencies and next integration

| Owner | Producer/interface -> consumer and affected branch |
| --- | --- |
| FRAME import | `feature/p22-frame` c36bba8 supplies GameSessionFrames, GameSessionFrameExternal, lifecycle bind_frames/input/fixed/update/retire_frames. None is in this base. Reconcile its lifecycle with current RENDER dispatch/binding. Replace GameApplicationSession's three missing_frames dispatch guards with that actual lifecycle coordinator, and satisfy its mandatory Runtime before removing the final activation admission guard. No competing coordinator is copied here. |
| FRAME + HUM/Player/Items/Dog/PACK | The owning branch's `p22-frame-external-contracts.json` lists54 typed Runtime contracts, not54 observed Tutorial failures. Includes actual0053BF90 input, B0/DD0 actions, weapon/item/attachment/formation, Dog/Sight, embedded Runtime11 physics and positive005B7EC0 retained restore. Current master already has physics kernels; the branch's old missing-body observations are stale. Inspected HUMb99f088 (after5711792) records blocked source closure and adds Car event/inventory/pending-damage receivers but is unmerged; its remaining5F9 producer/occupied/containing guards must not be fabricated. |
| OWN wiring after FRAME import | Adapt the exact same-world GameSessionFrameExternal using the available UI advance_hud/process_hud_controls/advance_first_person and real GameUiHudFrameSources/GameUiHudControlSources producers. Connect current-state/deferred-request APIs above, completed mode04 saved-state and mission_save_blocked0067FAA0 accessors above, completed warning0069BF70 forwarding, and Script end-cutscene00423EB0. The source-only FRAME contract still requires these endpoints; their presence in an abstract interface is not a production binding. The saved-state accessor is now concrete. Script cutscene fields824/828/82C/830/834 and frame lists868/87C/88C have no positive producer/typed ownership in current MissionScriptModuleLoading. Its actual private fields824 are cleared at activation; they are not a substitute positive camera/frame owner.00423EB0 restores camera parameters/Scene camera, dispatches reverse cutscene_done contexts/Human notifications, restores game6E/64, viewport/aspect, GEffects8C, and retires owned/borrowed frame lists before final control reset. Raw929-byte body is retained; no cutscene-end byte or successful absent producer substitutes for it. This specific OWN integration body remains open behind P22-FRAME/Script module4 and HUM cutscene producers, alongside FRAME import. |
| UI integration seam | UI's merged source is complete and its HUD/FPV/control consumers exist. FRAME additionally requests filter_hud_input0049FC10, hud_nonoffline_input_blocked0049F090, hud_player_input_blocked0049D300. Current GameUiPresentation has no methods with these contracts. Original filter reads actual HUD2F30 and signal visibility before consuming scans52/4F/50/51/4B/4C/4D/47/48/49 through006C71C0. Two queries read actual flags/map108FC visibility, command10B20/inventory34 visibility and message10315. Resolve the UI-owned accessors/producer mapping when integrating FRAME; do not force false predicates. Available binding-consumption and panel visibility leaves are preserved. |
| HUM/Actor + UI sources | UI source-owned receivers require actual order/change/count/mutable44, route/path/model, selection/action/occupied/Sight/realtime producers on positive branches. No null source or default empty list is claimed as a binding. See merged UI source-closure and current HUM handoff; conditionality depends on actual loaded populations. |
| PACK/control, conditional | Nonoffline descriptor/peer/match/queue and external configuration170 loading-progress00731720 remain required only when their real branches are reached. Offline mode/flag values are read from actual Startup, never forced. |
| LIFE + FRAME retirement | Imported lifecycle frame_retirement_started_/retire_frames must run BEFORE Scene suspend/unbind, UI loan retirement and World close, on success and every failure prefix. Existing LIFE prefix journal and primary failure behavior remain authoritative. |
| VIS/REPEAT/user acceptance | Real selected resource loading/populations, actual first frame/engine draw/present, failure/Main return/re-entry and desktop acceptance have not been executed. |

This branch completes independently available OWN composition and provenance work.
OWN is **blocked, not source-complete overall**: concrete FRAME external/runtime
installation and final dispatch remain open behind those imports/producers. No
remaining RENDER-owned body or old volume-history provenance question is asserted.

## Original evidence and verification levels

Client SHA2568c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.
Research index used before bounded address searches. Existing loader/module
006989B0/006CA780 exports, engine-loop006C0750/006C20F0,006DC940, kind7 and LIFE
contracts were checked. Original006C0750 writes controller04=0;0069A560 writes2/3
to006DC940's SAME008AED14 controller04. Bounded raw0049F090/0049D300 instructions
are retained in ignored WORK. C export signatures with unknown convention are
not used as ABI proof; existing typed host owners and original call/RET contracts
remain the binding authority. Source callback/lifetime guards earn no body credit;0067FAA0 is a newly implemented frozen body, partial only.

Native ASan/UBSan/LSan PASS100 ownership/alias/admission/false/primary-secondary/
retirement/retry/provenance/controller04/receiver-fallback/objective/warning
families. Engine/World/FRAME/application/parent endpoints are explicit doubles.
Actual concrete i386/application links are recorded in the receipt. Full tracker
retains unrelated missing0062D880 ignored evidence; focused baseline/uniqueness/
changed-row validation passes. New body0067FAA0 gets partial credit only. Base
ledger had3,004 partials despite stale README2,997; updated count3,005 is a
one-entry OWN delta, with34 complete and unchanged18,783 denominator. No original EXE/DLL/Proton, Windows
fixture or application output has been run. No permanent tests are added.
