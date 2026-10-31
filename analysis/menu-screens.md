# Registered menu screens and dependencies

This inventories the fourteen definitions registered by `00623DD0` and the
identifier-specific builders called by `00627060`. It is **not** the complete
reachable-menu census: the [complete owner/route census](menu-route-census.md)
now records nested products, in-game variants and application boundaries separately.
[Machine-readable evidence](menu-screens.json) contains exact IDs,
model strings, call sites, 53 static children and 100 initializer call sites.
These are metadata; no original pseudocode or localized caption text is included.
The [complete control/dependency contracts](menu-contracts.md) now extend these
static records across all owners, dynamic construction, captions and return rules.
The [identity resolution](menu-definition-resolution.md) preserves all fourteen
ordinary slots, including both same-name mission/cinematics definitions.

| Slot | Definition | Model key | Static children | Augmentation | Runtime |
| --- | --- | --- | --- | --- | --- |
| 1 | MAIN MENU | `main_menu` | 17 | — | Bounded functional |
| 2 | CHANGE PROFIL | `change_profile` | 17 | 006211b0 | Not enabled |
| 3 | SINGLEPLAYER | `singleplayer` | 15 | — | Not enabled |
| 4 | CONFIRM | `confirm` | 4 | — | Bounded functional |
| 5 | SINGLE MISSION 2 | `single mission 2` | 0 | 006391e0 | Not enabled |
| 6 | OPTIONS | `options` | 0 | 0061cf60 | Not enabled |
| 7 | SINGLE MISSION 2 | `single mission 2` | 0 | 00614140 | Not enabled |
| 8 | multiplayer menu | `multiplayer menu` | 0 | 00633f80 | Not enabled |
| 9 | create server | `create server` | 0 | 00628490 | Not enabled |
| 10 | PASSWORD | `password` | 0 | 006377c0 | Not enabled |
| 11 | Server Filters | `server filters` | 0 | 00638590 | Not enabled |
| 12 | Game info | `gameinfo` | 0 | 0062f3e0 | Not enabled |
| 13 | MP Connect console | `confirm` | 0 | 00632080 | Not enabled |
| 14 | ADD SERVER | `password` | 0 | 00636ff0 | Not enabled |

Slot zero is a separately constructed runtime confirmation object from
`0066C1D0`; it shares the `confirm` model with slot four, but its identity and
definitions differ. Its four-mode request/reply/key protocol and queued stack
routes are now recovered and native-audited; the slot-zero engine product remains
unbound in the host. See [runtime confirmation](menu-runtime-confirmation.md).

Slots five and seven are distinct despite identical names/models: the original
construction loop sends them to `006391E0` and `00614140`, respectively.
The Cinematics button selects slot seven. Keep their identifiers and builders
separate. An empty static child list above means construction is delegated to
the augmentation, not that the screen has no controls.

Credits changes application state to twelve through `00626B20`; it requires
tracing that state’s lifecycle instead of inventing a credits screen. Options
passes through `0061BEA0` and settings-backed messages before selecting slot six.
The options builder references Video, Sound, Controls, Game and Multiplayer
groups; their nested controls/settings semantics are unfinished. Profiles need
editable text, lists, difficulty state and profile persistence. Single Player
needs selector/radio controls and callbacks `0063B590`/`0063B740` before activation.
See the [options construction inventory](menu-options.md) for its six settings
groups, navigation group and unresolved widget/callback dependencies.

The reconstructed shared stack supports regular selection (hide current, push,
activate with focus zero), modal push (retain current visibility, push, activate
with focus `10200000`), back/pop and resume. Registry ownership outlives stack
entries. The host binds only slots one/four. Unsupported bindings are rejected
before changing a live screen. Targeted delivery for the two live screens is integrated and desktop-confirmed;
screen-specific observer/data callbacks, live generalized factory integration,
application state transitions and resource/cache lifetimes remain unfinished.
Native vector storage is not the original segmented-deque ABI.

## Shared construction and ownership recovered, 2026-10-30

`frontend/screen_construction.h` now reconstructs the shared 00627060 loop and
00668470 binding method. The loop loads each model before calling its builder,
skips failed loads, runs all eleven typed builder routes and positions the
confirmation model at (0.5, -0.375, -0.5). Identifier checks are independent and
reread the definition after callbacks; the two identically named mission screens
remain distinct. Builder bodies are still application-service dependencies.

