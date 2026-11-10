# World scene and actor ownership — source checkpoint, 2026-11-09

P2.2 remains open. `GameApplicationState` still declines loading. These source
owners compile with actual host types; there is no integrated mission, first
world frame or new desktop acceptance. Continue the full implementation checkbox.

## Shared scene and collision owners

`host/game_world_scene.h` now composes `GameMissionScene`, B700 environment,
light-map aliases, GEffects shadow/fade owners, actual collision mesh views,
`CollisionTree` and its dynamic buckets/exclusions. The engine scene and two
navigation datasets remain owned by the existing `SessionOwner`.

Every actor/Items/physics/Human/script user must borrow the **same**
`GameActorServices` base in `frames()`. In particular, constructing separate
services for Scene2 and callbacks would split frame+EC bindings and item entries.
Car callbacks need those services themselves: scene construction now allows
one explicit callback binding before the first Car. The required callback
factory receives the actual services, must return an actual owner, and cannot
default to a successful service. The old constructor remains supported.

Collision loading reads the actual resolver's `missions\\<resource>\\tree.klz`
and invokes the recovered006EE570 decoder with the real mesh-view adapter.
Missing/short/oversized required input fails. The tree receives its final stable
allocation **before** buckets borrow its dynamic grid; there is no moved-grid
reference. Replacement requires retiring the current owner. Close clears actor
borrowers while callbacks live, then callbacks/buckets/tree/mesh views before
the containing session resets/releases the engine scene. Closed owners reject
new operations. This is a source lifetime composition, not a new original body.

`host/game_world_actors.h` composes the selected catalogue's actual model service,
Items owner, rigid-body registry, runtime actors and Physics against that same
scene and bucket owner. The containing graph can now construct it cold before
the collision tree; queries/registration still require its actual grid binding.
See [containing construction](containing-world-loading.md). Its common active-list callbacks dispatch to the actual Scene2 or
runtime registry, based on the existing ActorBase policy. Sound/network/item
transfer and presentation-reference removal remain required real consumers.
There are no default implementations for them.

Teardown retires runtime35 before its borrowed Physics object/descriptor and
runtime11 before its body/Items entry. Ordinary Scene2 actors retire before
remaining entries and cached template models. Containing Human/state/script
owners must finish/retire before this close; the scene/session/catalogue and
consumer owners must outlive it. Activation/finish event order still belongs to
the complete session. Constructors alone cannot report loaded status.

## File stages and live document scope

`game/mission_scene_documents.h` supplies actual006CBF30/006D45C0/006D4880/
006D4280 wrapper policy around the existing parser and table decoder:

- Scene2 calls the required shared004D6900→004D57A0 Car prelude before opening.
  Missing file returns3. An opened file registers existing frame/sector names
  and prepares the original camera; short first header returns1. A complete
  root other than4C53 is ignored with status0, as original.
- Actors/sounds are optional; present4C53 roots enter the same parser with
  stages2/3, same borrowed name map and persistent Scene2 version/fields.
- `check2.bin` decodes both existing navigation datasets through00523C00.
  Missing check2 clears those datasets before probing `check.bin`. Static
  assembly006D43E4..006D459D confirms the legacy-present path only closes its
  file: it does **not** invoke a legacy decoder. Missing both clears again.
- Every parser runs through `GameSceneObjects::read_scene_document`. Embedded
  light-map payloads require its **live archive handle and matching byte view**;
  a second file read/copied buffer would violate that existing endpoint.

The source host buffers input before parser callbacks; malformed/short resource
reads throw with handle cleanup. Optional short headers reject the original
unwritten-header branch. No original undefined-memory behavior is simulated.
First table publication remains if the second decoder fails; containing rollback
must clear it, matching the existing per-dataset publication contract. Trailing
bytes outside the first scene root or two declared table blocks are not parsed.

Evidence under ignored `analysis/work/mission-world-20261103`: existing
`tyre-data-format/006cbf30`, `loader/{006d45c0,006d4880,006d4280}` and
`animation/{006cb280,006cb0e0}` exports; new `mission-check-wrapper.asm`.
Original client SHA256 is
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.

## Process module bindings

`StartupSession` now exposes its actual process registry/registrations.
Script4 and TeamMgr11 may borrow those registrations through
`ClientModuleBinding`; standalone source fixtures retain their owning path.
Borrowing validates slot identity, kind and name, never replaces/clears a slot,
and leaves the original0048D950/0048DA30 registration semantics unchanged.
This avoids invalidating `StartupModules` identity checks on repeat loading.
The source adapter itself receives no new function credit.

## Verification and next work

[Receipt](checkpoints/p2.2-world-owners-20261109.json): focused native
ASan/UBSan/LSan checks file stages/shared table decoding/failure cleanup, repeated
actual Script/Players registration lifetimes, and physics1/3/4/fade ownership.
Strict i386 explicitly instantiates actual scene/document/collision/Items/runtime/
Physics compositions; native tests use declared engine/archive/parser endpoints.
The host owner itself is **compile-only**, not an executed engine/world test.
Application compile/link and the existing27 tracker tests pass. Temporary
fixtures are hashed and removed at this checkpoint; no permanent tests added.

Continue actual containing session/Human/DD0/FPV/HUD/SOUND/VM receivers and
activation/first-frame orchestration, including required derived actor activation.
Mode7 skips006989B0's MissionA8 finalization loop; HUD map kinds2/3/4 bind
zone/goals, while ordinary kind1 does not. Establish the actual required caller
before bringing unrelated objective/zone consumers forward. Deferred script
queue008692C0's observed producer is save restore, not cold mission loading.
Physics6 still rejects its unresolved second registration; see the
[physics contract](item-transfer-physics-loading.md). No speculative correction,
default successful receiver or partial scene preview is enabled.

2026-11-10: [Concrete cold Car callback binding](car-callback-world-binding.md) now
supplies the actual factory, checked module2 borrower and same tyre/emitter pool
resources; WorldActors retracts the borrower before Physics retirement.
[Application squad authority](live-squad-session-ownership.md) is explicit.
The containing session and required reached positive receivers remain open.
