# Merged UI / RENDER wiring —2026-11-12

The merged UI could render HUD/command geometry, but004B5E40 modal cleanup still
required a Human/order source to implement the render-owned004870A0 call. The
application's render callback also still unconditionally threw. Both available
render paths are now connected over the existing owners; successful mission
activation and the unmerged FRAME coordinator are separate.

Worktree feature/p22-render starts at973b4f445240ab92df71f62200a71b25e48d8318,
whose tree629d5e206ec501ee0a791f7108bc748e30dfe28c exactly matches merged master
 a820e7018a625dc8a31a314706ab2f4122e0f2d8. The user's merge already includes UI
7a3f7b7 and RENDER345a390. No master/other worktree writes, reset, merge or push.
Inherited HUM/master handoff instructions are superseded by the user's isolated
wiring request. [Plan](p22-render-wiring-plan.md),
[receipt](checkpoints/p22-render-wiring-20261112.json).

## Concrete routes

| Trigger | Actual receiving path |
| --- | --- |
| Application render | GameApplicationState::render → existing render_session0069B870 policy → SAME parent Scene78(scene,0) → installed GameUiSceneCallbacks. |
| Lifecycle render | Existing GameSessionLifecycle::render → SAME policy/Scene; requires its own actual dispatcher still installed. |
| Stage1 | Original006C9260 HUD preparation → bound same-World GameRenderStages pre-scene → gated debug; no presentation/simulation/clock ownership change. |
| Stage2 | Merged UI HUD/positive command geometry or full viewport → text → actual RET00724CB0 → gated session fade → post effects → process text overlay. |
| Command modal special cleanup | Original004B5E40 gate → CommandModalQueries → UI's bound render stages → GameRenderStages → SAME Scene.effects().configure_special_model(false,nullptr,true). |

GameUiCommandModalSource no longer asks gameplay producers to supply render
cleanup. Orders, selection, command camera, Sight and realtime remain required
actual sources. No such operation was replaced with success. The existing
GameUiRenderStages interface now also supplies its World identity and concrete
special-model retirement. GameRenderStages is the production implementation.

The callback adapter implementation lives in game_ui_scene_callbacks.h and is
instantiated with actual GameUiPresentation through the existing public
GameUiSceneCallbacks alias. This is the same callback/policy pipeline, not a
second dispatcher or renderer. The type parameter permits native audits against
explicit UI/platform boundaries while strict i386 uses the actual host classes.

## Binding, publication and retirement

GameUiRenderBinding reuses WorldLoadingDependencyLink. It holds one render-service
loan and the owning callback token inside the existing UI object. No ownership
registry, retained World, device, camera, clock, cache or effects owner is added.

1. Lifecycle constructs the actual render stages over its ONE World.
2. The actual callback constructor binds the UI loan; null owner, duplicate and
   foreign-World sources reject before publication. Copying callbacks is disabled
   because the callback address identifies the borrower.
3. Only then does existing Scene.bind_callbacks publish/install the dispatcher.
   If registration throws, Scene restores its prior pointer/active guard and
   the local callback's destructor retracts its UI loan. Owned render construction
   stays local until successful registration; retry is possible.
4. Each dispatch validates the live token before HUD or render operations. Stage1
   pre-scene/debug resolve the same bound receiver. Modal operations resolve the
   current UI loan only when their existing original branch calls cleanup.
5. Existing LIFE finish/unload/rollback stops engine callbacks, unbinds Scene,
   destroys the callback (retracting UI loan), then destroys render stages before
   World/Scene/camera/textures. Retiring an older callback cannot unbind a newer
   loan to the same renderer. Explicit callback retire is idempotent; stale
   dispatch fails instead of touching a retired renderer.

These are native safety guards, not original engine exceptions or new gameplay
policy. Actual EffectsSpecialModel retirement still restores raw mode/power,
hides/releases the existing frame and preserves its first failure while cleanup
continues. No extra random or update call is introduced by this wiring.