The screen +8C method attempts every child, using the screen model as both
search and layout root, ignoring individual factory results and rereading child
storage/count. Only afterward does it retain the model name and assign the
screen identifier. The outer loop then gates on +8C success, runs the actual
mutable initialization list through +70, and appends the screen. Definition
storage is destroyed before directly routing initial 02000001 argument one.
Native products must retain their own metadata, model and child ownership;
factory/resource backends remain responsible for concrete construction and
exception cleanup. Observer contexts still borrow application services and must
outlive their registered screen products. Unlike the original failed-binding path, a rejected native
product is reclaimed by RAII, and null allocation throws instead of dereferencing
null. These are deliberate native failure policies.

`OwnedScreens` provides stable product/registration addresses, copies names and
model keys, and compacts successful insertions without rewriting definition IDs.
The same `ScreenStack` now accepts either this owner or the existing fixed host
registry; it stores the actual registration index rather than subtracting a
pointer from the global table. Stack entries and its previous-screen pointer
borrow registry metadata, so the stack must be destroyed before its registry.
Products remain responsible for their own backend/child resources; this owner
does not reconstruct the original class/vector/destructor ABI.

`DefinedScreenEvents` now copies its ID/model key and already-copied child IDs,
so message routing survives factory-definition teardown. Its new mutable-record
entry point preserves initialization query outputs; the existing const entry
point deliberately retains its copy semantics. Definitions can carry typed
screen initialization records, and the live main/confirmation message binding
path installs them after all child endpoints exist. Both current lists are
empty: no original application callback is silently installed or guessed.

Temporary ASan/UBSan audit and strict Windows i386 compilation pass for the
actual screen runtime, event/observer code, owning registry and stack. Coverage
includes all builders, a builder changing an identifier before a later check,
definition/child-list growth, failed loading/binding, rejected-product cleanup,
partial construction after exceptions, actual mutable query records, retained
payloads and callbacks after definition destruction, stable metadata after
registry growth, regular/modal/pop/clear paths and reverse owner teardown.
The strict i386 host build, existing native suite, nine frontend and seventeen
launcher checks also pass. No new permanent tests or agent-side Proton launch.

The [construction map](menu-screen-construction-map.json) validates ten byte
anchors, nine screen vtable slots and sixteen direct calls. Two census entries
gain PARTIAL coverage (00627060 and 00668470): 98 partial (94 client/four engine),
zero complete, frozen 18,783. Generic ownership glue earns no extra credit.

This is a source construction/ownership slice, not a completed screen checklist
entry. The host still uses its existing two-screen construction path, not the
generalized factory. Slot-zero protocol is recovered in the following slice;
its concrete engine/input binding, backend/builder integration, screen-specific
callbacks and remaining application routing are still open.

Next implementation dependency: recover original observer/data callbacks and
nested group construction, then choose a flow whose data/controller boundary
can be completed. Do not enable another button solely because its model loads.

## 2026-10-30: live construction and application routing integration

The main/confirmation host now calls `construct_registered_screens` through
`host/menu_screen_factory.h`. Engine model loading remains a supplied resource
service: this adapter receives the existing preloaded model roots. Each product
separately allocates, copies/binds a definition, initializes mutable root records
and enters `OwnedScreens`. Factory definitions are destroyed before direct
02000001 initial selection; the queue starts empty rather than pre-seeding a
screen. Host event bindings now use an indexed family table. The native owner
contains only two implemented named screens in construction order; the host's
original static event registry remains at indices 1/4, with slot zero separate.
No unavailable screen builder is silently treated as successful.

The slot-zero engine/input path was user desktop-confirmed before this slice.
It now uses the separate `construct_runtime_confirmation` adapter, installs its
owner before binding and registers after success without root initialization.
This factory startup integration is user desktop-confirmed (732 frames, clean teardown).

`frontend/application_events.h` recovers the five 00626b20 callback-service
priorities and common current-screen back routing, including profile cleanup,
family 14/10 deferred discard/pop, family four cancellation and generic back.
Credits selects state 12 through a service; resume remains a service rather
than reconstructing the unreliable exported save-string body. Concrete screen
callbacks, application transitions and remaining common dispatcher service cases
remain open. Unknown derived events retain the host's explicit failure instead
of the original silent ignore. The [routing map](menu-application-routing-map.json)
validates 17 original anchors and six calls. 00626b20 is absent from the frozen
inventory; existing partial-function coverage expands without count credit.

Temporary sanitizer/native/i386 audit covers factory stage order, definition
teardown before startup selection, direct selection before pending work,
runtime singleton stages, all 64 back families, callback precedence, and actual
queue deferred quit/cancel behavior. Strict host build, existing native suite
and 158 Python checks pass. No permanent tests or agent-side Proton run.
