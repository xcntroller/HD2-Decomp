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
reverse links, with up to four breadth levels.

0056A0F0 tests overlapping job destinations through 0056BE90 and returns
trimmed leases to the original pool in order. It retains the final job and
honors 0056A8E0's genuine zero-horizontal-span branch. Positive local detour
construction, collision sampling and path insertion through 0056A8E0,
0056C230 and 0056A6D0 still require the original search worker. The host facade
calls that required receiver when reached. The live application frame graph
still needs to instantiate the facade with the containing World.

The [source receipt](checkpoints/p2.2-shared-scene-movement-20261111.json)
records native ASan/UBSan/LSan, strict i386 actual-host compile and launcher
link. The native fixture supplied model and clock doubles with actual pooled
jobs, Human and Car corrections, linked-volume projection and reverse floor
search. Original static exports are under WORK `frame-shared-movement`,
`frame-shared-movement-path`, `frame-shared-movement-volume` and
`frame-shared-movement-replan`. No Windows fixture was executed.
