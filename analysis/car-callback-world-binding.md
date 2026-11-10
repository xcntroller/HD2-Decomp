# Car callback/world binding — source checkpoint 2026-11-10

P2.2 remains open and application loading still declines. The concrete cold Car
callback owner now constructs with actual host types and supplies the real
frame, body, solver, physics-contact and particle-pool borrowers. This is source
composition, not a loaded scene or desktop result.

## Containing owners

`GameCarCallbacks` and `make_game_car_callback_factory` require the existing
Startup, Car definitions and tyre registry. They create no application mode,
clock, random stream, frame-data store or Scene2 registry. The retained emitter
frames and pool states belong to its actual `CarParticlePool`; initialization is
separate and remains after real definition preparation. CarLoadingActivation
rejects different tyre/pool owners before activation. Tyre IDs are derived from
the current registry on each geometry/parameter application; the old borrowed
vector could bind a temporary or retain pre-reload IDs.

The Car owner can now be constructed before WorldActors/module2, without a
reference to an unconstructed containing dependency aggregate. A checked
`PhysicsContactLink` binds the SAME frame services and actual Physics module
later. Unbound/foreign/duplicate bindings reject. WorldActors installs the binding
only for its scene's actual callback owner and retracts it after runtime actors,
before Physics retirement. Scene2 Cars and callback/emitter owners then retire
under the existing scene/world lifetime. No second grid or Physics list exists.

Common Human getters use the same actual Human: signed EC health, field19C role
and existing005A0890 player policy. The fourteen original Scene2 constructor
vtables confirm every nonHuman+5C maps to00611070, genuine AL0/RET28; unknown
runtime families reject.0059C2D0 is a forwarding wrapper, not an empty receiver.
Its sixth argument preserves the contact FRAME, then appends null to+5C. The old
bool signature lost that pointer. Human005A07C0 genuinely returns when B0 is
absent; positive B0+C8/damage dispatch remains required.

## Concrete routing correction

004D4A70/geometry6A8 calls00410630, which returns00867990, physics module2.
Its vtable0080F640+4C points to00410530. Frame EC word4 is the actual stable
`PhysicsObjectHandle` published by0040FB00, not a script token. The callback now
routes to Physics before word5 Versions or flag8bit2 filtering, retaining fresh
EC reads. Slot naming and the earlier Script description were corrected.

00410530 moves a contacted second-list object to the first list BEFORE its
virtual14 call, regardless of the receiver's result. It swaps the last second
owner into the old slot, repairs that owner's stable index, appends the original
owner and updates its handle/list discriminator. Source validates actual handle
allocations before dereferencing frame tokens and reserves before transfer.
Callback failure leaves the already-migrated owner intact. Source lists are dense;
undefined native index/null-hole cases are not simulated.

0040B790 genuinely returns false for physics types0/1/3/4. Type2's0040EB00
genuinely returns true.00408780's cold kind3/nonpickup gate returns true without
tipping. Required type5 tipping, queued pickup00407BD0 and later movement still
reject; this checkpoint does not claim those positive bodies are supplied.

## Actual external force and impulse state

004F2A30 now accumulates into the SAME solver270/280 vectors, independently
honoring linear/angular suppression bits1/2. Torque is force CROSS offset,
transformed by cached350, scaled by supplied XYZ, then transformed by310.
22C0 is the SAME solver mass-changed/wake flag, followed by6A4 notification.
The installed cold Car has6A4 null; no replacement success service is invented.

004F2B60 changes actual body410 linear and210 angular velocity using918 mass,
220 inertia and current cached COM/matrices. It calls deformation under F6C8000,
then rereads F6C4000 before actual health/passenger callbacks. These precede
170 direction normalization and22C0. The passenger collision flag here is the
source actor pointer's truth value; native passes no contact descriptor. Failure
retains the original applied prefix rather than falsely completing the wake.

004D3EF0's offline004DE800=true branch now uses these bodies. Human contact
scales the force by500 and uses the original saved model Y with current COM X/Z.
Car contact uses1.2, or temporarily halves SAME918 and scales incoming impulse
by1.4 when both5E0 values are positive and target body health is zero. The original
mass is restored on return and source failure.004EA780's saved model160 position
is now retained separately from moving pose240, rather than inferred from a later
model transform. Gravity/mass/inertia/pools are not replaced with private copies.

## Required positive branches and evidence

GEffects+B4 is00485BA0, NOT opacity fade00486E60/00488D60. It checks visual1D0
bit800, reads mesh group0/face group0, walks reverse WORD triangles and invokes
00484F60 with actual vertex positions/UVs. The cold nonmesh gate is supplied;
positive fragments still require the real mesh/effect producer.004D4A10 only
hides after that callback returns and performs no frame update.

Required Human damage/state hits, nonoffline event serialization/ownership,
Versions part transitions0043C6A0→0043B000, runtime trigger005FEFB0, blast/surface
particles and detached Item12 remain explicit unrecovered receivers. Creating a
concrete class does not promote any of them to delivered or successful.

Original client SHA256:
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
WORK=`analysis/work/mission-world-20261103` retains exports in
`car-callback-{body-world,physics-contact,physics-type5,versions-contact}-required`,
`car-callback-world-required/004d3ef0.c`, the confirmed ctor/vtable map
`car-world-actor-hit-map.json`, `car-world-{human58,human5c,base-hit-rets,hit58,
part-mesh-effect}.asm` and `car-body-external-impulses.asm`.
0059C2D0/005A07C0/00611070/00485BA0/004D4A10/0040B790/0040EB00 are absent from the frozen
inventory and earn no new credit.00484F60/0043B000 research alone earns none.

[Receipt](checkpoints/p2.2-car-callback-world-binding-20261110.json) records focused
native sanitizer checks for real list migration/handle repair, actual body/solver
force and angular response, rotated inertia/live mass, failure prefixes, callback
order, metadata routing, checked late binding and retirement. Actual engine math,
frame and notification boundaries are doubles. Strict i386 composes the real
Car factory, WorldScene/WorldActors/Physics bind, shared resources and previous
Human/VM/router/offline spawn graph. Windows fixtures and original binaries were
not executed. No desktop claim. Continue the containing session and all reached
required receivers under the SAME full P2.2 checkbox.
