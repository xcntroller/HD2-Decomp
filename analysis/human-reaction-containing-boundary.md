# H2 containing-path boundary —2026-11-12

D0 completed against clean master415e850, including merged UI source. This is a
call-path/dependency decision, not additional implementation or function credit.
H2 and P2.2 remain OPEN. Existing original evidence is indexed in WORK
`human-d0-containing-index.txt`; detailed source contracts remain authoritative.

## Required paths and dependency classification

| D12 case / actual trigger | Containing path and owners | Remaining boundary |
| --- | --- | --- |
| T1: live/corpse hit, source present/NULL | Human D4→005A1C90→005ABBD0/0077F7B0 Look command;0077DE30→0074D510→007B0910→007A7470, SAME B0/inline98/DD0/equipment/tracks/physics/model | D10: compose real control/Look/geometry, not only separate selection and retirement doubles. Native required bones, camera, input/integrity observations remain required. |
| T2: clip reason2/3 collision, expiry and replacement |007B3C80/007B7410→same007B0910 effect; retained resource and point/frame owners→0079BAC0 retirement | Source S01/S02 complete; D10/D12 verify transition invalidation, query restore and repeated entry through actual containing owner. No new clip solver work without a discrepancy. |
| T3: joint sleep/contact/rest |007C77E0→007CBD50→007CBF90/007CC740; rest007B37C0→00787110→existing ShotMgr/position queue/SOUND | Source S01/S03 complete; D10/D12 compose producers/publication and retire frames/audio before owners. |
| T4: ragdoll contacts Car | Point/body contact checks004DE800 then calls004F2790 directly through existing Car body410/solver | This is NOT hit58→004F2B60→004FF8B0. It needs initialized Car physical state, not mesh backup or arbitrary Car periodic/transport branches. D5 only where selected mode/owner requires it. |
| T5: moving Car hits Human |0059C2D0→005A07C0; authority speed>2→Human D4 kind13; current driver is damage source; reread B0 after damage | Requires actual Car speed/current occupants and Human lifetime. Missing B0 initially returns false; loss after damage is a required failure. Occupied producer is H3/D5 where selected. No automatic mesh deformation dependency on this edge. |
| T6: Car collision with seated Human |004D3EF0 owned Car source→004F2B60; velocity mutates BEFORE deformation, damage and passenger690 | D1/D2 positive004FF8B0 is required before this path can reach passenger damage. D3 repairs the seat-associated Door; H3 provides real Human occupancy/seat state; no synthetic successful occupant producer. |
| T7: blast/Mine/Versions→Human/Car | Existing shared explosion/BlastWave D8/D4/Scene2; Car004DBA10→module13→004DB360→detachment/destruction | S09–S14 source present; D10/D12 compose delivery and retirement. Reached fresh Item12 part5F9/wheel695 and EXPL8/smoke14 are D11 blockers, not defaults to initialize. |
| T8: positive mesh damage then restoration/re-entry |004FF8B0 captures original mesh by resource/part;004FE910(0) restores; containing004EC5C0 retirement precedes geometry destruction | D1/D2 recover native658 vector owner/retained mesh/count; D3 repairs FA8+28 Door visual20 before variant/visibility. Cannot close restore using a manually seeded cache alone. |
| T9: authority Car frame after clock expiry |Authority004D2790 reads periodic event6/pending target BEFORE water/timed DM004E1990 | D4 is required for a containing Car authority frame over that elapsed boundary. Passing a direct DM timer fixture does not bypass this earlier failure. Event7 inventory applies incrementally;259C observes navigation, not SOUND. Pending damage is armed by00586200 subtype14→004DEAD0, with H4's actual AI/item admission still required. |
| T10: non7 authority/presentation packet and mounted reaction | Existing canonical process inputs/peers/streams→actual Human/Car receivers and same reaction owners | D5 needed for reached non7 Car loading/control, D10 for receiver composition. Use existing real policy/input storage, not fake transport success. |

T1–T10 define source verification cases, not a claim that any has passed over a
fully initialized World. D12 must distinguish supported actual compositions,
explicit engine/OS endpoints and exact remaining failures. H3–H6 receivers reached
by these cases remain real prerequisites. Their unrelated behaviors are not
silently added to H2.

## State producer and retirement obligations

| State | Native producer → consumer/retirement | Owning checkpoint |
| --- | --- | --- |
| Human reaction/effect and joint/contact frames | Actual Look hit/0074D510/007B0910 and existing geometry loading → actual frame branch/0079BAC0 before inline/model destruction | D10/D12; existing S01–S04 contracts |
| Mesh original and usage count |004FF8B0 first affected vertex, keyed by existing resource7B8/part10 →004FE910 reverse restore, flag4/count/release/erase | D1/D2. Native658 sharing still unproved here; no new global owner assumed. |
| Seat-associated Door visual | Existing004E2E30 Door/seat discovery and part72 binding →004FEFD0 current visual20 →004FE910 restoration before variants | D3 source complete; no occupied Human prerequisite. Actual Human seat behavior remains H3/D5. |
| Car2584 collision clock |004D16A0 before camera/reset →004D3EF0/004D3D10, same loading lifetime | S14 complete; D5 preserves it during actual activation/re-entry. |
| Periodic/pending/voice state | Actual clock/target/list producers →004D3110 and derived event6/7 readers, existing SOUND lifetime | D4. Do not skip earlier frame guards to call the recovered timer. |
| Detached part5F9 / wheel695 | No confirmed producer in reached constructor/reset/launch paths → actual native read/toggle/final500ms | D11. Confirmed source failure; warm seeded fixtures do not resolve it. |
| EXPL8 / smoke14 | Confirmed unwritten/unparsed native records → actual DM/particle dereference | D11. Preserve exact required failure until producer or unreachable-path proof exists. |

## Shared entry and external delivery

Status9/received0 are callers of common game initialization, not callees of T1–T9.
Their worker/wrapper00730A40→0072E890→00731610, manager replacement and differing
retirement paths remain D6–D9 work under the [shared entry contract](multiplayer-world/shared-world.md).
They are mandatory when selecting that entry path; they do not become a prerequisite
for every offline Human damage/clip fixture. Live synchronized transport acceptance
remains Phase3. Do not expand all network services before testing an offline
containing reaction. T10 still requires actual receiver/policy/input owners.

UI merged at415e850 supplies the UI-owned consumers described in
[p22-ui-source-closure.md](p22-ui-source-closure.md). It does not supply missing
Human/Actor data or action producers, nor install the containing frame/render
callbacks. Preserve those source limits and verify new Human loans against the
merged endpoints; do not repeat the completed UI audit.

## Execution order

D0 and D1–D3 are source-complete, with [D2/D3 ownership correction](human-car-positive-deformation-source.md).
D4 follows before
claiming a continuing Car authority frame. D5 is mode/occupancy conditional.
D10 can advance independently for unmounted T1–T4 using actual owners, without
waiting for all transport work. D6–D9 remain explicitly tracked for their actual
entry callers. At any newly reached missing state, update the dependency map with
its caller/owner/retirement and keep the required failure visible.

D11's existing findings are not an excuse to reopen confirmed producer searches.
If T7/T8 requires one of those fresh paths and no lawful source producer exists,
report the exact case as blocked and continue independent required paths. D12
cannot close H2 with a reached required failure hidden by a double. Application
loading/render/failure/repeat and user desktop acceptance remain P2.2 gates.