GameScene.has_bound_presentation checks engine registration, active Scene identity
and a real borrowed dispatcher. Its identity overload also checks the exact
owned callback. Lifecycle Scene entry rejects a suspended/replaced dispatcher;
application Scene entry rejects missing presentation. These guards do not publish
loaded14 or infer mission readiness from callback presence.

## Application boundary

GameApplicationState.session_owner() exposes its already constructed SAME parent
for OWN composition, using the existing registry identity check. It allocates no
replacement parent/lifecycle. OWN can construct the existing GameSessionLifecycle
with that parent's backend and SessionActivationState(application.status_storage())
so game04 stays the actual application status word. The lifecycle and World must
retire before the parent/application; this accessor does not install ownership.

GameApplicationState::render calls the existing0069B870 policy, preserving offline7
always-render versus non7 configuration170 skip. Actual Scene entry requires
bound presentation; it never calls the old application fallback HUD helpers to
simulate a World. Actual Scene's existing borrowed dispatcher selects the UI
adapter. Policy and Scene operations use existing DATA scopes, so policy skip
completion is separate from entering Scene78 and diagnostics preserve failures.

Application initialize STILL declines activation; input/fixed_update/update still
explicitly reject missing loaded-session integration. No successful loading,
forced status/HUD flag, new scheduler or graphics present was introduced. The
application render entry is callable once OWN installs actual lifecycle/presentation.

## Exact remaining integration

- FRAMEc36bba8 is NOT an ancestor of this merged base and
  recovered/host/game_session_frames.h is absent. Import that owning branch;
  its bind_frames/input/fixed_update/update APIs and actual Runtime/external
  services cannot be assumed here. Preserve its frame_retirement_started_ /
  retire_frames BEFORE Scene/presentation retirement when composing lifecycle
  changes. Do not create another module/frame coordinator.
- The current ApplicationState input signature uses const sampled views, whereas
  FRAME's actual controls require mutable64-word original arrays. OWN/controller
  must expose their writable provenance; do not cast arbitrary read-only views.
- OWN installs/stores/closes the ONE lifecycle/activation state, completes actual
  loading/activation and controller/cutscene services and forwards input/updates
  after the FRAME import. The render forwarding above is already connected.
- Actual Human order/path/presentation, selection/Sight/realtime/occupied and
  gameplay/PACK services remain their typed UI interfaces. Their presence as
  required interfaces is not a claim that every body is missing on every branch
  or that all are reached in Tutorial. No fabricated empty populations or success.
- Actual loaded resources/populations, executed engine draw/present and user
  mission/failure/return/repeat acceptance remain unobserved. Native fixture
  collections and compile-only Windows bodies do not establish mission reach.

## Verification

Native ASan/UBSan/LSan passes100 real render-binding/callback/policy/special-effect
lifetimes: foreign/duplicate/null rejection, live/retired/stale tokens, failed
registration unwind/retry, replacement, stage order/full viewport near/far, status14,
pre-scene failure/retry and actual EffectsSpecialModel restoration/release including
primary/secondary cleanup failures. World/UI and engine registration endpoints are
labelled doubles. A second100-family audit uses actual GameScene and the actual
callback adapter with engine/platform/World/UI doubles: SetCallback failure/retry,
registered/active/exact-dispatcher admission, Scene78, policy gates, callback-primary
versus later engine error, suspension and replacement. The host UI body itself
and application class execute only in the compile/link verification below.

Strict i386 full actual World/loading/UI/render/callback/modal/lifecycle and
application-owned render composition links. The defined throwing gameplay double
supplies real required modal interfaces and deliberately has NO render-cleanup
method; actual GameRenderStages supplies that path. No unresolved project leaves
or declarations-only receiver stand in for this wiring. Existing application link
passes separately, with final heavy work sequential. Windows outputs and original
EXE/DLL/Proton never run. Temporary sources/binaries/stubs are fingerprinted/removed;
concise logs retained. No new original-function credit, inventory change, full-body
promotion or desktop claim comes from these bindings or native guards.
