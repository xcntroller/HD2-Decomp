# Shared Scene2 movement source

005647C0 uses Scene2's captured active20 list and the Startup process owner
008AADD0. The process gathers Humans and Cars forward, then runs reverse
preparation and eight ordered substeps. Human+2C4 carries the exclusion mask
from signed health, B0 unavailability/action, attachment and DD0 animation.
Suspended Script peers receive bit 4 through the actual command-neighbour
lookup. 00475D50 can find a pending retirement for an empty identity below 32.

00564EF0 publishes immediate path steering for the excluded-from-constraints
branch. 005656E0 and 00565F30 borrow the same B0+F4 vector of Startup navigation
leases, use actual model pose, E0 posture and the 00816878 speed table, and
retain the cursor and progress rules. The vector borrower is cleared when the
owning B0 changes. Steering published by 005647C0 feeds the earlier B0 direction
and movement output through the same 2C4 allocation.

Car+210/+220 are the original embedded Car physics 180/190 collision bounds,
confirmed by the model-centre setup and 004EA780's descriptor construction.
005647C0 uses their differences times .55 as three radii. The Car contact
workers use the actual frame axes and the shared body velocity at Car+4A0.
The Human pair workers select the peer's prone posture. 00565540 combines
corrections, 00567C80 adds lateral correction, and 00569530 retains the signed
WORD path deadline. 00567AB0/00567BA0 use distinct starting F0 and projected F4
record borrowers. 00568190 and 00567DF0 project against linked floor volumes.
00569890 uses the containing navigation table's retained search buffer and
reverse links, with four breadth levels in ordinary movement and five in
local detour search.

0056A0F0 tests overlapping job destinations through 0056BE90 and returns
trimmed leases to the original pool in order, retaining the final job for the
search. 00563F90 supplies 200 work records and ten selected-route records.
0056A8E0 expands up to eight child edges per record and marks failed edges
explicitly. 0056C230 projects each endpoint through the actual navigation
table before sampling reverse Human neighbours and stationary Cars. Standing
Humans use two lateral points. Prone and Car ellipsoids use the ordered corner
sides from 0056DE90 and their live axes, with the Car expansion of 1.05.
The native ellipsoid-inside branch retains the last sampled line roots and
uses its perpendicular fallback. 0056A6D0 selects the nearest end, then the
lowest squared segment cost within the native tie tolerance. 0056A0F0 inserts
the selected route through the same Startup navigation pool and B0 vector.
The kind-two graph copy guards an uninitialized continuation beyond the two
edges written by the original. The containing application frame graph still
needs to invoke the host facade.

The [source receipt](checkpoints/p2.2-shared-scene-movement-20261111.json)
records native ASan/UBSan/LSan, strict i386 actual-host compile and launcher
link. The native fixture supplied model and clock doubles with actual pooled
jobs, Human and Car corrections, linked-volume projection and reverse floor
search. The [detour receipt](checkpoints/p2.2-shared-scene-movement-detours-20261111.json)
adds native standing, prone and Car reroutes, path selection, actual pooled
job replacement and return, and the containing i386 instantiation. Original
static exports are under WORK `frame-shared-movement`,
`frame-shared-movement-path`, `frame-shared-movement-volume` and
`frame-shared-movement-replan`, and `frame-shared-movement-detour-ellipse`.
