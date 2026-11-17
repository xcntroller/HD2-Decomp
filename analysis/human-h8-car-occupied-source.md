# Occupied offline Car presentation source

H8.4 source checkpoint, 2026-11-16, on `feature/p22-hum-h8-4` after `5aa99c9`.
This removes the artificial empty-seat restriction on offline presentation.
H8.4/D4/D5 remain open; this is neither a complete World nor desktop delivery.

## Original boundary

The existing indexed `car-activation-required/004d3110.c` (612-byte original)
clamps elapsed to83, publishes steering threshold and epoch, calls00509AA0 in
mode7, advances ten mounted controls in reverse through004C56A0, then calls
004D38F0. That118-byte original calls0050B180, publishes base direction from
actor2D0/core240 and position from actor1F0/core160, then004D3970 features.
Presentation continues with ascending00466110 seat cameras, navigation refresh
and reverse0051D5A0 observations, then the final model20 elapsed call. There is
no original empty-seat precondition. Non7 instead selects004D3380 and additional
network consumers; its existing required boundary remains in the caller.

Existing exports were reused after research-index lookup. No new export,
original execution or native ownership/default inference was needed.
`WORK/h8-4-contact-mapping-index.txt` also records004D3D10's retained boundary.
WORK is `analysis/work/mission-world-20261103` in the research checkout.

## Source composition

`present_car_offline_frame` shares the containing loading owner's actual
initializer, actor, frames, query/camera/navigation owners and services. The
production update calls it after mode/loading checks. The extracted helper keeps
the existing update order; the removed `require_unoccupied` was a reconstruction
guard. The source admission004D9430, input004D9050 and removal004D9A40 helpers
continue borrowing the SAME driver2321/controller2291, mass, physics, engine,
seat and camera state. No occupant, attachment or interior-audio state is
synthesized by presentation.00611080 remains a genuine empty receiver.

`CarLoadingSeats` accepts a Car type parameter, defaulting to its unchanged
production `CarSceneActor<Frames>`, so the identical source composition can be
exercised with explicit native actor/engine endpoints.

## Verification and limits

Temporary native audit passes596 checks,16 complete lifetimes and9 injected
failure prefixes with ASan/UBSan/leak detection. It executes actual CarGeometry,
authored wheel/seat discovery, default physics application, cold initialization,
mass, H3 admission/input/removal, controller, powered solver, BODYCOL controller,
finalizer, features, seat camera and navigation source on shared owners.
Positive ray hits use an analytic ground-plane endpoint and the real surface
filter. The first fresh suspension extension can clear contact; repeated frames
produce positive wheel contact without modifying production physics.

Checks cover driver/passenger linking and80-unit seat masses, borrowed driver
identity, first-buffer inputs, live actor pose, running fuel consumption,
attachment-gated process camera smoothing, elapsed83 clamp, masked/dead/absent
driver input, removal/re-entry and camera snap/restore. Source retirement removes
occupants/cameras before physical owners and leaves collision/scene borrowers
empty. Injected controller, solver, ray, finalizer, pose, sweep, camera position,
camera direction and final-tick failures verify no continuation past that point.

Frames/resource factories/math, Human/common actor storage, current-attachment
observation and terminal callbacks are explicit endpoints. The fixture does not
execute a complete Human attachment/Player lifetime, original engine trajectories,
positive body-shape collision, mounted firing, or every optional visual/audio
branch. Those branches retain their prior source contracts and verification;
they were not re-audited here. No full D5 closure is inferred from endpoint doubles.

Strict actual nested-host O0 i386 compilation/link instantiates both Car
activation/frame/retirement passes, real Human seat admission, driver inputs and
seat release. Launcher/probe builds and tracker checks are recorded in the
[receipt](checkpoints/p2.2-human-h8-car-occupied-20261116.json). Windows artifacts
are not executed. Temporary audits are fingerprinted and removed before commit;
no permanent tests, frozen entries, complete promotions or engine credit.

## Remaining H8.4 boundary

D11's confirmed fresh part5F9/wheel695/EXPL8/absent-model smoke14 omissions remain
explicit failures; there is no contradictory producer evidence. Repeated searches
or invented values cannot close them under current fidelity policy. See the
[exact failure contract](human-reaction-containing-boundary.md#source-closure-blockers).
User resumed work after clarifying D11 belongs to H8.4; that permits independent
required source work, not an implicit change to undefined-state policy.

D5's004D3D10 non-Car9A8/4A0 issue belongs to the selected non7 unowned collision
route: original004D3EF0/004DE800 makes offline7 owned unconditionally. It remains
required for non7; no general unreachability claim or fabricated mass is added.
Non7/entry D6–D9, reached absent actor/material owners, full occupied Human/Player
composition, application loading and desktop acceptance remain open. The known
D11 blocker prevents checking H8.4 complete with this source checkpoint.
